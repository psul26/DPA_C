/**********************************************************************************
Copyright Institut Telecom
Contributors: Renaud Pacalet (renaud.pacalet@telecom-paristech.fr)

This software is a computer program whose purpose is to exploit power traces
in order to retrieve a Data Encryption Standard (DES) secret key.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms. For more
information see the LICENCE-fr.txt or LICENSE-en.txt files.
**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <libpq-fe.h>
#define __USE_BSD
#include <termios.h>
#include <unistd.h>

#include <utils.h>
#include <traces.h>

/* Maximum output file size before warning and confirmation request (GB) */
#define WARNFILESIZE 1.0

/* Maximum byte memory usage before warning and confirmation request */
#define WARNMEMSIZE 0.25

/* Maximum number of traces per PosGreSQL query */
#define MAXTRACESPERQUERY 100

/* Prints usage message and exits */
void
usage ()
{
  fprintf (stderr, "\n\
usage: fetch <tablename> <filename> [<numtraces>] [<firsttrace>]\n\
  <tablename>:  name of the PosGreSQL table. secmatv1_2006_04_0809,\n\
                secmatv3_20070924_des and secmatv3_20071219_des are the currently\n\
                available tables.\n\
  <filename> :  name of the output binary trace file.\n\
  <numtraces>:  number of traces to fetch. If ommited all the traces in the\n\
                database table are fetched. Warning: this can be huge!\n\
  <firsttrace>: first trace to fetch. If ommited or set to 0, the first trace\n\
                in the database table is the first.\n\
  ");
  exit (-1);
}

char *tablename;		/* Name of the database table */
char *filename;			/* Name of the binary trace file */
int numtraces;			/* Number of traces to fetch */
int firsttrace;			/* Index of first trace to fetch */
int length;			/* Number of samples per power trace */
int offset;			/* Byte offset of the first sample of the power traces in the
				   Agilent binary format */
PGconn *conn;			/* PostGreSQL connection handler */
FILE *fp;			/* File descriptor of output trace file */
uint64_t key;			/* Secret key */

/* Connect to database, query number of rows in table, query first row and
 * retrieves secret key, number of samples per power trace and byte offset of
 * the first sample of the power traces in the Agilent binary format */
void
init (void)
{
  PGresult *res;
  char *cmd, *frm, *tmp;
  int i;

  /* Connect to database */
  conn = PQconnectdb ("host=dpa.enst.fr user=guest password=guest\
      dbname=production_traces");
  if (PQstatus (conn) != CONNECTION_OK)
    error (__func__, "connection to database failed (%s)\n",
	   PQerrorMessage (conn));
  /* Query number of rows in table */
  frm = "SELECT COUNT(*) AS numtraces FROM %s";
  cmd = malloc (strlen (frm) + strlen (tablename) - 1);
  if (cmd == NULL)
    error (__func__, "cannot allocate memory");
  sprintf (cmd, frm, tablename);	/* Form query string */
  res = PQexec (conn, cmd);	/* Send query to PosGreSQL server */
  if (PQresultStatus (res) != PGRES_TUPLES_OK)
    error (__func__, "SQL command '%s' failed (%s)\n", cmd,
	   PQerrorMessage (conn));
  if (PQntuples (res) != 1)
    error (__func__, "SQL command '%s' failed (got %d rows)\n", cmd,
	   PQntuples (res));
  if (PQnfields (res) != 1)
    error (__func__, "SQL command '%s' failed (got %d fields)\n", cmd,
	   PQnfields (res));
  i = PQfnumber (res, "numtraces");
  if (i == -1)
    error (__func__, "SQL command '%s' failed (column name mismatch)\n", cmd);
  tmp = PQgetvalue (res, 0, i);
  if (sscanf (tmp, "%d", &i) != 1)
    error (__func__,
	   "SQL command '%s' failed (non integer returned value %s)\n", cmd,
	   tmp);
  if (firsttrace >= i)
    error (__func__, "first trace index (%d) exceeds database size (%d)",
	   firsttrace, i);
  /* If number of traces is not specified on command line, or given a zero
   * value, fetch the maximum number of traces */
  if (numtraces == 0)
    numtraces = i - firsttrace;
  if (numtraces + firsttrace > i)	/* If required number of traces exceeds table depth */
    error (__func__,
	   "table contains %d rows, cannot fetch %d rows, starting from row #%d",
	   i, numtraces, firsttrace);
  free (cmd);			/* Free query string */
  PQclear (res);		/* No memory leaks in my code */

  /* Query secret key and first power trace. We use PQexecParams to specify that
   * the binary column (filecontent) shall be sent back in binary form */
  frm = "SELECT key, filecontent FROM %s LIMIT 1 OFFSET %d";
  if (firsttrace == 0)
    i = 1;
  else
    i = (int) (log10 ((double) (firsttrace)));
  cmd = malloc (strlen (frm) + strlen (tablename) + i - 3);
  if (cmd == NULL)
    error (__func__, "cannot allocate memory");
  sprintf (cmd, frm, tablename, firsttrace);	/* Form query string */
  /* Send query to PosGreSQL server */
  res = PQexecParams (conn, cmd, 0, NULL, NULL, NULL, NULL, 1);
  if (PQresultStatus (res) != PGRES_TUPLES_OK)
    error (__func__, "SQL command '%s' failed (%s)\n", cmd,
	   PQerrorMessage (conn));
  if (PQntuples (res) != 1)
    error (__func__, "SQL command '%s' failed (got %d rows)\n", cmd,
	   PQntuples (res));
  if (PQnfields (res) != 2)
    error (__func__, "SQL command '%s' failed (got %d fields)\n", cmd,
	   PQnfields (res));
  i = PQfnumber (res, "key");
  if (i == -1)
    error (__func__, "SQL command '%s' failed (column name mismatch)\n", cmd);
  tmp = PQgetvalue (res, 0, i);
  if (sscanf (tmp, "%016llx", &key) != 1)
    error (__func__,
	   "SQL command '%s' failed (non integer returned value %s)\n", cmd,
	   tmp);
  i = PQfnumber (res, "filecontent");
  if (i == -1)
    error (__func__, "SQL command '%s' failed (column name mismatch)\n", cmd);
  tmp = PQgetvalue (res, 0, i);	/* Raw data in Agilent format */
  offset = 12;			/* Skip global raw binary string header */
  offset += *((uint32_t *) (tmp + offset));	/* Skip waveform header */
  offset += *((uint32_t *) (tmp + offset));	/* Skip data header. offset is now
						   the byte offset from beginning of
						   raw data */
  length = *((uint32_t *) (tmp + offset - 4)) / 4;	/* Number of floats in each
							   power trace */
  free (cmd);			/* Free query string */
  PQclear (res);		/* No memory leaks in my code */
}

/* Waits for user keystroke, returns on 'y' or 'Y', exits on 'n' or 'N',
 * continue waiting on other keystrokes */
void
cont (void)
{
  struct termios t1, t2;
  int c;

  tcgetattr (STDIN_FILENO, &t1);
  t2 = t1;
  cfmakeraw (&t2);
  tcsetattr (STDIN_FILENO, TCSANOW, &t2);
  while (1)
    {
      c = getchar ();
      if (c == 'n' || c == 'N' || c == 'y' || c == 'Y')
	break;
    }
  tcsetattr (STDIN_FILENO, TCSANOW, &t1);
  fprintf (stdout, "\n");
  if (c == 'n' || c == 'N')
    exit (0);
}

/* Wrap everything together */
int
main (int argc, char **argv)
{
  int i, j, k, ip, ic, it;	/* Loop indices, table columns indices */
  PGresult *res;		/* PosGreSQL query resul handler */
  char *cmd, *frm, *tmp;	/* Query and result strings */
  uint64_t p, c;		/* Plain and cipher texts */
  double w;			/* File size and memory usage */
  FILE *fp;			/* Output file descriptor */

  if (argc < 3 || argc > 5)
    usage ();
  tablename = argv[1];
  filename = argv[2];
  numtraces = 0;
  firsttrace = 0;
  if (argc >= 4)
    {				/* If number of traces specified on command line */
      numtraces = atoi (argv[3]);
      if (numtraces < 0)
	error (__func__, "Illegal number of required traces (%d)", numtraces);
    }
  if (argc == 5)
    {				/* If first trace specified on command line */
      firsttrace = atoi (argv[4]);
      if (firsttrace < 0)
	error (__func__, "Illegal first trace index (%d)", firsttrace);
    }
  init ();			/* Intialization */
  /* Print out some information */
  fprintf (stdout, "Number of traces: %u\n", numtraces);
  fprintf (stdout, "Number of samples per traces: %u\n", length);
  fprintf (stdout, "Secret key: %016llx\n", key);
  /* Estimated size of output file */
  w = numtraces;
  w *= (2 * 8 + 4 * length);
  w += 5 + 2 * 4 + 8;
  w /= 1073741824.0;
  fprintf (stdout, "The output file size will be %.2f GB.\n", w);
  if (w > WARNFILESIZE)
    {				/* If file size exceeds the warning limit */
      fprintf (stdout, "Are you sure you want to continue (yn)? ");
      cont ();			/* Ask for a confirmation */
    }
  /* Estimated memory usage */
  w = MAXTRACESPERQUERY;
  w *= (2 * 8 + 4 * length);
  w /= 1073741824.0;
  fprintf (stdout, "The memory usage will be about %.2f GB.\n", w);
  if (w > WARNMEMSIZE)
    {				/* If memory usage exceeds the warning limit */
      fprintf (stdout, "Are you sure you want to continue (yn)? ");
      cont ();			/* Ask for a confirmation */
    }

  /* Open output file and write file header */
  fp = fopen (filename, "w");
  if (fp == NULL)
    error (__func__, "cannot open file %s", filename);
  fprintf (fp, "%s", HWSECMAGICNUMBER);	/* Magic number */
  fwrite (&numtraces, sizeof (numtraces), 1, fp);	/* Number of traces */
  fwrite (&length, sizeof (length), 1, fp);	/* Number of samples per traces */
  fwrite (&key, sizeof (key), 1, fp);	/* Secret key */

  /* Fetch plaintexts, ciphertexts and power traces from database,
   * MAXTRACESPERQUERY at a time, and write them in the output file. */
  for (i = firsttrace; i < firsttrace + numtraces; i += MAXTRACESPERQUERY)
    {
      j = firsttrace + numtraces - i;	/* Number of remaining traces to fetch */
      if (j == 0)
	break;			/* No more traces to fetch */
      /* Number of traces to read next */
      j = j < MAXTRACESPERQUERY ? j : MAXTRACESPERQUERY;
      /* Form PosGreSQL query */
      frm =
	"SELECT message, cryptogram, filecontent FROM %s LIMIT %d OFFSET %d";
      cmd =
	malloc (strlen (frm) + strlen (tablename) +
		(int) (log10 ((double) (j))) +
		(int) (log10 ((double) (i + 1))) + 1);
      if (cmd == NULL)
	error (__func__, "cannot allocate memory");
      sprintf (cmd, frm, tablename, j, i);
      /* Send query */
      printf ("rows %d to %d... ", i + 1, i + j);
      fflush (stdout);
      res = PQexecParams (conn, cmd, 0, NULL, NULL, NULL, NULL, 1);
      if (PQresultStatus (res) != PGRES_TUPLES_OK)
	error (__func__, "SQL command '%s' failed (%s)\n", cmd,
	       PQerrorMessage (conn));
      if (PQntuples (res) != j)
	error (__func__, "SQL command '%s' failed (got %d rows)\n", cmd,
	       PQntuples (res));
      if (PQnfields (res) != 3)
	error (__func__, "SQL command '%s' failed (got %d fields)\n", cmd,
	       PQnfields (res));
      ip = PQfnumber (res, "message");	/* Index of plaintext in result array */
      if (ip == -1)
	error (__func__, "SQL command '%s' failed (column name mismatch)\n",
	       cmd);
      ic = PQfnumber (res, "cryptogram");	/* Index of ciphertext in result array */
      if (ic == -1)
	error (__func__, "SQL command '%s' failed (column name mismatch)\n",
	       cmd);
      it = PQfnumber (res, "filecontent");	/* Power trace index in result array */
      if (it == -1)
	error (__func__, "SQL command '%s' failed (column name mismatch)\n",
	       cmd);
      for (k = 0; k < j; k++)
	{			/* For all fetched rows */
	  tmp = PQgetvalue (res, k, ip);	/* tmp points to plain text ASCII value */
	  if (sscanf (tmp, "%016llx", &p) != 1)
	    error (__func__,
		   "SQL command '%s' failed (non integer returned value %s)\n",
		   cmd, tmp);
	  fwrite (&p, sizeof (p), 1, fp);	/* Write plain text */
	  tmp = PQgetvalue (res, k, ic);	/* tmp points to cipher text ASCII value */
	  if (sscanf (tmp, "%016llx", &c) != 1)
	    error (__func__,
		   "SQL command '%s' failed (non integer returned value %s)\n",
		   cmd, tmp);
	  fwrite (&c, sizeof (c), 1, fp);	/* Write cipher text */
	  tmp = PQgetvalue (res, k, it);	/* tmp points to binary power trace */
	  fwrite (tmp + offset, sizeof (float), length, fp);	/* Write power trace */
	}
      free (cmd);		/* Free query string */
      PQclear (res);		/* Avoid memory leaks */
      printf ("done\n");
    }
  PQfinish (conn);		/* Close PosGreSQL connection */
  fclose (fp);			/* Close trace file */
  return 0;
}
