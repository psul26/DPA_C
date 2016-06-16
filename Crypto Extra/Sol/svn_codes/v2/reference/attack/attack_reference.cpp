/**
 * @file   attack_reference.cpp
 * @brief  This file builds a program that communicates with the attack wrapper,
 *         and implements the CPA on the last round of the AES.
 * @author Sylvain GUILLEY, <sylvain.guilley@TELECOM-ParisTech.fr>
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

extern "C"
{
	#include "dpa_contest.h"
}
#include "attack_last_round.h" // Attack that is really implemented

/* Information gotten from the attack wrapper */
uint16_t num_iterations;
struct attack_trace current_trace;
/* Information sent to the attack wrapper */
uint8_t const ready[] = { 0x0au, 0x2eu, 0x0au };
struct attack_partial_result result;

/*
 * This function sends data to the standard input of the attack
 * program.
 *
 * buf: address of the data to send
 * count: number of bytes to send
 */
void
write_data (const void *buf, size_t count)
{
	size_t total_b_written = 0;
	ssize_t bytes_written;

	while (total_b_written < count)
	{
		bytes_written = write ( 1, (char*)buf + total_b_written, count - total_b_written);
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
		bytes_read = read ( 0, (char*)buf + total_b_read, count - total_b_read);
		if (bytes_read == -1)
		{
			perror("read");
			exit (EXIT_FAILURE);
		}
		total_b_read += bytes_read;
	}
}

int main( void )
{
	uint16_t current_trace_num;

	/* Implementation of the communication protocole with the attack wrapper */

	read_data( &num_iterations, sizeof( num_iterations ));
	write_data( ready, 3 );

	attack_last_round_init( );
	for (current_trace_num = 0; current_trace_num < num_iterations; current_trace_num++)
	{
		read_data( &current_trace.plaintext,  sizeof( current_trace.plaintext  ));
		read_data( &current_trace.ciphertext, sizeof( current_trace.ciphertext ));
		read_data( &current_trace.samples,    sizeof( current_trace.samples    ));

		result = attack_last_round( current_trace.ciphertext, current_trace.samples ); /* Attack */

		write_data( &result, sizeof( result ));
	}

	return EXIT_SUCCESS;
}
