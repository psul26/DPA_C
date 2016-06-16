
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/****************************************************************/

#define TRUE  1
#define FALSE 0

#define OFFSET 1

// Put here the name of the base directory containing the curves.
// This directory must contain 9 subdirectories respectively named "0", "1", ..., "8".
// Subdirectory "0" must contain the first 10,000 curves (when listed in *temporal* order),
// Subdirectory "1" must contain the next 10,000 curves,
// ...
// Subdirectory "8" must contain the last 1089 curves.
#define CURVE_DIRECTORY ".."

// This is the initial seed for the batch of 100 averaged runs.
// The individual seed of the run number i is equal to this initial seed + i.
// The particular value 1252305000 has been chosen as related to a particular event.
// Namely it is the time() value (in seconds) of the beginning
// of the CHES 2009 conference (September 2009, 07 at 8:30:00).
// This way of defining the initial seed gives some confidence that
// the seed has not been chosen for obtaining particularly good results.
#define SEED_CHES 1252305000

// Total number of curves
#define NB_CURVE  81089

// Max number of curves that a run is supposed to use
#define NB_CURVE_USED_MAX  400

// Each run start trying to fond the key with NB_CURVE_BEGIN curves.
// Then it tries repeatedly with one more curve each time.
// As soon as a sequence of 100 consecutive succesfull attacks is obtained,
// that run terminates and its score is set to the number of curves needed
// for the first attack of this sequence.
// Note that the official score for the DPA contest is the number of curves
// needed for the last attack of the sequence (99 curves more).
#define NB_CURVE_BEGIN  30

// The representative score of the method is set to the average score
// of NB_RUN runs with randomly chosen curves
#define NB_RUN  100

// T_00 is the point of interest at the end of the first round.
// It happens to be located very close to the instant of max consumption.
#define T_00  5743

// Those parameters are good coefficients for a bivariate linear model at T_00.
// They are not used.
//#define A_00  0.00095159
//#define B_00  0.000367882
//#define C_00  0.0660436

// T_16 is the point of interest at the end of the last round.
// It happens to be located very close to the instant of max consumption.
#define T_16  15745

// Those parameters are good coefficients for a bivariate linear model at T_16.
// They are not used.
//#define A_16  0.00133322
//#define B_16  0.000431363
//#define C_16  0.0471272

// The 56-bit key will be searched by oriented walk starting from an initial random value.
// For each attack nb_try starting keys will be considered at the same time.
// nb_try need to be greater when the number of curve is small than when it is larger.
// The next four parameters define an exponential decreasing function
// of nb_try with respect to the number of curves.

#define NB_TRY_T1  35.0
#define NB_TRY_X1 100.0

#define NB_TRY_T2  60.0
#define NB_TRY_X2  50.0

// The next two parameters define some enforced bounds on nb_try.
#define NB_TRY_MIN   15
#define NB_TRY_MAX  200

// The number of consecutive successfull attacks in each run.
#define STABILITY   100

#define ascii_to_bin(x) ( ((x)>='0' && (x)<='9') ? (x)-'0' : 10+tolower(x)-'a' )
#define bin_to_ascii(x) ( ((x)<=9) ? (x)+'0' : (x)+'A'-10 )

/****************************************************************/

typedef unsigned char      BYTE;
typedef unsigned long       WORD32;
typedef signed   int       INT32;

/****************************************************************/

static BYTE IP[64] =
{
  58, 50, 42, 34, 26, 18, 10,  2,
  60, 52, 44, 36, 28, 20, 12,  4,
  62, 54, 46, 38, 30, 22, 14,  6,
  64, 56, 48, 40, 32, 24, 16,  8,
  57, 49, 41, 33, 25, 17,  9,  1,
  59, 51, 43, 35, 27, 19, 11,  3,
  61, 53, 45, 37, 29, 21, 13,  5,
  63, 55, 47, 39, 31, 23, 15,  7
};

static BYTE E[48]=
{
  32,  1,  2,  3,  4,  5,
   4,  5,  6,  7,  8,  9,
   8,  9, 10, 11, 12, 13,
  12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21,
  20, 21, 22, 23, 24, 25,
  24, 25, 26, 27, 28, 29,
  28, 29, 30, 31, 32,  1
};

static BYTE inv_P[32]=
{
   9, 17, 23, 31,
  13, 28,  2, 18,
  24, 16, 30,  6,
  26, 20, 10,  1,
   8, 14, 25,  3,
   4, 29, 11, 19,
  32, 12, 22,  7,
   5, 27, 15, 21
};

static BYTE S_Box[8][64] =
{
  {
    14,  0,  4, 15, 13,  7,  1,  4,  2, 14, 15,  2, 11, 13,  8,  1,
     3, 10, 10,  6,  6, 12, 12, 11,  5,  9,  9,  5,  0,  3,  7,  8,
     4, 15,  1, 12, 14,  8,  8,  2, 13,  4,  6,  9,  2,  1, 11,  7,
    15,  5, 12, 11,  9,  3,  7, 14,  3, 10, 10,  0,  5,  6,  0, 13
  },
  {
    15,  3,  1, 13,  8,  4, 14,  7,  6, 15, 11,  2,  3,  8,  4, 14,
     9, 12,  7,  0,  2,  1, 13, 10, 12,  6,  0,  9,  5, 11, 10,  5,
     0, 13, 14,  8,  7, 10, 11,  1, 10,  3,  4, 15, 13,  4,  1,  2,
     5, 11,  8,  6, 12,  7,  6, 12,  9,  0,  3,  5,  2, 14, 15,  9
  },
  {
    10, 13,  0,  7,  9,  0, 14,  9,  6,  3,  3,  4, 15,  6,  5, 10,
     1,  2, 13,  8, 12,  5,  7, 14, 11, 12,  4, 11,  2, 15,  8,  1,
    13,  1,  6, 10,  4, 13,  9,  0,  8,  6, 15,  9,  3,  8,  0,  7,
    11,  4,  1, 15,  2, 14, 12,  3,  5, 11, 10,  5, 14,  2,  7, 12
  },
  {
     7, 13, 13,  8, 14, 11,  3,  5,  0,  6,  6, 15,  9,  0, 10,  3,
     1,  4,  2,  7,  8,  2,  5, 12, 11,  1, 12, 10,  4, 14, 15,  9,
    10,  3,  6, 15,  9,  0,  0,  6, 12, 10, 11,  1,  7, 13, 13,  8,
    15,  9,  1,  4,  3,  5, 14, 11,  5, 12,  2,  7,  8,  2,  4, 14
  },
  {
     2, 14, 12, 11,  4,  2,  1, 12,  7,  4, 10,  7, 11, 13,  6,  1,
     8,  5,  5,  0,  3, 15, 15, 10, 13,  3,  0,  9, 14,  8,  9,  6,
     4, 11,  2,  8,  1, 12, 11,  7, 10,  1, 13, 14,  7,  2,  8, 13,
    15,  6,  9, 15, 12,  0,  5,  9,  6, 10,  3,  4,  0,  5, 14,  3
  },
  {
    12, 10,  1, 15, 10,  4, 15,  2,  9,  7,  2, 12,  6,  9,  8,  5,
     0,  6, 13,  1,  3, 13,  4, 14, 14,  0,  7, 11,  5,  3, 11,  8,
     9,  4, 14,  3, 15,  2,  5, 12,  2,  9,  8,  5, 12, 15,  3, 10,
     7, 11,  0, 14,  4,  1, 10,  7,  1,  6, 13,  0, 11,  8,  6, 13
   },
   {
     4, 13, 11,  0,  2, 11, 14,  7, 15,  4,  0,  9,  8,  1, 13, 10,
     3, 14, 12,  3,  9,  5,  7, 12,  5,  2, 10, 15,  6,  8,  1,  6,
     1,  6,  4, 11, 11, 13, 13,  8, 12,  1,  3,  4,  7, 10, 14,  7,
    10,  9, 15,  5,  6,  0,  8, 15,  0, 14,  5,  2,  9,  3,  2, 12
  },
  {
    13,  1,  2, 15,  8, 13,  4,  8,  6, 10, 15,  3, 11,  7,  1,  4,
    10, 12, 9,   5,  3,  6, 14, 11,  5,  0,  0, 14, 12,  9,  7,  2,
     7,  2, 11,  1,  4, 14,  1,  7,  9,  4, 12, 10, 14,  8,  2, 13,
     0, 15,  6, 12, 10,  9, 13,  0, 15,  3,  3,  5,  5,  6,  8, 11
  }
};

static BYTE K00_bit[48] =
{
  10, 51, 34, 60, 49, 17,
  33, 57,  2,  9, 19, 42,
   3, 35, 26, 25, 44, 58,
  59,  1, 36, 27, 18, 41,
  22, 28, 39, 54, 37,  4,
  47, 30,  5, 53, 23, 29,
  61, 21, 38, 63, 15, 20,
  45, 14, 13, 62, 55, 31
};

static BYTE K16_bit[48] =
{
  18, 59, 42,  3, 57, 25,
  41, 36, 10, 17, 27, 50,
  11, 43, 34, 33, 52,  1,
   2,  9, 44, 35, 26, 49,
  30,  5, 47, 62, 45, 12,
  55, 38, 13, 61, 31, 37,
   6, 29, 46,  4, 23, 28,
  53, 22, 21,  7, 63, 39
};

static BYTE K_left_bit[28]  = {1, 2, 3, 9, 10, 11, 17, 18, 19, 25, 26, 27, 33, 34, 35, 36, 41, 42, 43, 44, 49, 50, 51, 52, 57, 58, 59, 60};

static BYTE K_right_bit[28] = {4, 5, 6, 7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31, 37, 38, 39, 45, 46, 47, 53, 54, 55, 61, 62, 63};

/****************************************************************/

static  BYTE  hw_00[NB_CURVE];
static  BYTE  hw_01[NB_CURVE][8][64];
static  BYTE  hw_15[NB_CURVE][8][64];
static  BYTE  hw_16[NB_CURVE];

static  float data_00[NB_CURVE_USED_MAX];
static  float data_16[NB_CURVE_USED_MAX];

static  WORD32  id_list[NB_CURVE];

static  BYTE  b6_key00_best[NB_TRY_MAX][8];
static  BYTE  b6_key16_best[NB_TRY_MAX][8];

static  float score_best[NB_TRY_MAX];

static  unsigned char sz_plaintext[NB_CURVE][20];
static  unsigned char sz_ciphertext[NB_CURVE][20];
static  unsigned char sz_names[NB_CURVE][100];

/****************************************************************/

// Given a candidate (b6_key00) for the round key K1 of the first round,
// this function computes the mean square distance (equivalent to likelyhood)
// between the nb_curve measurements and the corresponding predicted consumptions
// under that key candidate. The predicted consumptions are computed
// according to the following general model whose coefficients are identified
// "online" based on the measurements:
  
//  Bivariate model:  consumption = a + b * hw01 + c * hw00 + noise
//  where hw00 and hw01 are the Hamming distance between left and right halves at the end of rounds 0 and 1 respectively

//  Y = X.A + noise
//  X = [ 1 hw01 hw00 ]_i  (i = 0,...,n-1)
//  A = [ a b c ]

//  Estimation of A :  (X'.X)^{-1} . X' . Y

//              s00 s01 s02
//  S = X'.X =  s10 s11 s12
//              s20 s21 s22

//  s00 = n
//  s11 = \sigma_i (hw01_i)^2
//  s22 = \sigma_i (hw00_i)^2
//  s01 = s10 = \sigma_i hw01_i
//  s02 = s20 = \sigma_i hw00_i
//  s12 = s21 = \sigma_i hw01_i * hw00_i

//  U = S^{-1} = (X'.X)^{-1}

float  likelihood00(WORD32 nb_curve, BYTE* b6_key00)
{
  double delta;
  double S[3][3];
  double U[3][3];

  float a, b, c, d, e, f;
  float error;
  float prediction;
  float sum = 0.0;

  WORD32 hw00[NB_CURVE_USED_MAX];
  WORD32 hw01[NB_CURVE_USED_MAX];
  WORD32 i;
  WORD32 id;
  WORD32 n0_id;

  
  S[0][0] = nb_curve;
  S[0][1] = 0.0;
  S[0][2] = 0.0;
  S[1][0] = 0.0;
  S[1][1] = 0.0;
  S[1][2] = 0.0;
  S[2][0] = 0.0;
  S[2][1] = 0.0;
  S[2][2] = 0.0;
  
  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    id = id_list[n0_id];

    hw01[n0_id] = 0;
    for (i = 0; i < 8; i++)
    {
      hw01[n0_id] += hw_01[id][i][b6_key00[i]];
    }
    hw00[n0_id] = hw_00[id];
    
    S[1][1] += (hw01[n0_id] * hw01[n0_id]);
    S[2][2] += (hw00[n0_id] * hw00[n0_id]);
    S[0][1] = S[1][0] += hw01[n0_id];
    S[0][2] = S[2][0] += hw00[n0_id];
    S[1][2] = S[2][1] += hw01[n0_id] * hw00[n0_id];
  }
  
  // Computation of S^{-1}
  
  delta = S[0][0] * ( S[1][1] * S[2][2] - S[1][2] * S[2][1] )
        - S[0][1] * ( S[1][0] * S[2][2] - S[1][2] * S[2][0] )
        + S[0][2] * ( S[1][0] * S[2][1] - S[1][1] * S[2][0] );
  
  U[0][0] =  ( S[1][1] * S[2][2] - S[1][2] * S[2][1] );
  U[0][1] = -( S[0][1] * S[2][2] - S[0][2] * S[2][1] );
  U[0][2] =  ( S[0][1] * S[1][2] - S[0][2] * S[1][1] );
  U[1][0] = -( S[1][0] * S[2][2] - S[1][2] * S[2][0] );
  U[1][1] =  ( S[0][0] * S[2][2] - S[0][2] * S[2][0] );
  U[1][2] = -( S[0][0] * S[1][2] - S[0][2] * S[1][0] );
  U[2][0] =  ( S[1][0] * S[2][1] - S[1][1] * S[2][0] );
  U[2][1] = -( S[0][0] * S[2][1] - S[0][1] * S[2][0] );
  U[2][2] =  ( S[0][0] * S[1][1] - S[0][1] * S[1][0] );

  d = 0.0;
  e = 0.0;
  f = 0.0;
  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    id = id_list[n0_id];
    
    d += data_00[n0_id];
    e += hw01[n0_id] * data_00[n0_id];
    f += hw00[n0_id] * data_00[n0_id];
  }
  
  a = (U[0][0] * d + U[0][1] * e + U[0][2] * f) / delta;
  b = (U[1][0] * d + U[1][1] * e + U[1][2] * f) / delta;
  c = (U[2][0] * d + U[2][1] * e + U[2][2] * f) / delta;

  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    prediction = a + b * hw01[n0_id] + c * hw00[n0_id];

    error = data_00[n0_id] - prediction;
    sum += error * error;
  }
	
  return(sum);
}

/****************************************************************/

// Given a candidate (b6_key16) for the round key K16 of the last round,
// this function computes the mean square distance (equivalent to likelyhood)
// between the nb_curve measurements and the corresponding predicted consumptions
// under that key candidate. The predicted consumptions are computed
// according to the following general model whose coefficients are identified
// "online" based on the measurements:
  
//  Bivariate model:  consumption = a + b * hw15 + c * hw16 + noise
//  where hw15 and hw16 are the Hamming distance between left and right halves at the end of rounds 15 and 16 respectively

//  Y = X.A + noise
//  X = [ 1 hw15 hw16 ]_i  (i = 0,...,n-1)
//  A = [ a b c ]

//  Estimation of A :  (X'.X)^{-1} . X' . Y

//              s00 s01 s02
//  S = X'.X =  s10 s11 s12
//              s20 s21 s22

//  s00 = n
//  s11 = \sigma_i (hw15_i)^2
//  s22 = \sigma_i (hw16_i)^2
//  s01 = s10 = \sigma_i hw15_i
//  s02 = s20 = \sigma_i hw16_i
//  s12 = s21 = \sigma_i hw15_i * hw16_i

//  U = S^{-1} = (X'.X)^{-1}

float  likelihood16(WORD32 nb_curve, BYTE* b6_key16)
{
  double delta;
  double S[3][3];
  double U[3][3];

  float a, b, c, d, e, f;
  float error;
  float prediction;
  float sum = 0.0;

  WORD32 hw15[NB_CURVE_USED_MAX];
  WORD32 hw16[NB_CURVE_USED_MAX];
  WORD32 i;
  WORD32 id;
  WORD32 n0_id;

  
  S[0][0] = nb_curve;
  S[0][1] = 0.0;
  S[0][2] = 0.0;
  S[1][0] = 0.0;
  S[1][1] = 0.0;
  S[1][2] = 0.0;
  S[2][0] = 0.0;
  S[2][1] = 0.0;
  S[2][2] = 0.0;
  
  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    id = id_list[n0_id];

    hw15[n0_id] = 0;
    for (i = 0; i < 8; i++)
    {
      hw15[n0_id] += hw_15[id][i][b6_key16[i]];
    }    
    hw16[n0_id] = hw_16[id];
    
    S[1][1] += (hw15[n0_id] * hw15[n0_id]);
    S[2][2] += (hw16[n0_id] * hw16[n0_id]);
    S[0][1] = S[1][0] += hw15[n0_id];
    S[0][2] = S[2][0] += hw16[n0_id];
    S[1][2] = S[2][1] += hw15[n0_id] * hw16[n0_id];
  }
  
  // Computation of S^{-1}
  
  delta = S[0][0] * ( S[1][1] * S[2][2] - S[1][2] * S[2][1] )
        - S[0][1] * ( S[1][0] * S[2][2] - S[1][2] * S[2][0] )
        + S[0][2] * ( S[1][0] * S[2][1] - S[1][1] * S[2][0] );
  
  U[0][0] =  ( S[1][1] * S[2][2] - S[1][2] * S[2][1] );
  U[0][1] = -( S[0][1] * S[2][2] - S[0][2] * S[2][1] );
  U[0][2] =  ( S[0][1] * S[1][2] - S[0][2] * S[1][1] );
  U[1][0] = -( S[1][0] * S[2][2] - S[1][2] * S[2][0] );
  U[1][1] =  ( S[0][0] * S[2][2] - S[0][2] * S[2][0] );
  U[1][2] = -( S[0][0] * S[1][2] - S[0][2] * S[1][0] );
  U[2][0] =  ( S[1][0] * S[2][1] - S[1][1] * S[2][0] );
  U[2][1] = -( S[0][0] * S[2][1] - S[0][1] * S[2][0] );
  U[2][2] =  ( S[0][0] * S[1][1] - S[0][1] * S[1][0] );

  d = 0.0;
  e = 0.0;
  f = 0.0;
  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    id = id_list[n0_id];
    
    d += data_16[n0_id];
    e += hw15[n0_id] * data_16[n0_id];
    f += hw16[n0_id] * data_16[n0_id];
  }
  
  a = (U[0][0] * d + U[0][1] * e + U[0][2] * f) / delta;
  b = (U[1][0] * d + U[1][1] * e + U[1][2] * f) / delta;
  c = (U[2][0] * d + U[2][1] * e + U[2][2] * f) / delta;

  for (n0_id = 0; n0_id < nb_curve; n0_id++)
  {
    prediction = a + b * hw15[n0_id] + c * hw16[n0_id];

    error = data_16[n0_id] - prediction;
    sum += error * error;
  }
	
  return(sum);
}

/****************************************************************/

int main(int argc, char** argv)
{
  BYTE  b6_correct_key00[8] = {56, 11, 59, 38,  0, 13, 25, 55};
  BYTE  b6_correct_key16[8] = {60, 11, 56, 46, 22, 50, 16, 44};

  BYTE  b06, b07, b11, b12, b14, b15, b19, b20, b43, b46, b50, b51, b52, b54, b58, b60;
  BYTE  b_finish = FALSE;
  BYTE  b_found = FALSE;
  BYTE  b_stable[NB_TRY_MAX];
  BYTE  b_used[NB_CURVE];
  BYTE  b1_ciphertext[64];
  BYTE  b1_plaintext[64];
  BYTE  b1_L00[32];
  BYTE  b1_R00[32];
  BYTE  b1_L16[32];
  BYTE  b1_R16[32];
  BYTE  b1_m00[48];
  BYTE  b1_m16[48];
  BYTE  b6_x;
  BYTE  b4_y;
  BYTE  b6_key00_base[NB_TRY_MAX][8];
  BYTE  b6_key00_guess[64];
  BYTE  b6_key16_base[NB_TRY_MAX][8];
  BYTE  b6_key16_guess[64];
  BYTE  b6_m00[8];
  BYTE  b6_m16[8];


  FILE* pfd_cipher = NULL;
  FILE* pfd_curve = NULL;
  FILE* pfd_names = NULL;
  FILE* pfd_plain = NULL;

  float score_base[NB_TRY_MAX];
  float score_correct_key;
  float score_guess;
  float likelihood_00;
  float likelihood_16;
  float nb_try_a;
  float nb_try_tau;

  char sz_file_name[200];
  char line[200];

  INT32   i, j, k;
  INT32   K00_pos[64];
  INT32   K16_pos[64];
  INT32   pos_in_key00[48];
  INT32   pos_in_key16[48];

  time_t  time_begin;
  time_t  time_end;

  WORD32  bit;
  WORD32  delta1;
  WORD32  id;
  WORD32  hexa_digit;
  WORD32  n0_best_key;
  WORD32  n0_bit;
  WORD32  n0_char;
  WORD32  n0_id;
  WORD32  n0_run;
  WORD32  nb_curve;
  WORD32  nb_try;
  WORD32  pos;
  WORD32  sbox1;
  WORD32  seed;
  WORD32  stability = 0;


  /*
    Seed management
    ---------------
  */
  
  // The inital seed is set to either:
  //   - the default SEED_CHES value if no second argument appear on the command line
  //   - a non null arbitrary value provided as a second argument (allows to run again some specific attack)
  //   - a time dependant value if the second argument is 0

  seed = SEED_CHES;
  if (argc == 2) seed = atol(argv[1]);
  if (seed == 0) seed = time(NULL);
  srand(seed);

  /*
    Read in the file containing all the plaintexts
    ----------------------------------------------
  */

  // A file named "input.txt" containing all the plaintext values
  // must be located in the parent directory.

  sprintf(sz_file_name,"../input.txt");
  if ((pfd_plain = fopen(sz_file_name, "r")) == NULL)
  {
    printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
    exit(-1);
  }

  id = 0;
  while ( (fgets(line, 200, pfd_plain) != NULL) && (id < NB_CURVE) )
  {
    sscanf(line, "%s", sz_plaintext[id]);

    /*
      Convert from string to binary
    */
    for (n0_char = 0; n0_char < 16; n0_char++)
    {
      hexa_digit = ascii_to_bin(sz_plaintext[id][n0_char]);
      b1_plaintext[4 * n0_char + 3] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_plaintext[4 * n0_char + 2] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_plaintext[4 * n0_char + 1] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_plaintext[4 * n0_char + 0] = (hexa_digit & 1);
    }

    /*
      Compute R00, L00 and the expansive permutation output of first round
    */
    for (i = 0; i < 32; i++)
    {
      b1_L00[i] = b1_plaintext[IP[i +  0] - OFFSET];
      b1_R00[i] = b1_plaintext[IP[i + 32] - OFFSET];
    }
    for (i = 0; i < 48; i++)
    {
      b1_m00[i] = b1_R00[E[i] - OFFSET];

      if (i % 6 == 0) b6_m00[i / 6] = 0;

      b6_m00[i / 6] += (b1_m00[i] << (5 - (i % 6)));
    }

    // For the curve of index id, for sbox number i (numbered from 0 to 7),
    // and assuming that k is the subkey value entering in this sbox,
    // hw_01[id][i][k] designates the contribution (between 0 and 4) of this
    // sbox to the Hamming distance between L01 and R01 halves.

    for (i = 0; i < 8; i++)
    {
      for (k = 0; k < 64; k++)
      {
        b6_x = b6_m00[i] ^ k;
        b4_y = S_Box[i][b6_x];

        hw_01[id][i][k] = 0;
        for (j = 0; j < 4; j++)
        {
          hw_01[id][i][k] += ((b4_y >> (3 - j)) & 1) ^ b1_R00[inv_P[4 * i + j] - OFFSET] ^ b1_L00[inv_P[4 * i + j] - OFFSET];
        }
      }
    }

    // For the curve of index id, hw_00[id] designates Hamming distance between L00 and R00 halves.

    hw_00[id] = 0;
    for (i = 0; i < 32; i++)
    {
      hw_00[id] += b1_R00[i] ^ b1_L00[i];
    }

    id++;
  }
  fclose(pfd_plain);


  /*
    Read in the file containing all the ciphertexts
    -----------------------------------------------
  */

  // A file named "output.txt" containing all the ciphertext values
  // must be located in the parent directory.

  sprintf(sz_file_name,"../output.txt");
  if ((pfd_cipher = fopen(sz_file_name, "r")) == NULL)
  {
    printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
    exit(-1);
  }

  id = 0;
  while ( (fgets(line, 200, pfd_cipher) != NULL) && (id < NB_CURVE) )
  {
    sscanf(line, "%s", sz_ciphertext[id]);

    /*
      Convert from string to binary
    */
    for (n0_char = 0; n0_char < 16; n0_char++)
    {
      hexa_digit = ascii_to_bin(sz_ciphertext[id][n0_char]);
      b1_ciphertext[4 * n0_char + 3] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_ciphertext[4 * n0_char + 2] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_ciphertext[4 * n0_char + 1] = (hexa_digit & 1); hexa_digit >>= 1;
      b1_ciphertext[4 * n0_char + 0] = (hexa_digit & 1);
    }

    /*
      Compute R16, L16 and the expansive permutation output of last round
    */
    for (i = 0; i < 32; i++)
    {
      b1_R16[i] = b1_ciphertext[IP[i +  0] - OFFSET];
      b1_L16[i] = b1_ciphertext[IP[i + 32] - OFFSET];
    }
    for (i = 0; i < 48; i++)
    {
      b1_m16[i] = b1_L16[E[i] - OFFSET];

      if (i % 6 == 0) b6_m16[i / 6] = 0;

      b6_m16[i / 6] += (b1_m16[i] << (5 - (i % 6)));
    }

    // For the curve of index id, for sbox number i (numbered from 0 to 7),
    // and assuming that k is the subkey value entering in this sbox,
    // hw_15[id][i][k] designates the contribution (between 0 and 4) of this
    // sbox to the Hamming distance between L15 and R15 halves.

    for (i = 0; i < 8; i++)
    {
      for (k = 0; k < 64; k++)
      {
        b6_x = b6_m16[i] ^ k;
        b4_y = S_Box[i][b6_x];

        hw_15[id][i][k] = 0;
        for (j = 0; j < 4; j++)
        {
          hw_15[id][i][k] += ((b4_y >> (3 - j)) & 1) ^ b1_R16[inv_P[4 * i + j] - OFFSET] ^ b1_L16[inv_P[4 * i + j] - OFFSET];
        }
      }
    }

    // For the curve of index id, hw_16[id] designates Hamming distance between L16 and R16 halves.

    hw_16[id] = 0;
    for (i = 0; i < 32; i++)
    {
      hw_16[id] += b1_R16[i] ^ b1_L16[i];
    }

    id++;
  }
  fclose(pfd_cipher);


  /*
    Read in the file containing all the curve file names
    ----------------------------------------------------
  */

  // A file named "AllNames.txt" containing all the curve file names
  // must be located in the parent directory.

  sprintf(sz_file_name,"../AllNames.txt");
  if ((pfd_names = fopen(sz_file_name, "r")) == NULL)
  {
    printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
    exit(-1);
  }

  id = 0;
  while ( (fgets(line, 200, pfd_names) != NULL) && (id < NB_CURVE) )
  {
    sscanf(line, "%*u %s", sz_names[id]);
    id++;
  }
  fclose(pfd_names);


  /*
    Read in the curves
    ------------------
  */

/*
  for (id = 0; id < NB_CURVE; id++)
  {
    sprintf(sz_file_name,"../%lu/%s", id / 10000, sz_names[id]);
    if ((pfd_curve = fopen(sz_file_name, "r")) == NULL)
    {
      printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
      exit(-1);
    }

    fseek(pfd_curve, 164 + T_00 * sizeof(float), SEEK_SET);
    fread(&(data_00[id]), sizeof(float), 1, pfd_curve);

    fseek(pfd_curve, 164 + T_16 * sizeof(float), SEEK_SET);
    fread(&(data_16[id]), sizeof(float), 1, pfd_curve);

    fclose(pfd_curve);
  }
*/
/*
    sprintf(sz_file_name,"./sample_%05lu.dat", (WORD32) T_00);
    if ((pfd_curve = fopen(sz_file_name, "r")) == NULL)
    {
      printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
      exit(-1);
    }
    fread(&data_00, sizeof(float), NB_CURVE, pfd_curve);
    fclose(pfd_curve);

    sprintf(sz_file_name,"./sample_%05lu.dat", (WORD32) T_16);
    if ((pfd_curve = fopen(sz_file_name, "r")) == NULL)
    {
      printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
      exit(-1);
    }
    fread(&data_16, sizeof(float), NB_CURVE, pfd_curve);
    fclose(pfd_curve);
*/

  /*
    Initialize K00_pos and K16_pos
    ------------------------------
  */

  for (i = 0; i < 64; i++)
  {
    K00_pos[i] = -1;
    K16_pos[i] = -1;
  }
  for (i = 0; i < 48; i++)
  {
    K00_pos[K00_bit[i] - OFFSET] = i;
    K16_pos[K16_bit[i] - OFFSET] = i;
  }

  /*
    Initialize pos_in_key00 and pos_in_key16
    ----------------------------------------
  */

  for (i = 0; i < 48; i++)
  {
    pos_in_key16[i] = -1;
    for (j = 0; j < 48; j++)
    {
      if (K16_bit[j] == K00_bit[i])
      {
        pos_in_key16[i] = j;
      }
    }
  }
  for (i = 0; i < 48; i++)
  {
    pos_in_key00[i] = -1;
    for (j = 0; j < 48; j++)
    {
      if (K00_bit[j] == K16_bit[i])
      {
        pos_in_key00[i] = j;
      }
    }
  }

  nb_try_tau = - (NB_TRY_T2 - NB_TRY_T1) / log( ((float) NB_TRY_X2) / NB_TRY_X1 );
  nb_try_a = NB_TRY_X1 / exp(- NB_TRY_T1 / ((float) nb_try_tau));

  /*
    Search for a key which minimize the quadratic distance
    between observations and predictions (likelihood criterion)
  */

  printf("\n");
  printf("\n");
  printf("seed = %lu\n", seed);
  printf("\n");
  printf("NB_CURVE_BEGIN = %u\n", NB_CURVE_BEGIN);
  printf("\n");
  printf("NB_TRY_T1 = %lu\n", (WORD32) NB_TRY_T1);
  printf("NB_TRY_X1 = %lu\n", (WORD32) NB_TRY_X1);
  printf("\n");
  printf("NB_TRY_T2 = %lu\n", (WORD32) NB_TRY_T2);
  printf("NB_TRY_X2 = %lu\n", (WORD32) NB_TRY_X2);
  printf("\n");

  seed--;

  n0_run = 0;
  while(n0_run < NB_RUN)
  {  
    seed++;
    srand(seed);

    /*
      Choose NB_CURVE_USED_MAX indices at random and read the curves
      --------------------------------------------------------------
    */
    
    for (id = 0; id < NB_CURVE; id++)
    {
      b_used[id] = FALSE;
    }
    for (n0_id = 0; n0_id < NB_CURVE_USED_MAX; n0_id++)
    {
      do
      {
        id = rand() % NB_CURVE;
      }
      while(b_used[id] == TRUE);
      b_used[id] = TRUE;

      id_list[n0_id] = id;
      
      sprintf(sz_file_name,"%s/%lu/%s", CURVE_DIRECTORY, id / 10000, sz_names[id]);
      if ((pfd_curve = fopen(sz_file_name, "r")) == NULL)
      {
        printf("\nERROR : can not open file named \"%s\" for reading\n\n", sz_file_name);
        exit(-1);
      }
      fseek(pfd_curve, 164 + T_00 * sizeof(float), SEEK_SET);
      fread(&(data_00[n0_id]), sizeof(float), 1, pfd_curve);
      fseek(pfd_curve, 164 + T_16 * sizeof(float), SEEK_SET);
      fread(&(data_16[n0_id]), sizeof(float), 1, pfd_curve);
      fclose(pfd_curve);
    }


    printf("\n");
    printf("**********   Run number %lu   **********\n", n0_run);
    printf("\n");
    printf("seed = %lu\n", seed);
    printf("\n");
    fflush(NULL);
    
    nb_curve = NB_CURVE_BEGIN;

    time_begin = time(NULL);

    stability = 0;
    b_found = FALSE;

    // b_found == TRUE means the end of the run,
    // i.e. after STABILITY consecutive successful attacks.

    while (b_found == FALSE)
    {
      // Computes the number of initial key candidates as an exponentially decreasing function of the number of curves.

      nb_try = (WORD32) ceil(nb_try_a * exp(- ((float) nb_curve) / nb_try_tau));

      if (nb_try < NB_TRY_MIN) nb_try = NB_TRY_MIN;
      if (nb_try > NB_TRY_MAX) nb_try = NB_TRY_MAX;

      /*
        Initialization
      */

      for (i = 0; i < nb_try; i++)
      {
        b_stable[i] = FALSE;

        for (j = 0; j < 8; j++) b6_key00_best[i][j] = 0;
        for (j = 0; j < 8; j++) b6_key16_best[i][j] = 0;

        for (j = 0; j < 28; j++)
        {
          n0_bit = K_left_bit[j] - OFFSET;

          bit = rand() % 2;

          if (K00_pos[n0_bit] != -1) b6_key00_best[i][K00_pos[n0_bit] / 6] |= ( bit << (5 - (K00_pos[n0_bit] % 6)) );
          if (K16_pos[n0_bit] != -1) b6_key16_best[i][K16_pos[n0_bit] / 6] |= ( bit << (5 - (K16_pos[n0_bit] % 6)) );

          n0_bit = K_right_bit[j] - OFFSET;

          bit = rand() % 2;

          if (K00_pos[n0_bit] != -1) b6_key00_best[i][K00_pos[n0_bit] / 6] |= ( bit << (5 - (K00_pos[n0_bit] % 6)) );
          if (K16_pos[n0_bit] != -1) b6_key16_best[i][K16_pos[n0_bit] / 6] |= ( bit << (5 - (K16_pos[n0_bit] % 6)) );
        }

        // For each key candidate, compute its initial score (sum of likelyhoods at first and last rounds).
        // This score is currently the best one found so far for this candidate thread.

        score_best[i] = likelihood00(nb_curve, b6_key00_best[i])
                      + likelihood16(nb_curve, b6_key16_best[i]);
      }

      b_finish = FALSE;

      // b_finish == TRUE means that every key candidate thread is reached convergence
      // i.e. b_stable[i] is TRUE for all i.

      while (b_finish == FALSE)
      {
        for (i = 0; i < nb_try; i++)
        {
          score_base[i] = score_best[i];

          memcpy(b6_key00_base[i], b6_key00_best[i], 8);
          memcpy(b6_key16_base[i], b6_key16_best[i], 8);
        }

        for (i = 0; i < nb_try; i++)
        {
          if (b_stable[i] == FALSE)
          {
            // For each key candidate thread, if this thread has not reached convergence yet,
            // then a better key candidate (better than the best one found so far for this thread)
            // is searched in a neighbourhood of the current best one (b6_keyxx_base)

            memcpy(b6_key00_guess, b6_key00_base[i], 8);
            memcpy(b6_key16_guess, b6_key16_base[i], 8);

            for (sbox1 = 0; sbox1 < 4; sbox1++)
            {
              for (delta1 = 0; delta1 < 64; delta1++)
              {
                b6_key00_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key16[6 * sbox1 + j];
                  if (pos != -1) b6_key16_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }

                likelihood_00 = likelihood00(nb_curve, b6_key00_guess);

                // Considering the left path of the key (S-Boxes 1 to 4),
                // for each sbox in the first round,
                // for each differential on the subkey entering this sbox
                // the differential is applied to the key.
                // Then the key is also modified by considering all 16 differentials
                // implying the 4 key bits of the left path which
                // are not involved in the first round.

                for (b11 = 0; b11 < 2; b11++)
                {
                  b6_key16_guess[2] ^= 0x20;

                  for (b43 = 0; b43 < 2; b43++)
                  {
                    b6_key16_guess[2] ^= 0x10;

                    for (b50 = 0; b50 < 2; b50++)
                    {
                      b6_key16_guess[1] ^= 0x01;

                      for (b52 = 0; b52 < 2; b52++)
                      {
                        b6_key16_guess[2] ^= 0x02;

                        // For each one of these modified keys, its score is computed as the sum
                        // of the likelyhoods at the first and last rounds, given that key guess.

                        score_guess = likelihood_00
                                    + likelihood16(nb_curve, b6_key16_guess);

                        if (score_guess < score_best[i])
                        {
                          score_best[i] = score_guess;

                          memcpy(b6_key00_best[i], b6_key00_guess, 8);
                          memcpy(b6_key16_best[i], b6_key16_guess, 8);
                        }
                      }
                    }
                  }
                }

                b6_key00_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key16[6 * sbox1 + j];
                  if (pos != -1) b6_key16_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }


                b6_key16_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key00[6 * sbox1 + j];
                  if (pos != -1) b6_key00_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }

                likelihood_16 = likelihood16(nb_curve, b6_key16_guess);

                // Considering the left path of the key (S-Boxes 1 to 4),
                // for each sbox in the last round,
                // for each differential on the subkey entering this sbox
                // the differential is applied to the key.
                // Then the key is also modified by considering all 16 differentials
                // implying the 4 key bits of the left path which
                // are not involved in the last round.

                for (b19 = 0; b19 < 2; b19++)
                {
                  b6_key00_guess[1] ^= 0x02;

                  for (b51 = 0; b51 < 2; b51++)
                  {
                    b6_key00_guess[0] ^= 0x10;

                    for (b58 = 0; b58 < 2; b58++)
                    {
                      b6_key00_guess[2] ^= 0x01;

                      for (b60 = 0; b60 < 2; b60++)
                      {
                        b6_key00_guess[0] ^= 0x04;

                        // For each one of these modified keys, its score is computed as the sum
                        // of the likelyhoods at the first and last rounds, given that key guess.

                        score_guess = likelihood00(nb_curve, b6_key00_guess)
                                    + likelihood_16;

                        if (score_guess < score_best[i])
                        {
                          score_best[i] = score_guess;

                          memcpy(b6_key00_best[i], b6_key00_guess, 8);
                          memcpy(b6_key16_best[i], b6_key16_guess, 8);
                        }
                      }
                    }
                  }
                }

                b6_key16_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key00[6 * sbox1 + j];
                  if (pos != -1) b6_key00_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }
              }
            }

            for (sbox1 = 4; sbox1 < 8; sbox1++)
            {
              for (delta1 = 0; delta1 < 64; delta1++)
              {
                b6_key00_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key16[6 * sbox1 + j];
                  if (pos != -1) b6_key16_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }

                likelihood_00 = likelihood00(nb_curve, b6_key00_guess);

                // Considering the right path of the key (S-Boxes 5 to 8),
                // for each sbox in the first round,
                // for each differential on the subkey entering this sbox
                // the differential is applied to the key.
                // Then the key is also modified by considering all 16 differentials
                // implying the 4 key bits of the right path which
                // are not involved in the first round.

                for (b06 = 0; b06 < 2; b06++)
                {
                  b6_key16_guess[6] ^= 0x20;

                  for (b07 = 0; b07 < 2; b07++)
                  {
                    b6_key16_guess[7] ^= 0x04;

                    for (b12 = 0; b12 < 2; b12++)
                    {
                      b6_key16_guess[4] ^= 0x01;

                      for (b46 = 0; b46 < 2; b46++)
                      {
                        b6_key16_guess[6] ^= 0x08;

                        // For each one of these modified keys, its score is computed as the sum
                        // of the likelyhoods at the first and last rounds, given that key guess.

                        score_guess = likelihood_00
                                    + likelihood16(nb_curve, b6_key16_guess);

                        if (score_guess < score_best[i])
                        {
                          score_best[i] = score_guess;

                          memcpy(b6_key00_best[i], b6_key00_guess, 8);
                          memcpy(b6_key16_best[i], b6_key16_guess, 8);
                        }
                      }
                    }
                  }
                }

                b6_key00_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key16[6 * sbox1 + j];
                  if (pos != -1) b6_key16_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }


                b6_key16_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key00[6 * sbox1 + j];
                  if (pos != -1) b6_key00_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }

                likelihood_16 = likelihood16(nb_curve, b6_key16_guess);

                // Considering the right path of the key (S-Boxes 5 to 8),
                // for each sbox in the last round,
                // for each differential on the subkey entering this sbox
                // the differential is applied to the key.
                // Then the key is also modified by considering all 16 differentials
                // implying the 4 key bits of the right path which
                // are not involved in the last round.

                for (b14 = 0; b14 < 2; b14++)
                {
                  b6_key00_guess[7] ^= 0x10;

                  for (b15 = 0; b15 < 2; b15++)
                  {
                    b6_key00_guess[6] ^= 0x02;

                    for (b20 = 0; b20 < 2; b20++)
                    {
                      b6_key00_guess[6] ^= 0x01;

                      for (b54 = 0; b54 < 2; b54++)
                      {
                        b6_key00_guess[4] ^= 0x04;

                        // For each one of these modified keys, its score is computed as the sum
                        // of the likelyhoods at the first and last rounds, given that key guess.

                        score_guess = likelihood00(nb_curve, b6_key00_guess)
                                    + likelihood_16;

                        if (score_guess < score_best[i])
                        {
                          score_best[i] = score_guess;

                          memcpy(b6_key00_best[i], b6_key00_guess, 8);
                          memcpy(b6_key16_best[i], b6_key16_guess, 8);
                        }
                      }
                    }
                  }
                }

                b6_key16_guess[sbox1] ^= delta1;
                for (j = 0; j < 6; j++)
                {
                  pos = pos_in_key00[6 * sbox1 + j];
                  if (pos != -1) b6_key00_guess[pos / 6] ^= ( ((delta1 >> (5 - j)) & 1) << (5 - (pos % 6)) );
                }
              }
            }


            // Starting from the best key found in the above considered neighbourhood,
            // all differentials implying the 8 bits not involved
            // in the last round key K16 are considered.

            memcpy(b6_key00_guess, b6_key00_best[i], 8);
            memcpy(b6_key16_guess, b6_key16_best[i], 8);

            for (b14 = 0; b14 < 2; b14++)
            {
              b6_key00_guess[7] ^= 0x10;

              for (b15 = 0; b15 < 2; b15++)
              {
                b6_key00_guess[6] ^= 0x02;

                for (b20 = 0; b20 < 2; b20++)
                {
                  b6_key00_guess[6] ^= 0x01;

                  for (b54 = 0; b54 < 2; b54++)
                  {
                    b6_key00_guess[4] ^= 0x04;

                    for (b19 = 0; b19 < 2; b19++)
                    {
                      b6_key00_guess[1] ^= 0x02;

                      for (b51 = 0; b51 < 2; b51++)
                      {
                        b6_key00_guess[0] ^= 0x10;

                        for (b58 = 0; b58 < 2; b58++)
                        {
                          b6_key00_guess[2] ^= 0x01;

                          for (b60 = 0; b60 < 2; b60++)
                          {
                            b6_key00_guess[0] ^= 0x04;

                            score_guess = likelihood00(nb_curve, b6_key00_guess)
                                        + likelihood16(nb_curve, b6_key16_guess);

                            if (score_guess < score_best[i])
                            {
                              score_best[i] = score_guess;

                              memcpy(b6_key00_best[i], b6_key00_guess, 8);
                              memcpy(b6_key16_best[i], b6_key16_guess, 8);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }

            // Then, all differentials implying the 8 bits not involved
            // in the first round key K1 are considered.

            for (b11 = 0; b11 < 2; b11++)
            {
              b6_key16_guess[2] ^= 0x20;

              for (b43 = 0; b43 < 2; b43++)
              {
                b6_key16_guess[2] ^= 0x10;

                for (b50 = 0; b50 < 2; b50++)
                {
                  b6_key16_guess[1] ^= 0x01;

                  for (b52 = 0; b52 < 2; b52++)
                  {
                    b6_key16_guess[2] ^= 0x02;

                    for (b06 = 0; b06 < 2; b06++)
                    {
                      b6_key16_guess[6] ^= 0x20;

                      for (b07 = 0; b07 < 2; b07++)
                      {
                        b6_key16_guess[7] ^= 0x04;

                        for (b12 = 0; b12 < 2; b12++)
                        {
                          b6_key16_guess[4] ^= 0x01;

                          for (b46 = 0; b46 < 2; b46++)
                          {
                            b6_key16_guess[6] ^= 0x08;

                            score_guess = likelihood00(nb_curve, b6_key00_guess)
                                        + likelihood16(nb_curve, b6_key16_guess);

                            if (score_guess < score_best[i])
                            {
                              score_best[i] = score_guess;

                              memcpy(b6_key00_best[i], b6_key00_guess, 8);
                              memcpy(b6_key16_best[i], b6_key16_guess, 8);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }


            // For the current key candidate thread,
            // if a better key has been found in the neighbourhood of the base candidate
            // then the base candidate is updated with this better one.
            // Else the key candidate thread is flagged as stable.

            if (score_best[i] < 0.999 * score_base[i])
            {
              score_base[i] = score_best[i];

              memcpy(b6_key00_base[i], b6_key00_best[i], 8);
              memcpy(b6_key16_base[i], b6_key16_best[i], 8);
            }
            else
            {
              b_stable[i] = TRUE;
            }
          }
        }

        // An attack is terminated once every key candidate thread is stable.

        b_finish = TRUE;
        for (i = 0; i < nb_try; i++)
        {
          if (b_stable[i] == FALSE) b_finish = FALSE;
        }
      }

      // Identify the key which reveals to be the overall best among all threads.

      n0_best_key = 0;
      for (i = 0; i < nb_try; i++)
      {
        if (score_base[i] < score_base[n0_best_key]) n0_best_key = i;
      }

      // Compute the score of the correct key for comparison purpose.

      score_correct_key = likelihood00(nb_curve, b6_correct_key00)
                        + likelihood16(nb_curve, b6_correct_key16);

      printf("%lu |  %3lu (%2lu) :  %11.6f %11.6f  ", time(NULL), nb_curve, nb_try, score_correct_key, score_base[n0_best_key]);

      for (j = 0; j < 8; j++)
      {
        printf(" %2u", b6_key00_best[n0_best_key][j]);
      }
      printf("  - ");
      for (j = 0; j < 8; j++)
      {
        printf(" %2u", b6_key16_best[n0_best_key][j]);
      }
      if ((memcmp(b6_key00_best[n0_best_key], b6_correct_key00, 8) == 0) && (memcmp(b6_key16_best[n0_best_key], b6_correct_key16, 8) == 0))
      {
        printf("  ->  OK");
        stability++;
        if (stability == STABILITY)
        {
          b_found = TRUE;
          time_end = time(NULL);
          printf("\n");
          printf("The key has been found with %lu curves in %lu seconds\n", nb_curve - STABILITY + 1, time_end - time_begin);
        }
      }
      else
      {
        stability = 0;
        time_begin = time(NULL);
      }
      printf("\n");
      fflush(NULL);

      nb_curve++;
    }
    
    n0_run++;
  }

  exit(0);
}
