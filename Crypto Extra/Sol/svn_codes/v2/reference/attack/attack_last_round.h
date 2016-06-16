#ifndef ATTACK_LAST_ROUND_H
#define ATTACK_LAST_ROUND_H

#include "dpa_contest.h"
#include <stdio.h>

// Initialization of the permanent structures (t, t2, w, w2 and tw)
void attack_last_round_init( );
// Stepping with a new trace
attack_partial_result attack_last_round(
	uint8_t ciphertext[TRACE_CIPHERTEXT_SIZE_BYTES], int16_t points[TRACE_NUM_POINTS] );

#endif /* ATTACK_LAST_ROUND_H */
