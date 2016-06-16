/*
 * Definition of data structures used in binary results
 */

#ifndef DPA_CONTEST_RESULTS_H
#define DPA_CONTEST_RESULTS_H

#include <stdint.h>
#include "dpa_contest.h"

#define RESULTS_BINARY_FORMAT_VERSION 1

struct result_file_header
{
  uint32_t format_version;
  uint32_t subkey_num;
  uint32_t num_iterations;
  uint32_t num_dbkey;
  uint8_t correct_key[TRACE_KEY_SIZE_BYTES];
  uint8_t correct_subkey[TRACE_KEY_SIZE_BYTES];
};


struct result_file_trace_header
{
  uint8_t plaintext[TRACE_PLAINTEXT_SIZE_BYTES];
  uint8_t ciphertext[TRACE_CIPHERTEXT_SIZE_BYTES];
  double time_consumed;
};


#endif /* DPA_CONTEST_RESULTS_H */
