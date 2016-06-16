/*
 * Functions used to read traces from files, used by the attack wrapper
 *
 * Copyright (C) 2009, 2010 Guillaume Duc <guillaume.duc@telecom-paristech.fr>
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
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "trace_files.h"
#include "utils.h"


#define TRACE_ARRAY_INCREMENTS 1024


char *trace_dir = NULL;
char *trace_dir_index_filename = NULL;


static struct trace_file *trace_filenames;
static int current_trace_filenames_index = 0;
static int num_trace_filenames = 0;
static int max_trace_filenames_index = 0;



/*
 * Comparaison between two trace files (to order the list of traces)
 */
static int
tracecmp (const void *p1, const void *p2)
{
  const struct trace_file *t1 = (const struct trace_file *) p1;
  const struct trace_file *t2 = (const struct trace_file *) p2;

  int res;

  res = memcmp (t1->key, t2->key, sizeof (t1->key));

  if (res != 0)
    return res;

  res = memcmp (t1->plaintext, t2->plaintext, sizeof (t1->plaintext));

  return res;
}


/*
 * Add a trace file to the list of trace files
 */
static void
add_trace_file_to_list (const struct trace_file *trace_filename)
{
  if (current_trace_filenames_index == max_trace_filenames_index)
    {
      /* We have reached the allocated size for the array, increase it */
      max_trace_filenames_index += TRACE_ARRAY_INCREMENTS;
      size_t new_size = max_trace_filenames_index * sizeof (struct trace_file);

      trace_filenames = realloc (trace_filenames, new_size);

      if (trace_filenames == NULL)
	{
	  printf ("EE - Error during realloc (out of memory?)\n");
	  exit (EXIT_FAILURE);
	}
    }

  memcpy (&trace_filenames[current_trace_filenames_index],
	  trace_filename, sizeof (struct trace_file));

  current_trace_filenames_index++;
  num_trace_filenames++;
}



/*
 * Parse a trace filename and if it is correct, add it to the list of
 * trace files
 */
static void
parse_trace_filename (const char *filename)
{
  struct trace_file trace_filename;

  char fullname[512];

  struct stat sb;
  char *k, *p, *c;
  char trace_key[32], trace_plaintext[32], trace_ciphertext[32];


  /* Build the fullname (path + filename) */
  snprintf (fullname, sizeof (fullname), "%s/%s",
	    trace_dir, filename);


  /* Check if the file exists and if its filename looks like a trace filename */
  if (stat (fullname, &sb) == -1)
    {
      perror ("stat");
      return;
    }

  if (! S_ISREG (sb.st_mode))
    {
      /* Not a regular file... Skip it */
      return;
    }

  k = strstr (filename, "k=");
  if (k == NULL)
    {
      /* 'k=' not found in filename... Skip it */
      return;
    }

  p = strstr (filename, "m=");
  if (p == NULL)
    {
      /* 'm=' not found in filename... Skip it */
      return;
    }

  c = strstr (filename, "c=");
  if (c == NULL)
    {
      /* 'c=' not found in filename... Skip it */
      return;
    }

  k += 2;
  p += 2;
  c += 2;

  memcpy (trace_key, k, 32);
  memcpy (trace_plaintext, p, 32);
  memcpy (trace_ciphertext, c, 32);

  /* Extract the key, plaintext and ciphertext from the filename */

  hex2uint8_16bytes (trace_key, trace_filename.key);
  hex2uint8_16bytes (trace_plaintext, trace_filename.plaintext);
  hex2uint8_16bytes (trace_ciphertext, trace_filename.ciphertext);

  strncpy (trace_filename.filename, filename, sizeof (trace_filename.filename));
  trace_filename.filename[sizeof (trace_filename.filename) - 1] = '\0';

  memcpy (trace_filename.key_hex, trace_key, 32);
  trace_filename.key_hex[32] = '\0';

  /* Add the trace to the list */
  add_trace_file_to_list (&trace_filename);
}



/*
 * Initialize the list of trace filenames by listing the trace
 * directory (may be (very) slow)
 */
static void
trace_dir_init_listing ()
{
  DIR *dir;
  struct dirent *dirent;

  if ((dir = opendir (trace_dir)) == NULL)
    {
      perror ("opendir (trace directory)");
      exit (EXIT_FAILURE);
    }

  while (dirent = readdir (dir))
    {
      parse_trace_filename (dirent->d_name);

      if ((current_trace_filenames_index % 10000 == 0) &&
	  (current_trace_filenames_index != 0))
	{
	  printf ("%i... ", current_trace_filenames_index);
	  fflush (stdout);
	}
    }

  closedir (dir);
}


/*
 * Initialize the list of trace filenames by using the index file
 * (faster)
 */
static void
trace_dir_init_index ()
{
  FILE *index_file = NULL;
  char buffer[512];
  char buf[33];
  size_t pos;
  struct trace_file trace_filename;

  assert (trace_dir_index_filename != NULL);

  index_file = fopen (trace_dir_index_filename, "r");
  if (index_file == NULL)
    {
      printf ("EE - Cannot open index file!\n");
      perror ("fopen (index file)");
      exit (EXIT_FAILURE);
    }

  while (fgets (buffer, sizeof (buffer), index_file) != NULL)
    {
      if (strlen (buffer) < 100)
	continue;

      /* Key */
      pos = 0;
      memcpy (buf, &buffer[pos], sizeof (buf));
      buf[32] = '\0';
      memcpy (trace_filename.key_hex, buf, sizeof (trace_filename.key_hex));

      hex2uint8_16bytes (buf, trace_filename.key);


      /* Plaintext */
      pos += 33;
      memcpy (buf, &buffer[pos], sizeof (buf));
      buf[32] = '\0';

      hex2uint8_16bytes (buf, trace_filename.plaintext);


      /* Ciphertext */
      pos += 33;
      memcpy (buf, &buffer[pos], sizeof (buf));
      buf[32] = '\0';

      hex2uint8_16bytes (buf, trace_filename.ciphertext);


      /* Filename */
      pos += 33;
      strncpy (trace_filename.filename, &buffer[pos], sizeof (trace_filename.filename));
      trace_filename.filename[strlen (&buffer[pos]) - 1] = '\0';

      /* Add the trace to the list */
      add_trace_file_to_list (&trace_filename);

    }

  fclose (index_file);
}




/*
 * Initialize the list of trace filenames
 */
void
trace_dir_init ()
{
  assert (trace_dir != NULL);

  printf ("II - Building trace list... ");
  fflush (stdout);

  trace_filenames = (struct trace_file *) malloc (sizeof (struct trace_file) *
						  TRACE_ARRAY_INCREMENTS);
  max_trace_filenames_index = TRACE_ARRAY_INCREMENTS;


  if (trace_dir_index_filename != NULL)
    trace_dir_init_index ();
  else
    trace_dir_init_listing ();

  qsort (trace_filenames, num_trace_filenames,
	 sizeof (struct trace_file), tracecmp);

  printf ("done\n");
}



/*
 * Find the key to attack in the trace files list
 */
void
trace_dir_findkey ()
{
  uint8_t key_tmp[TRACE_KEY_SIZE_BYTES];
  uint16_t index_key = 0;
  int index_tab = 0;

  printf ("II - Find the key to use in the list of traces... ");
  fflush (stdout);

  memcpy (key_tmp, trace_filenames[0].key, sizeof (key_tmp));

  do
    {
      if (memcmp (key_tmp, trace_filenames[index_tab].key, sizeof (key_tmp)) != 0)
	{
	  memcpy (key_tmp, trace_filenames[index_tab].key, sizeof (key_tmp));
	  index_key++;
	}

      index_tab++;

      if ((index_tab == num_trace_filenames) &&
	  (index_key != num_dbkey))
	{
	  printf ("key not found!\n");
	  exit (EXIT_FAILURE);
	}
    }
  while (index_key != num_dbkey);

  index_tab--;
  current_trace_filenames_index = index_tab;

  memcpy (correct_key_hex, trace_filenames[index_tab].key_hex, sizeof (correct_key_hex));
  memcpy (correct_key, trace_filenames[index_tab].key, sizeof (correct_key));

  printf ("Ok (key = %s)\n", correct_key_hex);
}


/*
 * Read a trace file
 */
void
parse_trace_file (const char *trace_filename)
{
  FILE *fs;
  size_t len = 0;

  int32_t value;
  int nb_values = 0;

  char fullname[512];
  char buffer[512];


  /* Build the fullname (path + filename) */
  snprintf (fullname, sizeof (fullname), "%s/%s",
	    trace_dir, trace_filename);


  /* Open the file */
  fs = fopen (fullname, "r");
  if (fs == NULL)
    {
      printf ("%s\n", fullname);
      perror ("fopen (trace file)");
      exit (EXIT_FAILURE);
    }

  /* Parse the file */
  while ((fgets (buffer, sizeof (buffer), fs)) != NULL)
    {
      if (strlen (buffer) < 1)
	continue;

      if (buffer[0] == '#')
	continue;

      if (sscanf (buffer, "%d", &value) != 1)
	continue;

      next_trace.samples[nb_values] = (int16_t) value;

      nb_values++;

      if (nb_values == TRACE_NUM_POINTS)
	break;
    }

  fclose (fs);
}




/*
 * This function retrieves a trace from a file
 * trace_num: Number of trace to retrieve from the disk
 */
void
trace_dir_get_trace (uint16_t trace_num)
{
  /* Check if we are still using the correct key */
  if (memcmp (correct_key,
	      trace_filenames[current_trace_filenames_index].key,
	      sizeof (correct_key)) != 0)
    {
      printf ("EE - No more traces for this key!\n");
      exit (EXIT_FAILURE);
    }

  /* Parse the trace file */
  parse_trace_file (trace_filenames[current_trace_filenames_index].filename);

  /* Fill the next_trace structure */
  memcpy (next_trace.plaintext,
	  trace_filenames[current_trace_filenames_index].plaintext,
	  sizeof (next_trace.plaintext));

  memcpy (next_trace.ciphertext,
	  trace_filenames[current_trace_filenames_index].ciphertext,
	  sizeof (next_trace.ciphertext));

  current_trace_filenames_index++;
}


/*
 * This function cleans some things
 */
void
trace_dir_close ()
{
  /* Nothing usefull */
}
