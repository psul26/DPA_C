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


#ifndef DB_POSTGRESQL_H
#define DB_POSTGRESQL_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */


#ifdef HAVE_POSTGRESQL
#include "libpq-fe.h"
#endif /* HAVE_POSTGRESQL */

#include "dpa_contest.h"


/* Global variables of the attack wrapper */
extern uint16_t num_dbkey;
extern char correct_key_hex[2*TRACE_KEY_SIZE_BYTES + 1];
extern uint8_t correct_key[TRACE_KEY_SIZE_BYTES];
extern struct attack_trace next_trace;


/* DB parameters (public table) */
extern char *db_host;
extern char *db_user;
extern char *db_pwd;
extern char *db_base;
extern char *db_table;
extern char *db_key;


/*
 * This function connects to the public database.
 */
void
db_connect ();


/*
 * Retrieve the key to use from the DB and the key number.
 */
void
db_findkey ();


/*
 * This functions retrieves a trace from the database.
 * trace_num: Number of trace to retrieve from the DB
 */
void
db_get_trace (uint16_t trace_num);


/*
 * This function closes the connection to the public database.
 */
void
db_close ();


#endif /* DB_POSTGRESQL_H */
