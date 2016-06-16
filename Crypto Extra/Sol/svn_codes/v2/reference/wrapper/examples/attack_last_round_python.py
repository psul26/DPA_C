#!/usr/bin/env python

import sys
import math

import dpa_contest


class Score:
	def __init__(self):
		self.kguess = 0
		self.cpa = 0.
	def __cmp__(self, other):
		return cmp(other.cpa, self.cpa)

class Locus:
	def __init__(self):
		self.sample = 0
		self.cpa = 0.



# Inverse of S
InvSubBytes = [0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	       0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	       0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	       0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	       0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	       0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	       0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	       0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	       0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	       0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	       0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	       0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	       0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	       0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	       0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	       0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d]

# The sum of traces (for the average trace)
#float t          [TRACE_NUM_POINTS];
t = [0.] * dpa_contest.AttackTrace.trace_num_points
# The sum of the traces weighted by the selection function (for the covariance trace)
#float tw[16][256][TRACE_NUM_POINTS];
tw = [[[0.] * dpa_contest.AttackTrace.trace_num_points for i in range(256)] for j in range(16)]
# The sum of weights (for the average trace)
#float w [16][256];
w = [[0.] * 256 for i in range(16)]
# The sum of squared traces (for the variance trace)
#float t2        [TRACE_NUM_POINTS];
t2 = [0.] * dpa_contest.AttackTrace.trace_num_points
# The sum of squared weights (for the variance of the weights)
#float w2[16][256];
w2 = [[0.] * 256 for i in range(16)]

best_score = [Score() for i in range(256)]


# ShiftRows
def get_previous_location(x9):
	if x9 == 0:
		return 0
	if x9 == 1:
		return 5
	if x9 == 2:
		return 10
	if x9 == 3:
		return 15
	if x9 == 4:
		return 4
	if x9 == 5:
		return 9
	if x9 == 6:
		return 14
	if x9 == 7:
		return 3
	if x9 == 8:
		return 8
	if x9 == 9:
		return 13
	if x9 == 10:
		return 2
	if x9 == 11:
		return 7
	if x9 == 12:
		return 12
	if x9 == 13:
		return 1
	if x9 == 14:
		return 6
	if x9 == 15:
		return 11
	assert(false)


# The Hamming weight, aka hw, of a byte, unsigned int, size_t,
# etc. (ancillary function for parity)
def hw(bitvector):
	w = 0 # Initial weight is set to zero
	while bitvector != 0:
		w += bitvector & 0x01 # Bits accumulation
		bitvector >>= 1
	return w


# Nota bene: we could speed up the attack knowing that the CPA peak is
# in [2345:2500], or simply restrict the search to sample 2370.



def attack_last_round(trace, current_trace_num):
	global InvSubBytes, best_score, t, tw, w, t2, w2

	result = dpa_contest.AttackPartialResult(10) # Last round

	# Now computing the purported order of the key guesses

	# 1. Updating the estimators:

	for sample in range(dpa_contest.AttackTrace.trace_num_points):
		t [sample] += trace.samples[sample]
		t2[sample] += trace.samples[sample] * trace.samples[sample]


	for byte in range(16):
		# Let x be the position 0 <= x < 16.
		# It is in column x/4 and in row x%4, because the state is like that:
		#
		# [  0  4  8 12 ]
		# [  1  5  9 13 ]
		# [  2  6 10 14 ]
		# [  3  7 11 15 ]
		#
		# Therefore, an hypothesis on the key 0 <= k < 16 will lead to guessing
		# a previous state (InvShiftRows) with the locations as:
		#
		# [  0  4  8 12 ]
		# [ 13  1  5  9 ]
		# [ 10 14  2  6 ]
		# [  7 11 15  3 ]

		# The differential waves:
		cpa_wave = [[0.] * dpa_contest.AttackTrace.trace_num_points for i in range(256)]

		the_locus = Locus()
		locus = [Locus() for i in range(256)]


		for kguess in range(256):
			locus[kguess].sample = 0
			locus[kguess].cpa    = -1. # Minimum possible value


		for kguess in range(256):

			final_state = trace.ciphertext[get_previous_location(byte)]
			initial_state = InvSubBytes[trace.ciphertext[byte] ^ kguess]

			# Also: final_state = c ; initial state = InvSubBytes o InvShiftRows o InvAddRoundKey( c ),
			# therefore a value of c XOR InvSubBytes o InvShiftRows o InvAddRoundKey( c ), which is equal to:
			# ShiftRows( c ) XOR InvSubBytes o InvAddRoundKey( c ), because of the linearity of ShiftRows w.r.t. XOR and
			# its commutatitivity with InvSubBytes (working on bytes vs with bytes).

			sensitive_transition = hw(initial_state ^ final_state) # The weight used in the linear prediction

			w [byte][kguess] += sensitive_transition
			w2[byte][kguess] += sensitive_transition * sensitive_transition

			for sample in range(dpa_contest.AttackTrace.trace_num_points):
				tw[byte][kguess][sample] += trace.samples[sample] * sensitive_transition


			# 2. Computing the differential waves
			h_var = (w2[byte][kguess] - w[byte][kguess] * w[byte][kguess] / current_trace_num) / current_trace_num
			for sample in range(dpa_contest.AttackTrace.trace_num_points):

				# Covariance
				cpa_wave[kguess][sample] = (tw[byte][kguess][sample] - w[byte][kguess] * t[sample] / current_trace_num) / current_trace_num

				# Normalization by the trace
				t_var = (t2[sample] - t[sample] * t[sample] / current_trace_num) / current_trace_num
				if t_var == 0.:
					cpa_wave[kguess][sample] = 0. # To avoid a division by zero ... we reset
				else:
					cpa_wave[kguess][sample] /= math.sqrt(t_var)

				# Normalization by the weight
				if h_var == 0:
					cpa_wave[kguess][sample] = 0. # To avoid a division by zero ... we reset
				else:
					cpa_wave[kguess][sample] /= math.sqrt(h_var)


			# Finding the maximum for this guess
			for sample in range(dpa_contest.AttackTrace.trace_num_points):
				# We do not consider CPA peaks in absolute value ;
				# rather, they are in "relative" = "signed" values, i.e. compared plain.
				if cpa_wave[kguess][sample] > locus[kguess].cpa:
					locus[kguess].sample = sample # X
					locus[kguess].cpa    = cpa_wave[kguess][sample] # Y


		# Looking for the time sample where the maximum amongst all key guesses is
		the_locus.sample = 0
		the_locus.cpa    = -1. # The minimum possible value
		for kguess in range(256):
			if  locus[kguess].cpa > the_locus.cpa:
				the_locus.sample = locus[kguess].sample
				the_locus.cpa    = locus[kguess].cpa

		# Sorting
		# TODO optimize
		for kguess in range(256):
			# All CPA points (for each key guess) are considered at the same date
			best_score[kguess].kguess = kguess
			best_score[kguess].cpa = cpa_wave[kguess][the_locus.sample]


		best_score.sort()

		for kguess in range(256):
			result.bytes[byte][kguess] = best_score[kguess].kguess
			# From the best CPA (highest == most probable key) to the poorest CPA

	return result



def main():
	try:
		attack_trace = dpa_contest.AttackTrace()

		#attack_last_round_init()

		num_traces = dpa_contest.read_num_traces(sys.stdin)
		dpa_contest.send_start(sys.stdout)

		for current_trace_num in range(1, num_traces+1):
			attack_trace.read(sys.stdin)

			attack_partial_result = attack_last_round(attack_trace, current_trace_num)

			attack_partial_result.write(sys.stdout)
	except:
		f = open('dbg', 'w')
		f.write("%s\n%s\n%s\n" % (str(sys.exc_info()[0]), str(sys.exc_info()[1]), str(sys.exc_info()[2])))
		f.close()



if __name__ == "__main__":
	main()
