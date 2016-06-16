/*
 * Attack wrapper
 *
 * This program is used to launch the attack program, to provide it with
 * traces from the public database (or from the private database when run
 * inside Telecom Paristech) and to collect results.
 *
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
#include <poll.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <libgen.h>

#include "dpa_contest.h"
#include "dpa_contest_results.h"
#include "aes.h"
#include "trace_files.h"


#ifdef HAVE_POSTGRESQL
#include "db_postgresql.h"
#endif /* HAVE_POSTGRESQL */


#define COMM_BUFFER_SIZE 128
#define NUM_ITERATIONS 20000
#define MAX_ITERATIONS 20000
#define NUM_DBKEY 0
#define MAX_NUM_DBKEY 31


int stdin_pipe_fd[2];
int stdout_pipe_fd[2];
int stderr_pipe_fd[2];

int child_stdin_fd;
int child_stdout_fd;
int child_stderr_fd;

int fifo_mode = 0;
char fifo_from_wrapper_filename[512];
char fifo_to_wrapper_filename[512];

pid_t process_id;

uint8_t comm_buffer[COMM_BUFFER_SIZE];

FILE *result_file;

char *output_filename = "results";
uint16_t num_iterations = NUM_ITERATIONS;
uint16_t num_dbkey = NUM_DBKEY;
char *attack_program_filename = NULL;
int output_binary = 0;

char correct_key_hex[2*TRACE_KEY_SIZE_BYTES + 1];
uint8_t correct_key[TRACE_KEY_SIZE_BYTES];
uint8_t correct_subkey[TRACE_KEY_SIZE_BYTES];
uint8_t subkey_num;

uint16_t current_trace_num = 0;

struct attack_trace current_trace;
struct attack_trace next_trace;

struct timeval start_time;
double time_consumed;


/* Data for multi-threading */
sem_t sem_num_trace_to_read;
sem_t sem_num_trace_to_write;
pthread_t trace_thread;
int trace_thread_launched = 0;




/*
 * This function creates the communication pipes used to
 * communicate between the wrapper and the attack program.
 * Three pipes are created, one to send data to the attack
 * program (stdin of the attack program) and two to retrieve
 * data from the attack program (stdout and stderr).
 */
void
create_communication_pipes ()
{
  if (fifo_mode == 0)
    {
      /*
       * Normal (fork) mode
       * -> Create pipes
       */
      if (pipe (stdin_pipe_fd) == -1)
	{
	  perror ("pipe");
	  exit (EXIT_FAILURE);
	}
      if (pipe (stdout_pipe_fd) == -1)
	{
	  perror ("pipe");
	  exit (EXIT_FAILURE);
	}
      if (pipe (stderr_pipe_fd) == -1)
	{
	  perror ("pipe");
	  exit (EXIT_FAILURE);
	}

      child_stdin_fd = stdin_pipe_fd[1];
      child_stdout_fd = stdout_pipe_fd[0];
      child_stderr_fd = stderr_pipe_fd[0];
    }
  else
    {
      /*
       * FIFO mode
       * -> Create and open FIFOs
       */

      snprintf (fifo_from_wrapper_filename, sizeof (fifo_from_wrapper_filename),
		"%s_from_wrapper", attack_program_filename);
      snprintf (fifo_to_wrapper_filename, sizeof (fifo_to_wrapper_filename),
		"%s_to_wrapper", attack_program_filename);

      if (mkfifo (fifo_from_wrapper_filename, 0600) < 0)
	{
	  perror ("mkfifo");
	  exit (EXIT_FAILURE);
	}

      if (mkfifo (fifo_to_wrapper_filename, 0600) < 0)
	{
	  perror ("mkfifo");
	  exit (EXIT_FAILURE);
	}

      printf ("II - FIFO Wrapper -> Attack: %s\n", fifo_from_wrapper_filename);
      printf ("II - FIFO Attack -> Wrapper: %s\n", fifo_to_wrapper_filename);


      /* The output fifo (wrapper->attack) should be non blocking
       * so that writes to it won't block */
      child_stdin_fd = open (fifo_from_wrapper_filename, O_RDWR | O_NONBLOCK);
      if (child_stdin_fd < 0)
	{
	  perror ("open write fifo");
	  exit (EXIT_FAILURE);
	}


      /* The input fifo (attack->wrapper) should be blocking
       * so that reads will block until data is available,
       * but we open it as O_RDWR to prevent open from blocking */
      child_stdout_fd = open (fifo_to_wrapper_filename, O_RDWR);
      if (child_stdout_fd < 0)
	{
	  perror ("open read fifo");
	  exit (EXIT_FAILURE);
	}

      child_stderr_fd = -1;
    }
}


/*
 * The function performs cleanup on exit
 */
void
close_everything ()
{
  if (result_file != NULL)
    fclose (result_file);
  result_file = NULL;

  if (trace_dir == NULL)
    db_close ();
  else
    trace_dir_close ();

  if (fifo_mode)
    {
      /*
       * Close and remove FIFOs
       */
      close (child_stdin_fd);
      close (child_stdout_fd);

      unlink (fifo_to_wrapper_filename);
      unlink (fifo_from_wrapper_filename);

      printf ("II - FIFO removed\n");
    }
}



/*
 * This signal handler is called when the attack program dies.
 * Note: this handler can only be called in normal (fork) mode not in FIFO mode
 * Do some cleaning.
 */
void
sig_chld_handler (int sig)
{
  if (current_trace_num >= num_iterations - 1)
    {
      /* This is the last iteration so it is normal */
    }
  else
    {
      /* The termination of the attack program is not normal at this point */
      printf ("\nEE - Attack program terminates unexpectedly!\n");

      close_everything ();
      wait (NULL);

      exit (EXIT_FAILURE);
    }
}


/*
 * This signal handler is called when the attack program dies.
 * Note: this handler can only be called in FIFO mode
 * Do some cleaning.
 */
void
sig_pipe_handler (int sig)
{
  if (current_trace_num >= num_iterations - 1)
    {
      /* This is the last iteration so it is normal */
    }
  else
    {
      /* The termination of the attack program is not normal at this point */
      printf ("\nEE - Attack program terminates unexpectedly!\n");

      close_everything ();

      exit (EXIT_FAILURE);
    }
}


/*
 * This function installs the SIGCHLD (fork mode) or SIGPIPE (FIFO mode)
 * signal handler to detect when the child (attack program) dies.
 */
void
install_sig_handler ()
{
  struct sigaction sa;

  if (fifo_mode)
    {
      /* FIFO mode */
      sa.sa_handler = sig_pipe_handler;
      sa.sa_flags = 0;
      sigemptyset (&sa.sa_mask);

      sigaction (SIGPIPE, &sa, NULL);
    }
  else
    {
      /* Fork mode */
      sa.sa_handler = sig_chld_handler;
      sa.sa_flags = 0;
      sigemptyset (&sa.sa_mask);

      sigaction (SIGCHLD, &sa, NULL);
    }
}




/*
 * This function launches the attack program. First it forks, next it
 * properly sets the file descriptors of the child so stdin, stdout
 * and stderr are redirected to the communication pipes and finally,
 * it launches the attack program.
 *
 * Used only in fork (normal) mode
 */
void
launch_attack ()
{
  /* Fork */
  process_id = fork ();

  if (process_id == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }
  if (process_id == 0)
    {
      /* Child */

      /* Preparation of file descriptors */
      close (stdin_pipe_fd[1]);
      close (stdout_pipe_fd[0]);
      close (stderr_pipe_fd[0]);
      close (0); /* stdin */
      close (1); /* stdout */
      close (2); /* stderr */

      dup2 (stdin_pipe_fd[0], 0);
      dup2 (stdout_pipe_fd[1], 1);
      dup2 (stderr_pipe_fd[1], 2);

      /* Launch of the attack program */
      execl (attack_program_filename, basename (attack_program_filename), (char *) NULL);

      /* Normally we only reach this part if execl fails */
      perror ("execl");
      exit (EXIT_FAILURE);
    }
  else
    {
      /* Parent */

      /* Close unused file descriptors */
      close (stdin_pipe_fd[0]);
      close (stdout_pipe_fd[1]);
      close (stderr_pipe_fd[1]);
    }
}



/*
 * This function waits until the attack program is ready to start the
 * attack (i.e. the attack program writes LF.LF (hex: 0A2E0A) to its
 * standard output).
 */
void
wait_start_attack_phase ()
{
  ssize_t size_read;
  struct pollfd fds[1];
  int state = 0;

  fds[0].fd = child_stdout_fd;
  fds[0].events = POLLIN;

  while (1)
    {
      if (poll (fds, 1, -1) == -1)
	{
	  perror ("poll");
	  exit (EXIT_FAILURE);
	}

      if (fds[0].revents & POLLIN)
	{
	  size_read = read (child_stdout_fd, comm_buffer, 1);
	  if (size_read == -1)
	    {
	      perror ("read");
	      exit (EXIT_FAILURE);
	    }
	  if (size_read == 0)
	    {
	      printf ("EE - Error with poll and read\n");
	      exit (EXIT_FAILURE);
	    }

	  switch (state)
	    {
	    case 0:
	      if (comm_buffer[0] == 0x0A)
		state = 1;
	      break;
	    case 1:
	      if (comm_buffer[0] == 0x2E)
		state = 2;
	      else
		state = 0;
	      break;
	    case 2:
	      if (comm_buffer[0] == 0x0A)
		state = 3;
	      else
		state = 0;
	      break;
	    }

	  if (state == 3)
	    break;
	}

      else if (fds[0].revents & POLLHUP)
	{
	  printf ("EE - Error: nothing to read!\n");
	  exit (EXIT_FAILURE);
	}
    }

  printf ("II - Start attack phase detected\n");
}


/*
 * This functions sends data to the standard input of the attack
 * program.
 *
 * buf: address of the data to send
 * count: number of bytes to send
 */
void
send_data (const void *buf, size_t count)
{
  size_t total_b_written = 0;
  ssize_t bytes_written;

  while (total_b_written < count)
    {
      bytes_written = write (child_stdin_fd,
			     ((const char *)buf) + total_b_written,
			     count - total_b_written);

      if (bytes_written == -1)
	{
	  perror("write");
	  exit (EXIT_FAILURE);
	}
      total_b_written += bytes_written;
    }
}


/*
 * This function reads data from the standard output of the attack
 * program. It blocks until count bytes are read from the attack
 * program.
 *
 * buf: address where the data read will be written
 * count: number of bytes to read
 */
void
read_data (void *buf, size_t count)
{
  size_t total_b_read = 0;
  ssize_t bytes_read;

  while (total_b_read < count)
    {
      bytes_read = read (child_stdout_fd,
			 ((char *)buf) + total_b_read,
			 count - total_b_read);

      if (bytes_read == -1)
	{
	  perror("read");
	  exit (EXIT_FAILURE);
	}
      total_b_read += bytes_read;
    }
}


/*
 * Record the start time of the analysis of a trace
 */
void
start_timer ()
{
  gettimeofday (&start_time, NULL);
}


/*
 * Record the end time of the analysis of a trace and compute the time consumed
 */
void
end_timer ()
{
  struct timeval end_time;

  double start_time_d, end_time_d;

  gettimeofday (&end_time, NULL);

  start_time_d = (double)start_time.tv_sec + ((double)start_time.tv_usec * 1e-6);
  end_time_d = (double)end_time.tv_sec + ((double)end_time.tv_usec * 1e-6);
  time_consumed = end_time_d - start_time_d;
}



/*
 * This function sends the next trace to the attack program.
 */
void
send_trace ()
{
  printf ("/ Sending trace ");
  fflush (stdout);

  start_timer ();

  send_data (current_trace.plaintext, sizeof (current_trace.plaintext));
  send_data (current_trace.ciphertext, sizeof (current_trace.ciphertext));
  send_data (current_trace.samples, sizeof (current_trace.samples));
}


/*
 * This function dumps the result of the attack to a result file.
 *
 * result: the partial result sent by the attack program to dump
 */
void
dump_result_to_file (struct attack_partial_result *result)
{
  int key = 0;
  int index = 0;

  if (current_trace_num == 0)
    {
      subkey_num = result->subkey_num;
      AES_key_schedule (correct_key, correct_subkey, subkey_num);

      if (output_binary)
	{
	  /* Binary output */
	  struct result_file_header header;
	  header.format_version = RESULTS_BINARY_FORMAT_VERSION;
	  header.num_iterations = num_iterations;
	  header.num_dbkey = num_dbkey;
	  header.subkey_num = subkey_num;
	  memcpy (header.correct_subkey, correct_subkey, sizeof (header.correct_subkey));
	  memcpy (header.correct_key, correct_key, sizeof (header.correct_key));

	  fwrite (&header, sizeof (header), 1, result_file);
	}
      else
	{
	  /* Text output */
	  fprintf (result_file, " Attacked subkey #%hhu\n", subkey_num);
	  fprintf (result_file,
		   " Correct subkey: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n\n",
		   correct_subkey[0],
		   correct_subkey[1],
		   correct_subkey[2],
		   correct_subkey[3],
		   correct_subkey[4],
		   correct_subkey[5],
		   correct_subkey[6],
		   correct_subkey[7],
		   correct_subkey[8],
		   correct_subkey[9],
		   correct_subkey[10],
		   correct_subkey[11],
		   correct_subkey[12],
		   correct_subkey[13],
		   correct_subkey[14],
		   correct_subkey[15]);
	}

    }

  if (output_binary)
    {
      /* Binary output */
      struct result_file_trace_header trace_header;
      trace_header.time_consumed = time_consumed;
      memcpy (trace_header.plaintext, current_trace.plaintext, sizeof (trace_header.plaintext));
      memcpy (trace_header.ciphertext, current_trace.ciphertext, sizeof (trace_header.ciphertext));

      fwrite (&trace_header, sizeof (trace_header), 1, result_file);
      fwrite (result, sizeof (struct attack_partial_result), 1, result_file);
    }
  else
    {
      /* Text output */
      fprintf (result_file, "### Trace #%04i ###\n", current_trace_num);

      fprintf (result_file,
	       "Plaintext: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n",
	       current_trace.plaintext[0],
	       current_trace.plaintext[1],
	       current_trace.plaintext[2],
	       current_trace.plaintext[3],
	       current_trace.plaintext[4],
	       current_trace.plaintext[5],
	       current_trace.plaintext[6],
	       current_trace.plaintext[7],
	       current_trace.plaintext[8],
	       current_trace.plaintext[9],
	       current_trace.plaintext[10],
	       current_trace.plaintext[11],
	       current_trace.plaintext[12],
	       current_trace.plaintext[13],
	       current_trace.plaintext[14],
	       current_trace.plaintext[15]);

      fprintf (result_file,
	       "Ciphertext: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n",
	       current_trace.ciphertext[0],
	       current_trace.ciphertext[1],
	       current_trace.ciphertext[2],
	       current_trace.ciphertext[3],
	       current_trace.ciphertext[4],
	       current_trace.ciphertext[5],
	       current_trace.ciphertext[6],
	       current_trace.ciphertext[7],
	       current_trace.ciphertext[8],
	       current_trace.ciphertext[9],
	       current_trace.ciphertext[10],
	       current_trace.ciphertext[11],
	       current_trace.ciphertext[12],
	       current_trace.ciphertext[13],
	       current_trace.ciphertext[14],
	       current_trace.ciphertext[15]);

      fprintf (result_file, "Time: %f s\n", time_consumed);


      for (key = 0; key < 16; key++)
	{
	  fprintf (result_file, "Subkey byte #%02i: ", key + 1);
	  for (index = 0; index < 256; index++)
	    {
	      fprintf (result_file, "%02X ", result->bytes[key][index]);
	    }
	  fprintf (result_file, "\n");
	}

      fprintf (result_file, "\n");
    }
}


/*
 * This function prints the header of the result file
 */
void
print_result_file_header ()
{
  if (! output_binary) {
    fprintf (result_file, "Attack parameters:\n");
    fprintf (result_file, " Wrapper version: %s\n", PACKAGE_VERSION);
    fprintf (result_file, " Num iterations: %hu\n", num_iterations);
    fprintf (result_file, " Num key: %hu\n", num_dbkey);
    fprintf (result_file, " Key: %s\n", correct_key_hex);
  }
}



/*
 * This function read the partial result from the attack program.
 */
void
read_result ()
{
  struct attack_partial_result result;

  read_data (&result, sizeof (result));

  end_timer ();

  printf ("/ Results received\n");

  dump_result_to_file (&result);
}


/*
 * This function sends the number of traces that will be send to the
 * attack program.
 */
void
send_num_iterations ()
{
  printf ("II - Sending # of iterations\n");

  send_data (&num_iterations, sizeof (num_iterations));
}




/*
 * This function is the main function of the thread managing the trace retrival
 */
void *
trace_thread_routine (void * arg)
{
  uint16_t trace_num;

  for (trace_num = 0; trace_num < num_iterations; trace_num++)
    {
      /* Wait until the main thread has copied the lastest trace */
      sem_wait (&sem_num_trace_to_write);

      /* Retrieve the next trace from the DB/files */
      if (trace_dir == NULL)
	db_get_trace (trace_num);
      else
	trace_dir_get_trace (trace_num);

      /* Signal to the main thread that it can use the new trace */
      sem_post (&sem_num_trace_to_read);
    }


  return NULL;
}


/*
 * Retrieve a trace from the trace thread
 */
void
get_trace_from_trace_thread ()
{
  printf ("Trace thread ");
  fflush (stdout);

  /* Wait until the trace thread has retrieved a trace */
  sem_wait (&sem_num_trace_to_read);

  /* Copy the trace retrieve by the trace thread */
  memcpy (&current_trace, &next_trace, sizeof (current_trace));

  /* Signal to the trace thread that it can retrieve a new trace */
  sem_post (&sem_num_trace_to_write);
}



/*
 * This function starts the thread managing the traces
 */
void
launch_trace_thread ()
{
  /* Create the two synchronization semaphores */
  if (sem_init (&sem_num_trace_to_read, 0, 0) < 0)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }

  if (sem_init (&sem_num_trace_to_write, 0, 1) < 0)
    {
      perror ("sem_init");
      exit (EXIT_FAILURE);
    }

  /* Create the DB thread */
  if (pthread_create (&trace_thread, NULL, &trace_thread_routine, NULL) < 0)
    {
      perror ("pthread_create");
      exit (EXIT_FAILURE);
    }

  trace_thread_launched = 1;
}



/*
 * This function waits until the trace thread terminates
 */
void
wait_trace_thread_termination ()
{
  if (trace_thread_launched)
    {
      pthread_join (trace_thread, NULL);

      sem_destroy (&sem_num_trace_to_read);
      sem_destroy (&sem_num_trace_to_write);
    }
}



/*
 * This function displays how to use the attack wrapper.
 */
void
print_usage (char *argv0)
{
  printf ("Usage: %s [OPTION]... FILENAME\n", argv0);
  printf ("Launch the attack program. FILENAME represents the name of the executable\n");
  printf ("file containing the attack program (except in FIFO mode).\n\n");
  printf ("The options are:\n");
  printf ("\t-i, --iterations=NUM\tProvide NUM traces to the attack program (1-%i, default: %i)\n", MAX_ITERATIONS, NUM_ITERATIONS);
  printf ("\t-k, --key=NUM\t\tUse the key number NUM in the base (0-%i, default: %i)\n", MAX_NUM_DBKEY, NUM_DBKEY);
  printf ("\t-o, --output=FILENAME\tStore the results of the attack in FILENAME (default: results)\n");
  printf ("\t-b, --binary\t\tOutput results in binary format (default: false)\n");
  printf ("\t-d, --trace-dir=DIR\tPath to the directory containg the traces (default: read from DB)\n");
  printf ("\t-x, --dir-index=FILENAME\tPath to the trace directory index file\n");
  printf ("\t-f, --fifo\t\tFIFO mode (FILENAME represents the base name for the 2 FIFOs)\n");
  printf ("\t-h, --help\t\tPrint this help message\n");
  printf ("\t-v, --version\t\tPrint the version of the attack wrapper\n");
}


/*
 * This function displays the version of the attack wrapper.
 */
void
print_version ()
{
  printf ("Attack wrapper\n");
  printf ("Version: %s\n", PACKAGE_VERSION);
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
	  {"iterations", required_argument, 0, 'i'},
	  {"key", required_argument, 0, 'k'},
	  {"output", required_argument, 0, 'o'},
	  {"trace_dir", required_argument, 0, 'd'},
	  {"dir_index", required_argument, 0, 'x'},
	  {"help", no_argument, 0, 'h'},
	  {"version", no_argument, 0, 'v'},
	  {"binary", no_argument, &output_binary, 1},
	  {"fifo", no_argument, &fifo_mode, 1},
	  {0, 0, 0, 0}
	};

      int option_index = 0;

      c = getopt_long (argc, argv, "i:k:o:d:x:hvbf", options, &option_index);

      if (c == -1)
	break;

      switch (c)
	{
	case 'i':
	  num_iterations = atoi (optarg);
	  break;
	case 'k':
	  num_dbkey = atoi (optarg);
	  break;
	case 'o':
	  output_filename = strdup (optarg);
	  break;
	case 'd':
	  trace_dir = strdup (optarg);
	  break;
	case 'x':
	  trace_dir_index_filename = strdup (optarg);
	  break;
	case 'h':
	  print_usage (argv[0]);
	  exit (EXIT_SUCCESS);
	  break;
	case 'v':
	  print_version ();
	  exit (EXIT_SUCCESS);
	  break;
	case 'b':
	  output_binary = 1;
	  break;
	case 'f':
	  fifo_mode = 1;
	  break;
	}
    }

  if (optind >= argc)
    {
      printf ("EE - The name of the attack program is missing!\n\n");
      print_usage (argv[0]);
      exit (EXIT_FAILURE);
    }

  attack_program_filename = argv[optind];


  /* Do some checks */
  if ((num_iterations < 1) || (num_iterations > MAX_ITERATIONS))
    {
      printf ("EE - The number of iterations is out of range (1-%i)!\n", MAX_ITERATIONS);
      exit (EXIT_FAILURE);
    }
  if (num_dbkey > MAX_NUM_DBKEY)
    {
      printf ("EE - The number of the key is out of range (0-%i)!\n", MAX_NUM_DBKEY);
      exit (EXIT_FAILURE);
    }

  if ((trace_dir_index_filename != NULL) && (trace_dir == NULL))
    {
      printf ("EE - Index file supplied but no trace directory!\n");
      printf ("EE - Please use the --trace-dir option.\n");
      exit (EXIT_FAILURE);
    }

  /* Outputs the options chosen */
  printf ("DD - # iterations = %hu\n", num_iterations);
  printf ("DD - # key = %hu\n", num_dbkey);
  printf ("DD - Output filename = %s (%s)\n", output_filename, output_binary ? "binary" : "text");
  if (fifo_mode == 0)
    {
      printf ("DD - Normal mode (fork mode)\n");
      printf ("DD - Attack program = %s\n", attack_program_filename);
    }
  else
    {
      printf ("DD - FIFO mode\n");
      printf ("DD - Base name for FIFOs = %s\n", attack_program_filename);
    }


  if (trace_dir == NULL)
    printf ("DD - Traces will be read from DB\n");
  else
    {
      printf ("DD - Traces will be read from directory %s\n", trace_dir);
      if (trace_dir_index_filename == NULL)
	printf ("DD - No index filename (directory listing may be slow)\n");
      else
	printf ("DD - Using index file %s\n", trace_dir_index_filename);
    }
}




/*
 * The main function of the program.
 */
int
main (int argc, char **argv)
{
  /* Do some checks to verify the proper alignemnt of data structure */
  assert (sizeof (struct attack_partial_result) == 16*256+1);
  assert (sizeof (struct attack_trace) == TRACE_PLAINTEXT_SIZE_BYTES
	  + TRACE_CIPHERTEXT_SIZE_BYTES + 2 * TRACE_NUM_POINTS);

  assert (sizeof (struct result_file_header) == 2*TRACE_KEY_SIZE_BYTES+4*4);
  assert (sizeof (struct result_file_trace_header) == TRACE_PLAINTEXT_SIZE_BYTES
	  + TRACE_CIPHERTEXT_SIZE_BYTES + sizeof(double));

  parse_command_line (argc, argv);

  result_file = fopen (output_filename, "w");
  if (result_file == NULL)
    {
      perror ("fopen (result file)");
      exit (EXIT_FAILURE);
    }

  create_communication_pipes ();

  if (trace_dir == NULL)
    {
      db_connect ();
      db_findkey ();
    }
  else
    {
      trace_dir_init ();
      trace_dir_findkey ();
    }

  print_result_file_header ();

  install_sig_handler ();

  if (fifo_mode == 0)
    launch_attack ();

  send_num_iterations ();
  wait_start_attack_phase ();

  launch_trace_thread ();

  for (current_trace_num = 0; current_trace_num < num_iterations; current_trace_num++)
    {
      printf ("II - Trace #%hu: ", current_trace_num);

      get_trace_from_trace_thread ();

      send_trace ();
      read_result ();
    }

  close_everything ();

  wait_trace_thread_termination ();

  if (fifo_mode == 0)
    {
      printf ("II - Waiting the end of the attack program...\n");

      wait (NULL); /* Wait until the child (attack program) is finished */
    }

  exit (EXIT_SUCCESS);
}
