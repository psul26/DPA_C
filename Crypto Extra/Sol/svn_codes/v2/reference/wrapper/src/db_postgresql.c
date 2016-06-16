/*
 * PostgreSQL functions used by the attack wrapper
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
#include <string.h>

#include "db_postgresql.h"
#include "utils.h"

#ifdef HAVE_POSTGRESQL
#include "libpq-fe.h"
#endif /* HAVE_POSTGRESQL */


/* DB parameters (public table) */
char *db_host = "dpa.enst.fr";
char *db_user = "guest";
char *db_pwd = "guest";
char *db_base = "production_traces";
char *db_table = "DPA_CONTEST_V2_public_base";
char *db_key = "index";


#ifdef HAVE_POSTGRESQL

static PGconn *db_connection;
static PGresult *db_query_result;



/*
 * This function connects to the public database.
 */
void
db_connect ()
{
  char connection_string[256];

  snprintf (connection_string, 256, "host=%s user=%s password=%s dbname=%s",
	    db_host, db_user, db_pwd, db_base);

  printf ("II - Connecting to database... ");

  db_connection = PQconnectdb (connection_string);

  if (PQstatus (db_connection) != CONNECTION_OK)
    {
      printf (" FAILED! (%s)", PQerrorMessage (db_connection));
      exit (EXIT_FAILURE);
    }
  printf ("Ok\n");
}


/*
 * Retrieve the key to use from the DB and the key number.
 */
void
db_findkey ()
{
  PGresult *db_query_res;
  char *data;
  char query[256];

  snprintf (query, 256, "SELECT key FROM \"%s\" GROUP BY key ORDER BY key LIMIT 1 OFFSET %hu",
	    db_table, num_dbkey);

  printf ("II - Database query to retrieve the key... ");
  fflush (stdout);

  db_query_res = PQexec (db_connection, query);

  if ((db_query_res == NULL) || (PQresultStatus (db_query_res) != PGRES_TUPLES_OK))
    {
      printf (" FAILED! (%s)\n", PQerrorMessage (db_connection));
      exit (EXIT_FAILURE);
    }


  if (PQntuples (db_query_res) != 1)
    {
      printf (" FAILED! (no tuple returned by the DB!)\n");
      exit (EXIT_FAILURE);
    }


  data = PQgetvalue (db_query_res,
		     0,
		     0);

  strncpy (correct_key_hex, data, sizeof (correct_key_hex));

  hex2uint8_16bytes (data, correct_key);

  PQclear (db_query_res);

  printf ("Ok (key = %s)\n", correct_key_hex);
}


/*
 * This function performs the SQL query to ask for a trace from the
 * public database.
 * trace_num: Number of trace to retrieve from the DB
 */
void
db_query (uint16_t trace_num)
{
  char query[256];

  snprintf (query, 256, "SELECT message,ciphertext,key,trace FROM \"%s\" WHERE \"key\"='%s' ORDER BY \"%s\" LIMIT 1 OFFSET %i",
	    db_table, correct_key_hex, db_key, trace_num);

  db_query_result = PQexec (db_connection, query);

  if ((db_query_result == NULL) || (PQresultStatus (db_query_result) != PGRES_TUPLES_OK))
    {
      printf ("DB FAILED! (Trace #%hu, Message: %s)\n",
	      trace_num,
	      PQerrorMessage (db_connection));

      exit (EXIT_FAILURE);
    }

  if (PQntuples (db_query_result) != 1)
    {
      printf ("DB FAILED! (Trace #%hu, no tuple returned by the DB!)\n",
	      trace_num);

      exit (EXIT_FAILURE);
    }
}


/*
 * This functions retrieves a trace from the database.
 * trace_num: Number of trace to retrieve from the DB
 */
void
db_get_trace (uint16_t trace_num)
{
  char *data;
  unsigned char *bin_data;
  size_t bin_data_length;


  /* Retrieve the trace from the DB */
  db_query (trace_num);


  /* Message (32 hexa digits) */
  data = PQgetvalue (db_query_result,
		     0,
		     0);

  hex2uint8_16bytes (data, next_trace.plaintext);


  /* Ciphertext (32 hexa digits) */
  data = PQgetvalue (db_query_result,
		     0,
		     1);

  hex2uint8_16bytes (data, next_trace.ciphertext);


  /* Trace (bytea) */
  data = PQgetvalue (db_query_result,
		     0,
		     3);

  bin_data = PQunescapeBytea ((unsigned char *) data, &bin_data_length);

  /*
  printf ("DD - length = %i / Type = %i / bin_data_length = %zi\n",
	  PQgetlength (db_query_result, 0, 3),
	  PQfformat (db_query_result, 3),
	  bin_data_length); */

  if (bin_data_length != sizeof (next_trace.samples))
    {
      printf ("EE - Wrong trace length!\n");
      exit (EXIT_FAILURE);
    }

  memcpy (next_trace.samples, bin_data, bin_data_length);

  PQfreemem (bin_data);

  PQclear (db_query_result);
}


/*
 * This function closes the connection to the public database.
 */
void
db_close ()
{
  if (db_connection != NULL)
    {
      printf ("II - Closing database connection... ");

      PQfinish (db_connection);
      db_connection = NULL;

      printf ("Ok\n");
    }
}


#else /* HAVE_POSTGRESQL */

static void
not_compiled_with_db ()
{
  printf ("EE - The wrapper was not compiled with DB support!\n");
  printf ("EE - Please use the --trace-dir option to provide a directory with the traces\n");

  exit (EXIT_FAILURE);
}

void
db_connect ()
{
  not_compiled_with_db ();
}

void
db_findkey ()
{
  not_compiled_with_db ();
}

void
db_get_trace (uint16_t trace_num)
{
  not_compiled_with_db ();
}

void
db_close ()
{
  not_compiled_with_db ();
}


#endif /* HAVE_POSTGRESQL */
