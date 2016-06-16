/*
 * @file   attack_last_round.cpp
 * @note   Side-Channel Attack on the AES-128 algorithm,
 *         exploiting the absence of MixColumns in the last round.
 * @author Sylvain GUILLEY, <sylvain.guilley@TELECOM-ParisTech.fr>
*/

#include "attack_last_round.h"
#include <string.h>
#include <algorithm>
#include <stdexcept>
#include <math.h>
using namespace std;

// ShiftRows
static unsigned int get_previous_location( unsigned int x9 )
{
	switch( x9 ) // and returns x10
	{
		case  0: return  0;
		case  1: return  5;
		case  2: return 10;
		case  3: return 15;
		case  4: return  4;
		case  5: return  9;
		case  6: return 14;
		case  7: return  3;
		case  8: return  8;
		case  9: return 13;
		case 10: return  2;
		case 11: return  7;
		case 12: return 12;
		case 13: return  1;
		case 14: return  6;
		case 15: return 11;
		default: throw range_error( "Unexpected byte location in the AES state" );
	}
}

static unsigned char const InvSubBytes[] = ///< Inverse of S
{
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
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
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

/// The Hamming weight, aka hw, of a byte, unsigned int, size_t, etc. (ancillary function for parity)
static unsigned int hw( unsigned int bitvector )
{
  unsigned int w = 0; // Initial weight is set to zero
  while( bitvector )
  {
    w += bitvector & 0x01u; // Bits accumulation
    bitvector >>= 1; // Unsignedness is necessary here to avoid sign extension
  }
  return w;
}

// TODO Which order for the index is correct? Nb_bytes_in_state x Nb_hypotheses x Nb_samples ?
// In C textbooks, they say "array[ROWS][COLS]".
// of size: 4*16*256*3253 = 53297152 bytes = 50.8 Mbytes.

// The sum of traces (for the average trace)
float t          [TRACE_NUM_POINTS];
// The sum of the traces weighted by the selection function (for the covariance trace)
float tw[16][256][TRACE_NUM_POINTS];
// The sum of weights (for the average trace)
float w [16][256];
// The sum of squared traces (for the variance trace)
float t2        [TRACE_NUM_POINTS];
// The sum of squared weights (for the variance of the weights)
float w2[16][256];

// Nota bene: we could speed up the attack knowing that the CPA peak is in [2345:2500], or simply restrict the search to sample 2370.

class score_t
{
public: // Set public for convenience
	uint8_t kguess;
	float cpa;
public:
	score_t( uint8_t kguess=0, float cpa=0 )
	: kguess( kguess )
	, cpa   ( cpa    )
	{}
	score_t& set_kguess( uint8_t _ ) { kguess=_; return *this; }
	score_t& set_cpa   ( float   _ ) { cpa   =_; return *this; }
	static bool cmp( score_t const& a, score_t const& b )
	{
		return a.cpa > b.cpa;
	}
} best_score[256]; // Global

void attack_last_round_init( )
{
	// Initialized to full zeros
	memset(  t, 0.f, sizeof(  t )); // sizeof( t  ) = 4*3253 = 13012
	memset( tw, 0.f, sizeof( tw )); // sizeof( tw ) = 4*256*3253 = 3331072
	memset(  w, 0.f, sizeof(  w ));
	memset( t2, 0.f, sizeof( t2 ));
	memset( w2, 0.f, sizeof( w2 ));
}

attack_partial_result attack_last_round(
	uint8_t ciphertext[TRACE_CIPHERTEXT_SIZE_BYTES], int16_t points[TRACE_NUM_POINTS] )
{
	attack_partial_result result;
	result.subkey_num = 10; // Last round
	// Now computing the purported order of the key guesses

	static int n=0;

	// 1. Updating the estimators:

	n++; // Number of processed waves == number of calls of this function

	for( int sample=0; sample<TRACE_NUM_POINTS; ++sample )
	{
		t [sample] +=  (float)points[sample];
		t2[sample] += ((float)points[sample]) * ((float)points[sample]);
	}

	for( int byte=0; byte<16; ++byte )
	{
		// Let x be the position 0 <= x < 16.
		// It is in column x/4 and in row x%4, because the state is like that:
		//
		// [  0  4  8 12 ]
		// [  1  5  9 13 ]
		// [  2  6 10 14 ]
		// [  3  7 11 15 ]
		//
		// Therefore, an hypothesis on the key 0 <= k < 16 will lead to guessing
		// a previous state (InvShiftRows) with the locations as:
		//
		// [  0  4  8 12 ]
		// [ 13  1  5  9 ]
		// [ 10 14  2  6 ]
		// [  7 11 15  3 ]

		// The differential waves:
		float cpa_wave[256][TRACE_NUM_POINTS];
		struct { int sample; float cpa; } locus[256], the_locus;
		for( int kguess=0; kguess<256; ++kguess )
		{
			locus[kguess].sample = 0;
			locus[kguess].cpa    = -1.f; // Minimum possible value
		}

		for( int kguess=0; kguess<256; ++kguess )
		{
			uint8_t const
				final_state   = ciphertext[ get_previous_location( byte ) ],
				initial_state = InvSubBytes[ ciphertext[ byte ] ^ kguess ];
			// Also: final_state = c ; initial state = InvSubBytes o InvShiftRows o InvAddRoundKey( c ),
			// therefore a value of c XOR InvSubBytes o InvShiftRows o InvAddRoundKey( c ), which is equal to:
			// ShiftRows( c ) XOR InvSubBytes o InvAddRoundKey( c ), because of the linearity of ShiftRows w.r.t. XOR and
			// its commutatitivity with InvSubBytes (working on bytes vs with bytes).
			int const sensitive_transition = hw( initial_state ^ final_state ) // The weight used in the linear prediction
				- 4; // Already centered (although not strictly necessary at this point [the covariance computation does so], it enables an optimization).

			if( sensitive_transition != 0 ) // Because in this case, there is no accumulation to be done
			{
				w [byte][kguess] +=  (float)sensitive_transition;
				w2[byte][kguess] += ((float)sensitive_transition)*((float)sensitive_transition);

				for( int sample=0; sample<TRACE_NUM_POINTS; ++sample )
				{
					tw[byte][kguess][sample] += (float)points[sample] * (float)sensitive_transition;
				}
			}

			// 2. Computing the differential waves
			float const h_var = ( w2[byte][kguess] - w[byte][kguess] * w[byte][kguess] / n ) / n;
			for( int sample=0; sample<TRACE_NUM_POINTS; ++sample )
			{
				// Covariance
				cpa_wave[ kguess ][ sample ] = ( tw[byte][kguess][sample] - w [byte][kguess] * t[sample] / n) / n;
				// Normalization by the trace
				float const t_var = ( t2[sample] - t[sample] * t[sample] / n ) / n;
				if( t_var == 0.f )
				{
					cpa_wave[ kguess ][ sample ] = 0.f; // To avoid a division by zero ... we reset
				}
				else
				{
					cpa_wave[ kguess ][ sample ] /= sqrt( t_var );
				}
				// Normalization by the weight
				if( h_var == 0 )
				{
					cpa_wave[ kguess ][ sample ] = 0.f; // To avoid a division by zero ... we reset
				}
				else
				{
					cpa_wave[ kguess ][ sample ] /= sqrt( h_var );
				}
			}

			// Finding the maximum for this guess
			for( int sample=0; sample<TRACE_NUM_POINTS; ++sample )
			{
				// We do not consider CPA peaks in absolute value ;
				// rather, they are in "relative" = "signed" values, i.e. compared plain.
				if( cpa_wave[kguess][sample] > locus[kguess].cpa )
				{
					locus[kguess].sample = sample; // X
					locus[kguess].cpa    = cpa_wave[kguess][sample]; // Y
				}
			}
		}

		// Looking for the time sample where the maximum amongst all key guesses is
		the_locus.sample = 0;
		the_locus.cpa    = -1.f; // The minimum possible value
		for( int kguess=0; kguess<256; ++kguess )
		{
			if( locus[kguess].cpa > the_locus.cpa )
			{
				the_locus.sample = locus[kguess].sample;
				the_locus.cpa    = locus[kguess].cpa;
			}
		}

		//fprintf( log, "Locus at sample: %d, with value: %f\n", the_locus.sample, the_locus.cpa ); fflush( log );

		// Sorting
		// TODO optimize
		for( int kguess=0; kguess<256; ++kguess )
		{
			// All CPA points (for each key guess) are considered at the same date:       vvvvvvvvvvvvvvvv
			best_score[kguess].set_kguess( (uint8_t)kguess ).set_cpa( cpa_wave[ kguess ][ the_locus.sample ] );
		}

		sort( best_score, best_score+sizeof( best_score )/sizeof( best_score[0] ), score_t::cmp );

		for( int kguess=0; kguess<256; ++kguess )
		{
			result.bytes[byte][kguess] = best_score[kguess].kguess; // From the best CPA (highest == most probable key) to the poorest CPA
		}

		//+ // HEAVY DEBUG: saving the differential traces after a given number of iterations. At least 10000 for the regular CPA!
		//+ if( n==19000 )
		//+ {
		//+ 	FILE* f = fopen( "/tmp/diff.csv", "w" );
		//+ 	for( int sample=0; sample<TRACE_NUM_POINTS; ++sample )
		//+ 	{
		//+ 		for( int kguess=0; kguess<256; ++kguess )
		//+ 		{
		//+ 			fprintf( f, "%f\t", cpa_wave[ kguess ][ sample ] );
		//+ 		}
		//+ 		fprintf( f, "\n" );
		//+ 	}
		//+ 	fclose( f );
		//+ }

		//fprintf( log, "Result matrix filled OK\n" ); fflush( log );
	}

	return result;
}
