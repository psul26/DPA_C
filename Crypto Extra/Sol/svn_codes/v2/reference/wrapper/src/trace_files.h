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


#ifndef TRACE_FILES_H
#define TRACE_FILES_H

#include <stdint.h>

#include <dpa_contest.h>


/* Global variables of the attack wrapper */
extern uint16_t num_dbkey;
extern char correct_key_hex[2*TRACE_KEY_SIZE_BYTES + 1];
extern uint8_t correct_key[TRACE_KEY_SIZE_BYTES];
extern struct attack_trace next_trace;


/* Path to trace directory and index file */
extern char *trace_dir;
extern char *trace_dir_index_filename;


/* Structure representing a trace file */
struct trace_file
{
  uint8_t key[TRACE_KEY_SIZE_BYTES];
  uint8_t plaintext[TRACE_PLAINTEXT_SIZE_BYTES];
  uint8_t ciphertext[TRACE_CIPHERTEXT_SIZE_BYTES];
  char filename[256];
  char key_hex[2*TRACE_KEY_SIZE_BYTES + 1];
};


/*
 * Initialize the list of trace filenames
 */
void
trace_dir_init ();


/*
 * Find the key to attack in the trace files list
 */
void
trace_dir_findkey ();


/*
 * This function retrieves a trace from a file
 * trace_num: Number of trace to retrieve from the disk
 */
void
trace_dir_get_trace (uint16_t trace_num);


/*
 * This function cleans some things
 */
void
trace_dir_close ();



#endif /* TRACE_FILES_H */
