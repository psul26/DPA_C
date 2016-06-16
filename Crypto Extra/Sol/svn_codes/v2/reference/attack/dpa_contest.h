/*
 * Definitions of structures and constants used
 * in the DPA contest v2
 */

#ifndef DPA_CONTEST_H
#define DPA_CONTEST_H

#include <stdint.h>

#define TRACE_PLAINTEXT_SIZE_BYTES 16
#define TRACE_CIPHERTEXT_SIZE_BYTES 16
#define TRACE_KEY_SIZE_BYTES 16
#define TRACE_NUM_POINTS 3253


struct attack_trace
{
  uint8_t plaintext[TRACE_PLAINTEXT_SIZE_BYTES];
  uint8_t ciphertext[TRACE_CIPHERTEXT_SIZE_BYTES];
  int16_t samples[TRACE_NUM_POINTS];
};


struct attack_partial_result
{
  uint8_t subkey_num;
  uint8_t bytes[16][256];
};


#endif /* DPA_CONTEST_H */
