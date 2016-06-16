/*
 * Compute results
 *
 * This program computes the different metrics from the information
 * collected by the attack wrapper.
 *
 *
 * Copyright (C) 2010 Guillaume Duc <guillaume.duc@telecom-paristech.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>

#include "dpa_contest.h"
#include "dpa_contest_results.h"

#define GSR_THRESHOLD 0.8
#define PSR_THRESHOLD 0.8
#define PGE_THRESHOLD 10.0

uint32_t num_iterations;
uint32_t num_experiments;
uint32_t current_experiment;

uint32_t *partial_success_rate_acc;
uint32_t *partial_guessing_entropy_acc;
uint32_t *global_success_rate_acc;

float *partial_success_rate;
float *partial_guessing_entropy;
float *global_success_rate;

double time_consumed_acc;
double mean_time_consumed;

uint32_t trace_gsr_above_thr;
uint32_t trace_psr_above_thr;
uint32_t trace_pge_below_thr;
uint32_t trace_gsr_above_thr_stable;
uint32_t trace_psr_above_thr_stable;
uint32_t trace_pge_below_thr_stable;
float gsr_end;
float min_psr_end, max_psr_end;
float min_pge_end, max_pge_end;

FILE *current_result_file;

uint8_t current_correct_subkey[TRACE_KEY_SIZE_BYTES];

int program_argc_start;
char **program_argv;

char *output_filename_prefix = "results_";

char *program_version = "$Revision$";



/*
 * Create the different intermediate and result arrays
 */
void
create_data_structures ()
{
  partial_success_rate = (float *) malloc (TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (float));
  partial_guessing_entropy = (float *) malloc (TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (float));
  global_success_rate = (float *) malloc (num_iterations * sizeof (float));

  partial_success_rate_acc = (uint32_t *) malloc (TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (uint32_t));
  partial_guessing_entropy_acc = (uint32_t *) malloc (TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (uint32_t));
  global_success_rate_acc = (uint32_t *) malloc (num_iterations * sizeof (uint32_t));

  if ((partial_success_rate == NULL) ||
      (partial_guessing_entropy == NULL) ||
      (global_success_rate == NULL) ||
      (partial_success_rate_acc == NULL) ||
      (partial_guessing_entropy_acc == NULL) ||
      (global_success_rate_acc == NULL))
    {
      printf ("EE - Error during malloc!\n");
      exit (EXIT_FAILURE);
    }


  memset (partial_success_rate_acc, 0,
	  TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (uint32_t));
  memset (partial_guessing_entropy_acc, 0,
	  TRACE_KEY_SIZE_BYTES * num_iterations * sizeof (uint32_t));
  memset (global_success_rate_acc, 0,
	  num_iterations * sizeof (uint32_t));

  time_consumed_acc = 0.0;
}


/*
 * This function displays how to use the program.
 */
void
print_usage (char *argv0)
{
  printf ("Usage: %s [OPTION]... FILENAME [FILENAME...]\n", argv0);
  printf ("Launch the program to compute the results.\n");
  printf ("FILENAME represents the name of a binary result file produced by the attack wrapper\n\n");

  printf ("The options are:\n");
  printf ("\t-o, --output=PREFIX\tPrefix of the output filenames (default: results_)\n");
  printf ("\t-h, --help\t\tPrint this help message\n");
  printf ("\t-v, --version\t\tPrint the version of the attack wrapper\n");
}


/*
 * This function displays the version of the program.
 */
void
print_version ()
{
  printf ("Compute results\n");
  printf ("Version: %s\n", program_version);
}


/*
 * This function parses the command line arguments.
 */
void
parse_command_line (int argc, char **argv)
{
  int c;

  while (1)
    {
      static struct option options[] =
	{
	  {"output", required_argument, 0, 'o'},
	  {"help", no_argument, 0, 'h'},
	  {"version", no_argument, 0, 'v'},
	  {0, 0, 0, 0}
	};

      int option_index = 0;

      c = getopt_long (argc, argv, "o:hv", options, &option_index);

      if (c == -1)
	break;

      switch (c)
	{
	case 'o':
	  output_filename_prefix = strdup (optarg);
	  break;
	case 'h':
	  print_usage (argv[0]);
	  exit (EXIT_SUCCESS);
	  break;
	case 'v':
	  print_version ();
	  exit (EXIT_SUCCESS);
	  break;
	}
    }

  if (optind >= argc)
    {
      printf ("EE - The name of, at least, one result file is missing!\n\n");
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  program_argv = argv;
  program_argc_start = optind;
  num_experiments = argc - optind;
}


/*
 * Open a file containing results collected by the wrapper
 */
void
open_result_file ()
{
  current_result_file = fopen (program_argv[program_argc_start + current_experiment], "r");

  if (current_result_file == NULL)
    {
      perror ("fopen");
      exit (EXIT_FAILURE);
    }

  printf ("II - Open file %s\n",
	  program_argv[program_argc_start + current_experiment]);
}


/*
 * Close a file containing results collected by the wrapper
 */
void
close_result_file ()
{
  fclose (current_result_file);
}


/*
 * Check the content of the result file
 */
void
check_result_file ()
{
  size_t res;
  struct result_file_header header;

  res = fread (&header, sizeof (header), 1, current_result_file);

  if (res != 1)
    {
      printf ("EE - Cannot read the file header!\n");
      exit (EXIT_FAILURE);
    }

  if (header.format_version != 1)
    {
      printf ("EE - Wrong file format!\n");
      exit (EXIT_FAILURE);
    }

  if (current_experiment == 0)
    {
      num_iterations = header.num_iterations;

      create_data_structures ();
    }
  else
    {
      if (num_iterations != header.num_iterations)
	{
	  printf ("EE - Experiments have different number of iterations!\n");
	  exit (EXIT_FAILURE);
	}
    }

  memcpy (current_correct_subkey, header.correct_subkey,
	  sizeof (current_correct_subkey));
}


/*
 * This function parse a result file
 */
void
parse_result_file ()
{
  uint32_t iter;
  uint32_t subkey_byte;
  uint32_t byte_index;
  uint32_t num_correct_subkey_bytes;
  size_t res;

  struct result_file_trace_header trace_header;
  struct attack_partial_result attack_partial_result;

  for (iter = 0; iter < num_iterations; iter++)
    {
      res = fread (&trace_header, sizeof (trace_header),
		   1, current_result_file);

      if (res != 1)
	{
	  printf ("EE - Cannot read file!\n");
	  exit (EXIT_FAILURE);
	}

      res = fread (&attack_partial_result,
		   sizeof (attack_partial_result),
		   1, current_result_file);

      if (res != 1)
	{
	  printf ("EE - Cannot read file!\n");
	  exit (EXIT_FAILURE);
	}


      num_correct_subkey_bytes = 0;

      for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
	{
	  // Accumulation for partial guessing entropy
	  for (byte_index = 0; byte_index < 256; byte_index++)
	    {
	      if (current_correct_subkey[subkey_byte] ==
		  attack_partial_result.bytes[subkey_byte][byte_index])
		{
		  partial_guessing_entropy_acc[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] += byte_index + 1;
		  break;
		}
	    }
	  if (byte_index == 256)
	    {
	      // The byte value does not appear in the list, give the worst possible score
	      partial_guessing_entropy_acc[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] += 256;
	    }


	  // Accumulation for partial success rate
	  if (current_correct_subkey[subkey_byte] ==
	      attack_partial_result.bytes[subkey_byte][0])
	    {
	      partial_success_rate_acc[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] += 1;
	      num_correct_subkey_bytes++;
	    }
	}


      // Accumulation for global success rate
      if (num_correct_subkey_bytes == TRACE_KEY_SIZE_BYTES)
	{
	  global_success_rate_acc[iter] += 1;
	}

      // Accumulation for time consumed
      time_consumed_acc += trace_header.time_consumed;
    }
}


/*
 * Compute the result metrics from the data accumulated
 * during the analysis of result files.
 */
void
compute_metrics ()
{
  uint32_t iter;
  uint32_t subkey_byte;

  printf ("II - Compute result metrics\n");

  trace_gsr_above_thr = num_iterations + 1;
  trace_psr_above_thr = num_iterations + 1;
  trace_pge_below_thr = num_iterations + 1;

  trace_gsr_above_thr_stable = num_iterations + 1;
  trace_psr_above_thr_stable = num_iterations + 1;
  trace_pge_below_thr_stable = num_iterations + 1;


  for (iter = 0; iter < num_iterations; iter++)
    {
      float min_psr = 2.0;
      float max_pge = 0.0;

      for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
	{
	  partial_success_rate[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] =
	    (float) partial_success_rate_acc[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]
	    / (float) num_experiments;

	  partial_guessing_entropy[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] =
	    (float) partial_guessing_entropy_acc[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]
	    / (float) num_experiments;


	  if (partial_success_rate[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] < min_psr)
	    min_psr = partial_success_rate[subkey_byte+iter*TRACE_KEY_SIZE_BYTES];
	  if (partial_guessing_entropy[subkey_byte+iter*TRACE_KEY_SIZE_BYTES] > max_pge)
	    max_pge = partial_guessing_entropy[subkey_byte+iter*TRACE_KEY_SIZE_BYTES];
	}

      global_success_rate[iter] = (float) global_success_rate_acc[iter]
	/ (float) num_experiments;


      if ((global_success_rate[iter] > GSR_THRESHOLD) && (iter < trace_gsr_above_thr))
	trace_gsr_above_thr = iter;
      if ((min_psr > PSR_THRESHOLD) && (iter < trace_psr_above_thr))
	trace_psr_above_thr = iter;
      if ((max_pge < PGE_THRESHOLD) && (iter < trace_pge_below_thr))
	trace_pge_below_thr = iter;

      if ((global_success_rate[iter] > GSR_THRESHOLD) && (iter < trace_gsr_above_thr_stable))
	trace_gsr_above_thr_stable = iter;
      if ((min_psr > PSR_THRESHOLD) && (iter < trace_psr_above_thr_stable))
	trace_psr_above_thr_stable = iter;
      if ((max_pge < PGE_THRESHOLD) && (iter < trace_pge_below_thr_stable))
	trace_pge_below_thr_stable = iter;

      if ((global_success_rate[iter] <= GSR_THRESHOLD) && (iter > trace_gsr_above_thr_stable))
	trace_gsr_above_thr_stable = num_iterations + 1;
      if ((min_psr <= PSR_THRESHOLD) && (iter > trace_psr_above_thr_stable))
	trace_psr_above_thr_stable = num_iterations + 1;
      if ((max_pge >= PGE_THRESHOLD) && (iter > trace_pge_below_thr_stable))
	trace_pge_below_thr_stable = num_iterations + 1;
    }

  mean_time_consumed = time_consumed_acc / (num_iterations * num_experiments);

  gsr_end = global_success_rate[num_iterations - 1];
  min_psr_end = partial_success_rate[(num_iterations - 1)*TRACE_KEY_SIZE_BYTES];
  max_psr_end = partial_success_rate[(num_iterations - 1)*TRACE_KEY_SIZE_BYTES];
  min_pge_end = partial_guessing_entropy[(num_iterations - 1)*TRACE_KEY_SIZE_BYTES];
  max_pge_end = partial_guessing_entropy[(num_iterations - 1)*TRACE_KEY_SIZE_BYTES];

  for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
    {
      float psr = partial_success_rate[subkey_byte + (num_iterations - 1)*TRACE_KEY_SIZE_BYTES];
      float pge = partial_guessing_entropy[subkey_byte + (num_iterations - 1)*TRACE_KEY_SIZE_BYTES];

      if (psr < min_psr_end)
	min_psr_end = psr;
      if (psr > max_psr_end)
	max_psr_end = psr;
      if (pge < min_pge_end)
	min_pge_end = pge;
      if (pge > max_pge_end)
	max_pge_end = pge;
    }
}


/*
 * Output the result metrics
 */
void
output_metrics ()
{
  uint32_t iter;
  uint32_t subkey_byte;

  char filename[256];

  FILE *partial_success_rate_files[TRACE_KEY_SIZE_BYTES];
  FILE *partial_guessing_entropy_files[TRACE_KEY_SIZE_BYTES];

  FILE *partial_success_rate_global_file;
  FILE *partial_guessing_entropy_global_file;
  FILE *global_success_rate_global_file;

  FILE *misc_file, *misc_html_file;


  printf ("II - Writing result files\n");


  // Open the files
  for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
    {
      snprintf (filename, sizeof (filename), "%spartial_success_rate_b=%02u.dat",
		output_filename_prefix, subkey_byte + 1);

      partial_success_rate_files[subkey_byte] = fopen (filename, "w");

      snprintf (filename, sizeof (filename), "%spartial_guessing_entropy_b=%02u.dat",
		output_filename_prefix, subkey_byte + 1);

      partial_guessing_entropy_files[subkey_byte] = fopen (filename, "w");
    }

  snprintf (filename, sizeof (filename), "%spartial_success_rate_all.dat",
	    output_filename_prefix);

  partial_success_rate_global_file = fopen (filename, "w");

  snprintf (filename, sizeof (filename), "%spartial_guessing_entropy_all.dat",
	    output_filename_prefix);

  partial_guessing_entropy_global_file = fopen (filename, "w");

  snprintf (filename, sizeof (filename), "%sglobal_success_rate.dat",
	    output_filename_prefix);

  global_success_rate_global_file = fopen (filename, "w");

  snprintf (filename, sizeof (filename), "%smisc.dat",
	    output_filename_prefix);

  misc_file = fopen (filename, "w");

  snprintf (filename, sizeof (filename), "%smisc_html.dat",
	    output_filename_prefix);

  misc_html_file = fopen (filename, "w");


  // Print the files headers
  for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
    {
      fprintf (partial_guessing_entropy_files[subkey_byte], "# Trace_num Guessing_entropy\n");
      fprintf (partial_success_rate_files[subkey_byte], "# Trace_num Partial_success_rate\n");

    }
  fprintf (partial_guessing_entropy_global_file, "# Trace_num Subkey_byte Guessing_entropy\n");
  fprintf (partial_success_rate_global_file, "# Trace_num Subkey_byte Partial_success_rate\n");
  fprintf (global_success_rate_global_file, "# Trace_num Global_success_rate\n");


  // Output the results
  for (iter = 0; iter < num_iterations; iter++)
    {
      for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
	{
	  fprintf (partial_guessing_entropy_files[subkey_byte], "%u %f\n", iter,
		   partial_guessing_entropy[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]);

	  fprintf (partial_guessing_entropy_global_file, "%u %u %f\n", iter, subkey_byte,
		   partial_guessing_entropy[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]);

	  fprintf (partial_success_rate_files[subkey_byte], "%u %f\n", iter,
		   partial_success_rate[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]);

	  fprintf (partial_success_rate_global_file, "%u %u %f\n", iter, subkey_byte,
		   partial_success_rate[subkey_byte+iter*TRACE_KEY_SIZE_BYTES]);
	}

      fprintf (global_success_rate_global_file, "%u %f\n", iter,
	       global_success_rate[iter]);

      // For Gnuplot and pm3d
      fprintf (partial_guessing_entropy_global_file, "\n");
      fprintf (partial_success_rate_global_file, "\n");
    }


  // Output misc results
  fprintf (misc_file, "Min trace GSR > %u%%: %u\n", ((int)(GSR_THRESHOLD * 100)), trace_gsr_above_thr);
  fprintf (misc_file, "Min trace Min PSR > %u%%: %u\n", ((int)(PSR_THRESHOLD * 100)), trace_psr_above_thr);
  fprintf (misc_file, "Min trace Max PGE < %u: %u\n", ((int)PGE_THRESHOLD), trace_pge_below_thr);
  fprintf (misc_file, "Min trace GSR stable > %u%%: %u\n", ((int)(GSR_THRESHOLD * 100)), trace_gsr_above_thr_stable);
  fprintf (misc_file, "Min trace Min PSR stable > %u%%: %u\n", ((int)(PSR_THRESHOLD * 100)), trace_psr_above_thr_stable);
  fprintf (misc_file, "Min trace Max PGE stable < %u: %u\n", ((int)PGE_THRESHOLD), trace_pge_below_thr_stable);
  fprintf (misc_file, "GSR at trace %u: %.2f\n", num_iterations, gsr_end);
  fprintf (misc_file, "Min PSR at trace %u: %.2f\n", num_iterations, min_psr_end);
  fprintf (misc_file, "Max PSR at trace %u: %.2f\n", num_iterations, max_psr_end);
  fprintf (misc_file, "Min PGE at trace %u: %.2f\n", num_iterations, min_pge_end);
  fprintf (misc_file, "Max PGE at trace %u: %.2f\n", num_iterations, max_pge_end);
  fprintf (misc_file, "Mean time / trace: %.2f\n", mean_time_consumed);


  // Output misc HTML results
  fprintf (misc_html_file, "<tr>\n<td>X<br />\nY<br />\nZ<br />\n.</td>\n");
  fprintf (misc_html_file, "<td>%u</td>\n", trace_gsr_above_thr);
  fprintf (misc_html_file, "<td>%u</td>\n", trace_psr_above_thr);
  fprintf (misc_html_file, "<td>%u</td>\n", trace_pge_below_thr);
  fprintf (misc_html_file, "<td>%u</td>\n", trace_gsr_above_thr_stable);
  fprintf (misc_html_file, "<td>%u</td>\n", trace_psr_above_thr_stable);
  fprintf (misc_html_file, "<td>%u</td>\n", trace_pge_below_thr_stable);
  fprintf (misc_html_file, "<td>%.2f</td>\n", gsr_end);
  fprintf (misc_html_file, "<td>%.2f</td>\n", min_psr_end);
  fprintf (misc_html_file, "<td>%.2f</td>\n", max_psr_end);
  fprintf (misc_html_file, "<td>%.2f</td>\n", min_pge_end);
  fprintf (misc_html_file, "<td>%.2f</td>\n", max_pge_end);
  fprintf (misc_html_file, "<td>%.2f s</td>\n</tr>\n", mean_time_consumed);


  // Close the files
  for (subkey_byte = 0; subkey_byte < TRACE_KEY_SIZE_BYTES; subkey_byte++)
    {
      fclose (partial_success_rate_files[subkey_byte]);
      fclose (partial_guessing_entropy_files[subkey_byte]);
    }

  fclose (partial_success_rate_global_file);
  fclose (partial_guessing_entropy_global_file);
  fclose (global_success_rate_global_file);
  fclose (misc_file);
  fclose (misc_html_file);
}



/*
 * The main function of the program.
 */
int
main (int argc, char **argv)
{
  // Do some checks to verify the proper alignemnt of data structure
  assert (sizeof (struct attack_partial_result) == 16*256+1);
  assert (sizeof (struct result_file_header) == 2*TRACE_KEY_SIZE_BYTES+4*4);
  assert (sizeof (struct result_file_trace_header) == TRACE_PLAINTEXT_SIZE_BYTES
	  + TRACE_CIPHERTEXT_SIZE_BYTES + sizeof(double));

  parse_command_line (argc, argv);


  for (current_experiment = 0; current_experiment < num_experiments; current_experiment++)
    {
      open_result_file ();

      check_result_file ();

      parse_result_file ();

      close_result_file ();
    }

  compute_metrics ();

  output_metrics ();

  exit (EXIT_SUCCESS);
}
