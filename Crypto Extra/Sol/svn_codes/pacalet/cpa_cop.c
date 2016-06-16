/**********************************************************************************
Copyright Institut Telecom
Contributors: Renaud Pacalet (renaud.pacalet@telecom-paristech.fr)

This software is a computer program whose purpose is to exploit power traces
in order to retrieve a Data Encryption Standard (DES) secret key.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms. For more
information see the LICENCE-fr.txt or LICENSE-en.txt files.
**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include <utils.h>		/* Utility functions (errors, memory allocation) */
#include <des.h>		/* DES processing */
#include <traces.h>		/* Power traces processing */
#include <tr_pcc.h>		/* traces/scalars Pearson Correlation Coefficients (PCC) */
#include <km.h>			/* Key partial knowledge management */
#include <cop.h>		/* Cooperative optimization */

#define START_ATTACK 50		/* Attack at least START_ATTACK traces, never less */

/* Power models. Standard = transitions count Optimized = count twice
 * transitions on duplicated bits of R */
typedef enum
{ NONE, STANDARD, OPTIMIZED } modes;

struct params_s
{
  modes pwr;   /* Power model */
  int ntraces; /* Total number of traces */
  int nbits;   /* Number of bits to retreive */
  int nsets;   /* Number of traces set on which algorithm must be evaluated */
};

typedef struct params_s params; /* Type renaming */

/* Data structure to hold the correspondence between the sub-function of an
 * agent of the cooperative optimization algorithm and the sub-keys. */
struct sf2sk_s
{
  int s;			/* Sub-key number (or -1 if none). */
  int *val;			/* Array of sub-key values for each valuation of the variables in
				   the sub-function support. */
};

typedef struct sf2sk_s sf2sk;	/* Type renaming. */

int period; /* Samples per clock period (traces length / 32) */
int *l2l;   /* Array of p.ntraces L0->L1 transitions counts */

/* r2r[r][s][g] is an array of p.ntraces transition counts from R0->R1 (r=0) or
 * R14->R15 (r=1) when considering only the 4 output bits of SBox s under
 * sub-key guess g. */
int *r2r[2][8][64];

int *traces_list; /* Array of p.ntraces integers used to randomize the traces order */

/* cd2rk[r][b] is the index of bit b+1 of C0D0 in round key K1 (r=0) or K16 (r=1) */
int cd2rk[2][56];

/* rk2cd[r][b] is the index of bit b+1 of round key K1 (r=0) or K16 (r=1) in C0D0 */
int rk2cd[2][48];

uint64_t c0d0; /* Actual C0D0 */

/* Data structure storing all the plaintexts, ciphertexts, power traces, ... See traces.h */
tr_context tr_ctx;

/* Data structure for cooperative optimization. See cop.h */
cop_context cop_ctx;

/* Converter from cooperative optimization subfunctions domains to sub-keys
 * domains. sf2s[b][r].s is the sub-key of round key K1 (r=0) or K16 (r=1) to
 * in which bit b+1 of C0D0 can be found (-1 if bit is not in round key).
 * sf2s[b][r].val is an array of 2^n integers (n being the number of neighbor
 * bits of key bit #b) if bit b+1 is in round key K1 (r=0) or K16 (r=1) (NULL if
 * bit is not in round key). If sf2s[b][r].val is not NULL, sf2s[b][r].val[v] is
 * the value taken by sub-key s of round key K1 (r=0) or K16 (r=1) when the
 * neighbor bits of key bit #b take value v. In v, C0D0 bits are ordered from
 * lowest index on the left to highest index on the right. In sub-keys values,
 * bits are ordered like in round key. */
sf2sk sf2s[56][2];

/* The parameters passed when calling the program */
params p;

/* The name of the logfile */
char *fname_template = "cpa_cop.log.XXXXXX";
char *fname;

/* ANSI sequences for printing report messages */
#define SETPOS "[1G"
#define TRACESPOS "[7G"
#define HITSPOS "[14G"
#define AVERAGEPOS "[19G"
#define EXTRAPOS "[27G"
#define CLRL "[K"

/* Parse and check command line options */
void parse_options (int argc, char **argv);

/* Initialize global variables */
void initialize_data_structures (int argc, char **argv);

/* Randomize the traces order in traces_list */
void randomize_traces (void);

/* Apply CPA-COP attack on traces of traces_list. Return -1 on failure and
 * number of traces (including the 100 used for the stability check) on success.
 * */
int cpa_cop (void);

/* Deallocate global variables */
void free_data_structures (void);

int
main (int argc, char **argv)
{
  int i, cnt, stats;

  parse_options (argc, argv);
  initialize_data_structures (argc, argv);
  stats = 0;
  fprintf (stderr, SETPOS "%5s", "set");
  fprintf (stderr, TRACESPOS "%6s", "traces");
  fprintf (stderr, HITSPOS "%4s", "hits");
  fprintf (stderr, AVERAGEPOS "%7s\n", "average");
  flog (fname, "  set traces hits average\n"); /* Log file */
  for (i = 0; i < p.nsets; i++)	/* For sets */
    {
      fprintf (stderr, SETPOS "%5d ", i + 1);
      flog (fname, "%5d ", i + 1); /* Log file */
      randomize_traces (); /* Rondomization */
      cnt = cpa_cop ();    /* Attack */
      if (cnt != -1)		/* If attack successful */
	{
	  stats += cnt;
	  fprintf (stderr, AVERAGEPOS "%7.3f\n",
		   (double) (stats) / (double) (i + 1));
	  flog (fname, "%7.3f\n", (double) (stats) / (double) (i + 1));
	}
      else
	{
	  fprintf (stderr, AVERAGEPOS "failed\n");
	  flog (fname, "failed\n");
	  break;
	}
    }				/* For traces counts */
  free_data_structures ();
  return 0;
}

void
usage (char *program_name)
{
  fprintf (stderr, "usage: %s [OPTION]... TRACEFILE [TRACEFILE...]\n",
	   program_name);
  fprintf (stderr,
	   "Apply a cooperative optimization algorithm to retrieve the 56 bits\n");
  fprintf (stderr,
	   "DES secret key from a set of power traces, plain texts and cipher texts.\n\n");
  fprintf (stderr,
	   "Mandatory arguments to long options are mandatory for short options too.\n");
  fprintf (stderr,
	   "  -n, --nsets=N        number of traces sets to attack (default: 1)\n");
  fprintf (stderr,
	   "  -b, --bits=B         number of bits to retrieve without brute force (default: 48)\n");
  fprintf (stderr,
	   "  -p, --power          use the standard power model (default: off)\n");
  fprintf (stderr,
	   "  -o, --optimized      use the optimized power model (default: on)\n");
  fprintf (stderr, "  -h, --help           display this help and exit\n\n");
  exit (-1);
}

void
parse_options (int argc, char **argv)
{
  int o;

  p.nsets = 1;
  p.nbits = 48;
  p.pwr = NONE;
  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
	{"nsets", 1, 0, 'n'},
	{"bits", 1, 0, 'b'},
	{"power", 0, 0, 'p'},
	{"optimized", 0, 0, 'o'},
	{"help", 0, 0, 'h'},
	{0, 0, 0, 0}
      };
      o = getopt_long (argc, argv, "n:b:poh", long_options, &option_index);
      if (o == -1)
	{
	  break;
	}
      switch (o)
	{
	case 'n':
	  p.nsets = atoi (optarg);
	  break;
	case 'b':
	  p.nbits = atoi (optarg);
	  break;
	case 'p':
	  if (p.pwr != NONE)
	    {
	      usage (argv[0]);
	    }
	  p.pwr = STANDARD;
	  break;
	case 'o':
	  if (p.pwr != NONE)
	    {
	      usage (argv[0]);
	    }
	  p.pwr = OPTIMIZED;
	  break;
	default:
	  usage (argv[0]);
	  break;
	}
    }
  if (optind >= argc)
    {
      usage (argv[0]);
    }
  if (p.nsets < 1)
    {
      error (__func__, "invalid number of sets: %d", p.nsets);
    }
  if ((p.nbits < 1) || (p.nbits > 56))
    {
      error (__func__, "invalid number of bits to retrieve: %d", p.nbits);
    }
  if (p.pwr == NONE)
    {
      p.pwr = OPTIMIZED;
    }
}

/* E permutation table as in the DES standard */
int e_table[48] = {
  32, 1, 2, 3, 4, 5,
  4, 5, 6, 7, 8, 9,
  8, 9, 10, 11, 12, 13,
  12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21,
  20, 21, 22, 23, 24, 25,
  24, 25, 26, 27, 28, 29,
  28, 29, 30, 31, 32, 1
};

void
create_power_models (void)
{
  int i, g, s, n, bc[32];
  uint64_t pt, l0r0, r0, l0, er0, npr0xl0, ct, r16l16, l16, r16, er15;
  uint64_t npr16xl16, npr0xr1[2], npr14xr15[2], rk, power_modifier;

  n = tr_number (tr_ctx);	/* Total number of traces */
  for (i = 0; i < 32; i++)	/* For bits in R register */
    {
      bc[i] = 0;		/* Reset bit count */
    }				/* For bits in R register */
  for (i = 0; i < 48; i++)	/* For output bits of E permutation */
    {
      /* Increment bit count for the corresponding input bit */
      bc[e_table[i] - 1] += 1;
    }				/* For output bits of E permutation */
  power_modifier = ZERO64;	/* Reset power modifier */
  if (p.pwr == OPTIMIZED)
    {
      for (i = 0; i < 32; i++)	/* For bits in R register */
	{
	  power_modifier <<= 1;
	  if (bc[i] == 2)	/* If bit enters 2 SBoxes */
	    {
	      power_modifier |= ONE64;	/* Set corresponding bit in power modifier */
	    }
	}			/* For bits in R register */
      power_modifier = des_n_p (power_modifier);	/* Apply inverse P permutation */
    }
  for (i = 0; i < n; i++)	/* For all traces */
    {
      pt = tr_plaintext (tr_ctx, i);	/* Get plain text */
      l0r0 = des_ip (pt);
      r0 = des_right_half (l0r0);
      l0 = des_left_half (l0r0);
      l2l[i] = hamming_distance (l0, r0);
      er0 = des_e (r0);
      npr0xl0 = des_n_p (r0 ^ l0);
      ct = tr_ciphertext (tr_ctx, i);	/* Get cipher text */
      r16l16 = des_ip (ct);
      l16 = des_right_half (r16l16);
      r16 = des_left_half (r16l16);
      er15 = des_e (l16);
      npr16xl16 = des_n_p (l16 ^ r16);
      /* For all 64 guesses. rk is a 48 round key in which each sub-key is set
       * to the current guess. */
      for (g = 0, rk = ZERO64; g < 64; g++, rk += UINT64_C (0x041041041041))
	{
	  /* Compute inverse P permutation of distance R0 xor R1 */
	  npr0xr1[0] = npr0xl0 ^ des_sboxes (er0 ^ rk);
	  /* Apply power modifier */
	  npr0xr1[1] = npr0xr1[0] & power_modifier;
	  /* Compute inverse P permutation of distance R14 xor R15 */
	  npr14xr15[0] = npr16xl16 ^ des_sboxes (er15 ^ rk);
	  /* Apply power modifier */
	  npr14xr15[1] = npr14xr15[0] & power_modifier;
	  for (s = 7; s >= 0; s--)	/* For 8 sub-keys */
	    {
	      /* Store power models */
	      r2r[0][s][g][i] = hamming_weight (npr0xr1[0] & UINT64_C (0xf)) +
		hamming_weight (npr0xr1[1] & UINT64_C (0xf));
	      r2r[1][s][g][i] =
		hamming_weight (npr14xr15[0] & UINT64_C (0xf)) +
		hamming_weight (npr14xr15[1] & UINT64_C (0xf));
	      /* Shift distances */
	      npr0xr1[0] >>= 4;
	      npr0xr1[1] >>= 4;
	      npr14xr15[0] >>= 4;
	      npr14xr15[1] >>= 4;
	    }			/* For 8 sub-keys */
	}			/* For 64 guesses */
    }				/* For all traces */
}

/* Translates a valuation of the n1 secret key bits which indices are in list1
 * into a valuation of a n2 bits subset which indices are in list2. */
uint64_t
projection (uint64_t val, int n1, int *list1, int n2, int *list2)
{
  uint64_t res;
  int i;
  int j;
  int idx;

  if ((n1 > 64) || (n2 > 64))
    {
      error (__func__, "maximum list length exceeded: %d and %d", n1, n2);
    }
  res = ZERO64;
  for (i = 0; i < n2; i++)	/* For indices in list2 */
    {
      res <<= 1;
      idx = list2[i];		/* Get index */
      for (j = 0; j < n1; j++)	/* For indices in list1 */
	{
	  if (list1[j] == idx)	/* If match */
	    {
	      break;
	    }
	}			/* For indices in list1 */
      if (j == n1)		/* If bit of list2 not in list1 */
	{
	  error (__func__, "destination is not a subset of source");
	}
      res |= (val >> (n1 - 1 - j)) & ONE64;	/* Set bit in result */
    }				/* For indices in list2 */
  return res;
}

/* PC-2 permutation table as in the DES standard */
int pc2_table[48] = {
  14, 17, 11, 24, 1, 5,
  3, 28, 15, 6, 21, 10,
  23, 19, 12, 4, 26, 8,
  16, 7, 27, 20, 13, 2,
  41, 52, 31, 37, 47, 55,
  30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53,
  46, 42, 50, 36, 29, 32
};

void
create_neighborhoods (void)
{
  int i, j, k, r, nn, nb, nbs[11];
  agent a;

  for (i = 0; i < 56; i++)	/* For 56 bits of C0D0 secret key */
    {
      for (j = 0; j < 11; j++)	/* For up to 11 neighbors */
	{
	  nbs[j] = -1;		/* Reset neighbor to "none" */
	}			/* For up to 11 neighbors */
      a = cop_ctx->a + i;	/* Current bit's agent */
      nn = 0;			/* Reset number of neighbors */
      for (r = 0; r < 2; r++)	/* For first (r=0) and last (r=1) round keys */
	{
	  if (cd2rk[r][i] != -1)	/* If current bit is in round key */
	    {
	      sf2s[i][r].s = cd2rk[r][i] / 6;	/* Set sub-key number */
	      for (j = 0; j < 6; j++)	/* For 6 bits of sub-key */
		{
		  /* Get index of neighbor bit */
		  nb = rk2cd[r][6 * sf2s[i][r].s + j];
		  for (k = 0; k < nn; k++)	/* For already known neighbors */
		    {
		      if (nbs[k] == nb)	/* If current neighbor already known */
			{
			  break;
			}
		    }		/* For already known neighbors */
		  if (k == nn)	/* If neighbor not already known */
		    {
		      nbs[k] = nb;	/* Store new neighbor */
		      nn += 1;	/* Increment neighbors count */
		    }
		}		/* For 6 bits of sub-key */
	    }
	  else			/* Current bit not in round key */
	    {
	      sf2s[i][r].s = -1;	/* Set sub-key number to "none" */
	    }
	}			/* For first (r=0) and last (r=1) round keys */
      a->nn = nn;		/* Set neighbor count of agent */
      a->w = 1.0 / (double) (nn - 1);	/* Set cooperation weight of agent */
      /* Allocate neighbors table */
      a->nbs = xcalloc (nn, sizeof (int), __func__);
      for (k = 0; k < nn; k++)	/* For all neighbors */
	{
	  a->nbs[k] = nbs[k];	/* Store neighbor index */
	}			/* For all neighbors */
      /* Allocate sub-function */
      a->e = xcalloc ((1 << nn), sizeof (double), __func__);
      for (r = 0; r < 2; r++)	/* For first (r=0) and last (r=1) round keys */
	{
	  if (sf2s[i][r].s != -1)	/* If bit in round key */
	    {
	      /* Allocate sub-function valuations to sub-key valuations converter */
	      sf2s[i][r].val = xcalloc ((1 << nn), sizeof (int), __func__);
	      for (j = 0; j < (1 << nn); j++)	/* For valuations of neighborhood */
		{
		  /* Set valuation of sub-key */
		  sf2s[i][r].val[j] =
		    (int) (projection
			   ((uint64_t) (j), nn, a->nbs, 6,
			    &(rk2cd[r][sf2s[i][r].s * 6])));
		}		/* For valuations of neighborhood */
	    }
	  else			/* Bit not in round key */
	    {
	      /* No converter */
	      sf2s[i][r].val = NULL;
	    }
	}			/* For first (r=0) and last (r=1) round keys */
    }				/* For 56 bits of C0D0 secret key */
}

void
initialize_data_structures (int argc, char **argv)
{
  int i, r, s, g, idx, fp;

  tr_ctx = tr_init_context ();	/* Initialize trace context */
  for (i = optind; i < argc; i++)	/* For all trace files */
    {
      /* Load traces from trace file in trace context */
      tr_load_context (tr_ctx, argv[i], 0);
    }				/* For all trace files */
  p.ntraces = tr_number (tr_ctx);	/* Total number of traces in trace context */
  /* If number of traces less than minimum number of traces for an attack */
  if (p.ntraces < START_ATTACK)
    {
      error (__func__, "invalid number of traces: %d (minimum = %d)",
	     p.ntraces, START_ATTACK);
    }
  c0d0 = des_pc1 (tr_key (tr_ctx));	/* Actual 56 bits secret key */
  /* Allocate the l2l and r2r transition counts arrays */
  l2l = xcalloc (p.ntraces, sizeof (int), __func__);
  for (r = 0; r < 2; r++) /* For first and last round keys */
    {
      for (s = 0; s < 8; s++)	/* For 8 sub-keys */
	{
	  for (g = 0; g < 64; g++)	/* For 64 guesses */
	    {
	      r2r[0][s][g] = xcalloc (p.ntraces, sizeof (int), __func__);
	      r2r[1][s][g] = xcalloc (p.ntraces, sizeof (int), __func__);
	    }			/* For 64 guesses */
	}			/* For 8 sub-keys */
    } /* For first and last round keys */
  for (i = 0; i < 56; i++)	/* For 56 bits of C0D0 secret key */
    {
      /* Corresponding bit of first and last round keys initialized to "none" */
      cd2rk[0][i] = -1;
      cd2rk[1][i] = -1;
    }				/* For 56 bits of C0D0 secret key */
  for (i = 0; i < 48; i++)	/* For 48 bits of round keys */
    {
      idx = pc2_table[i] - 1;	/* Get index in C0D0=C16D16 */
      rk2cd[1][i] = idx;	/* Set last round key to C0D0 converter */
      cd2rk[1][idx] = i;	/* Set C0D0 to last round key converter */
      /* Get index in LS(C0DO)=C1D1 */
      idx = (idx + 1) % 28 + ((idx >= 28) ? 28 : 0);
      rk2cd[0][i] = idx;	/* Set first round key to C0D0 converter */
      cd2rk[0][idx] = i;	/* Set C0D0 to first round key converter */
    }				/* For 48 bits of round keys */
  /* Compute power models for all traces */
  create_power_models ();
  /* Initialize cooperative optimization context */
  cop_ctx = cop_init (56);
  /* Initialize neighborhoods */
  create_neighborhoods ();
  /* Number of samples per clock period is traces length divided by 32 */
  period = tr_length (tr_ctx) / 32;
  /* Allocate and initialize an n-long trace list */
  traces_list = xcalloc (p.ntraces, sizeof (int), __func__);
  for (i = 0; i < p.ntraces; i++)
    {
      traces_list[i] = i;
    }
  /* Allocate log file name */
  fname = xcalloc (strlen (fname_template) + 1, sizeof (char), __func__);
  strcpy (fname, fname_template);
  fp = mkstemp (fname); /* Create unique log file */
  fprintf (stderr, "log file: %s\n\n", fname); /* Print log file name */
  close (fp); /* Close log file */
}

void
randomize_traces (void)
{
  int i, j, tmp;

  for (i = 0; i < p.ntraces; i++)	/* For all trace indexes in traces_list */
    {
    /* Permutate index traces_list[i] and a random, different index traces_list[j] */
      tmp = traces_list[i];
      j = rand () % (p.ntraces - 1);
      if (j >= i)
	{
	  j += 1;
	}
      traces_list[i] = traces_list[j];
      traces_list[j] = tmp;
    }
}

int
cpa_cop (void)
{
  int l, cnt, i, r, n, s, j, tr, g, idx, b, from[2], to[2], width[2], br, bs;
  int bg, stop;

  /* Data structures for Pearson correlation coefficients computations (see tr_pcc.h) */
  tr_pcc_context tr_pcc_ctx[3];

  uint64_t guessed_c0d0, mask, val, kg, consensus, decision;
  double *t, max, min, sum, lk[2][8][64], confidence;
  agent a;

  /* Management of partial knowledge on secret key (see km.h) */
  des_key_manager km;

  l = tr_length (tr_ctx) / 2; /* Half of traces length */
  cnt = 0; /* Hit counter. We return when it reaches 100. */
  max = 0.0;

  /* Initialize PCC context #2 for one single PCC trace between L0->L1
   * transition counts and power traces */
  tr_pcc_ctx[2] = tr_pcc_init (l, 1);

  for (i = 0; i <= p.ntraces; i++) /* For all traces in randomized list */
    {
      t = tr_trace (tr_ctx, traces_list[i]); /* t points to power trace */

      /* Add first half of power trace to PCC manager */
      tr_pcc_insert_x (tr_pcc_ctx[2], t);

      /* Add L0->L1 transitions count to PCC manager */
      tr_pcc_insert_y (tr_pcc_ctx[2], 0, l2l[traces_list[i]]);

      if (i < START_ATTACK) /* If not enough traces to start attack */
	{
	  continue;
	}

      tr_pcc_consolidate (tr_pcc_ctx[2], UNBIASED); /* Finalize PCC computation */
      t = tr_pcc_get_pcc (tr_pcc_ctx[2], 0); /* t = PCC trace */
      max = tr_max (l, t, &idx); /* [max,idx] are max and argmax of PCC trace */
      from[0] = idx - 1; /* Search for positive area of PCC trace around argmax */
      while ((t[from[0]] >= 0.0) && (from[0] > 0))
	{
	  from[0] -= 1;
	}
      if ((t[from[0]] < 0.0) && (from[0] < (l - 1)))
	{
	  from[0] += 1;
	}
      to[0] = idx + 1;
      while ((t[to[0]] >= 0.0) && (to[0] < l - 1))
	{
	  to[0] += 1;
	}
      if ((t[to[0]] < 0.0) && (to[0] > 0))
	{
	  to[0] -= 1;
	}
      width[0] = to[0] - from[0] + 1;

      /* Search window for R14->R15 transitions is the same as R0->R1 shifted
       * ahead by 14 clock periods */
      from[1] = from[0] + 14 * period;
      to[1] = to[0] + 14 * period;
      width[1] = width[0];

      /* Initialize key knowledge manager to zero knowledge */
      km = des_km_init ();
      stop = 0;
      while (!stop)
	{
	  for (r = 0; r < 2; r++) /* For first and last round keys */
	    {
	      n = 0; /* Number of surviving candidate sub-keys values */
	      for (s = 0; s < 8; s++)	/* For 8 sub-keys */
		{
                  /* Get knowledge about sub-key */
		  des_km_get_sk (km, 1 + 15 * r, s + 1, &mask);
		  j = hamming_weight (mask); /* Number of known bits */
                  /* Accumulate number of surviving candidate sub-keys values */
		  n += (1 << (6 - j));
                }
              /* Initialize PCC context for width[r] samples and n candidate
               * sub-keys values */
	      tr_pcc_ctx[r] = tr_pcc_init (width[r], n);
	    } /* For first and last round keys */

	  for (j = 0; j < i; j++) /* For #i first traces of randomized traces list */
	    {
	      t = tr_trace (tr_ctx, traces_list[j]); /* t = power trace */
	      for (r = 0; r < 2; r++) /* For first and last round keys */
		{
		  /* Insert search window of power trace into PCC context */
		  tr_pcc_insert_x (tr_pcc_ctx[r], t + from[r]);
		  tr = 0; /* Reset known transitions count */
		  for (s = 0; s < 8; s++)	/* For 8 sub-keys */
		    {
                      /* Get knowledge about sub-key */
		      kg = des_km_get_sk (km, 1 + 15 * r, s + 1, &mask);
		      if (mask == UINT64_C (0x3f)) /* If sub-key completely known */
			{
                          /* Accumulate corresponding transitions count (BS-CPA) */
			  tr += r2r[r][s][kg][traces_list[j]];
			}
		    }
		  n = 0; /* Counter of surviving candidate sub-keys values */
		  for (s = 0; s < 8; s++)	/* For 8 sub-keys */
		    {
                      /* Get knowledge about sub-key */
		      kg = des_km_get_sk (km, 1 + 15 * r, s + 1, &mask);
                      /* For all valuations on unknown bits of sub-key */
		      for (val = ZERO64; val < UINT64_C (64);
			   val = (val + mask + ONE64) & (~mask))
			{
                          /* Add known plus guessed transitions count to PCC context */
			  tr_pcc_insert_y (tr_pcc_ctx[r], n,
					   r2r[r][s][val | kg][traces_list[j]]
					   + tr);
			  n += 1; /* Next surviving candidate sub-keys values */
			}
		    }		/* For 8 sub-keys */
		} /* For first and last round keys */
	    } /* For #i first traces of randomized traces list */

          /* Finalize PCC computations */
	  tr_pcc_consolidate (tr_pcc_ctx[0], UNBIASED);
	  tr_pcc_consolidate (tr_pcc_ctx[1], UNBIASED);
	  for (r = 0; r < 2; r++) /* For first and last round keys */
	    {
	      n = 0; /* Counter of surviving candidate sub-keys values */
	      for (s = 0; s < 8; s++) /* For 8 sub-keys */
		{
                  /* Get knowledge about sub-key */
		  kg = des_km_get_sk (km, 1 + 15 * r, s + 1, &mask);
		  sum = 0.0; /* Reset sum */
		  for (g = 0; g < 64; g++) /* For all guesses on sub-key */
		    {
		      lk[r][s][g] = 0.0; /* Reset likelihood ratio */
		    }
                  /* For all valuations on unknown bits of sub-key */
		  for (val = ZERO64; val < UINT64_C (64);
		       val = (val + mask + ONE64) & (~mask))
		    {
		      t = tr_pcc_get_pcc (tr_pcc_ctx[r], n); /* PCC trace */
                      /* Max and argmax of PCC trace */
		      max = tr_max (width[r], t, &idx) + 1.0;
		      lk[r][s][val | kg] = max; /* Update likelihood ratio */
		      sum += max; /* Accumulate sum of LRs for this sub-key */
		      n += 1; /* Next surviving candidate sub-keys values */
		    }
		  if (sum != 0.0) /* Avoid division by zero */
		    {
		      for (g = 0; g < 64; g++) /* For all guesses on sub-key */
			{
			  lk[r][s][g] /= sum; /* Compute likelihood ratio */
			} /* For all guesses on sub-key */
		    }
		}		/* For 8 sub-keys */
	    } /* For first and last round keys */
	  /* Free PCC contexts */
	  tr_pcc_free (tr_pcc_ctx[0]);
	  tr_pcc_free (tr_pcc_ctx[1]);
	  /* Compute sub-functions from PCCs */
	  for (j = 0; j < 56; j++)	/* For 56 bits of secret key */
	    {
	      a = cop_ctx->a + j;	/* Bit's agent */
	      /* For neighborhood valuations */
	      for (val = ZERO64; val < (ONE64 << a->nn); val++)
		{
		  a->e[val] = 1.0;	/* Reset sub-function value for valuation */
		  for (r = 0; r < 2; r++)	/* For first (r=0) and last (r=1) round keys */
		    {
		      if (sf2s[j][r].s != -1)	/* If bit in round key */
			{
			  /* Add corresponding LK to sub-function value for valuation */
			  a->e[val] *=
			    lk[r][sf2s[j][r].s][sf2s[j][r].val[val]];
			}
		    }		/* For first (r=0) and last (r=1) round keys */
                  /* Likelihood of val -> likelihood not val */
		  a->e[val] = 1.0 - a->e[val];
		}		/* For neighborhood valuations */
	    }			/* For 56 bits of secret key */
	  /* Initialize cooperative optimization agents */
	  cop_init_agents (cop_ctx);
	  /* Iterate cooperative optimization until stability or ridiculous
           * number of iterations */
	  for (j = 1;
	       (j < 10000)
	       && (cop_iterate (cop_ctx, 1.0 - 1.0 / (double) (j))); j++);
	  if (j == 10000) /* If ridiculous number of iterations */
	    {
              /* Crash: this algorithm does not work, let's try something else! */
	      error (__func__, "did not reach stability in 10000 iterations");
	    }
	  max = 0.0; /* Max confidence */
	  br = 0;    /* Best round key */
	  bs = 0;    /* Best sub-key */
	  bg = 0;    /* Best guess on sub-key */
	  stop = 1;  /* Assume we will stop there for this traces number */
	  for (r = 0; r < 2; r++) /* For first and last round keys */
	    {
	      for (s = 0; s < 8; s++) /* For 8 sub-keys */
		{
                  /* Get knowledge about sub-key */
		  kg = des_km_get_sk (km, 1 + 15 * r, s + 1, &mask);
		  consensus = ZERO64; /* Reset consensus on sub-key */
		  confidence = 1.0;   /* Reset confidence on sub-key */
		  decision = ZERO64;  /* Reset preferred value of sub-key */
		  for (b = 0; b < 6; b++) /* For 6 bits of sub-key */
		    {
		      idx = rk2cd[r][s * 6 + b]; /* Index of bit in C0D0 */
		      consensus <<= 1;
                      /* If consensus among agents about bit's preferred value */
		      if (cop_consensus (cop_ctx, idx))
			{
			  consensus |= ONE64; /* Set corresponding bit */
			}
		      decision <<= 1;
                      /* If preferred value of bit is 1 */
		      if (cop_decision (cop_ctx, idx))
			{
			  decision |= ONE64; /* Set corresponding bit */
			}
                      /* Update confidence on sub-key */
		      confidence *= cop_confidence (cop_ctx, idx);
		    } /* For 6 bits of sub-key */

                  /* If consensus on sub-key and
                   * preferred value differs from already known bits or sub-key
                   * contains not yet known bits and
                   * confidence is the largest one */
		  if ((consensus == UINT64_C (0x3f)) && (confidence > max) &&
		      ((mask != UINT64_C (0x3f))
		       || (((kg ^ decision) & mask) != ZERO64)))
		    {
		      max = confidence; /* Remember max confidence */
		      br = r;           /* Remember round key */
		      bs = s;           /* Remember sub-key */
		      bg = (int) (decision); /* Remember sub-key value */
		      stop = 0; /* Do not stop yet, the system is still moving */
		    }
		} /* For 8 sub-keys */
	    } /* For first and last round keys */
	  if (!stop) /* If something new happened */
	    {
              /* Add new knowledge to key knowledge manager */
	      des_km_set_sk (km, br * 15 + 1, bs + 1, 0, UINT64_C (0x3f),
			     (uint64_t) (bg));
              /* If target number of bits are known */
	      if (des_km_known (km) >= p.nbits)
		{
		  stop = 1; /* Stop anyway */
		}
	    }
	} /* While not stop */
      /* If target number of bits not reached (that is, the system froze before that) */
      if (des_km_known (km) < p.nbits)
	{
	  cnt = 0; /* We lose on this number of traces */
	}
      else /* We have at least the target number of bits */
	{
          /* Let's remove the extra bits with less confidence */
	  while (des_km_known (km) > p.nbits)
	    {
	      min = -1.0;
	      guessed_c0d0 = des_km_get_c0d0 (km, &mask);
	      for (j = 55; j >= 0; j--)
		{
		  confidence = cop_confidence (cop_ctx, j);
		  if ((mask & ONE64) && ((confidence < min) || (min == -1.0)))
		    {
		      min= confidence;
		      idx = j + 1;
		    }
		  mask >>= 1;
		}
	      des_km_clear_bit (km, idx); /* Discard less confident bit */
	    }
	  /* Extract p.bits bits of secret key */
	  guessed_c0d0 = des_km_get_c0d0 (km, &mask);
	  /* If correct guess return true, else return false */
	  if ((mask & c0d0) == (mask & guessed_c0d0))
	    {
	      cnt += 1; /* Increment success counter */
	    }
	  else
	    {
	      cnt = 0; /* Reset success counter */
	    }
	}
      des_km_free (km); /* Free key knowledge manager */
      fprintf (stderr, TRACESPOS "%5d", i);
      fprintf (stderr, HITSPOS "%3d", cnt);
      if (cnt == 100) /* If 100 consecutive successes */
	{
	  break; /* We did it with the current number of traces */
	}
    }
  tr_pcc_free (tr_pcc_ctx[2]); /* Free PCC context for L0->L1 */
  flog (fname, "%6d %4d ", i, cnt); /* Log file */
  if (cnt != 100) /* Failure */
    {
      return -1;
    }
  else /* Success */
    {
      return i;
    }
}

void
free_data_structures (void)
{
  int s, g, i, r;

  /* Free allocated memory */
  free (l2l);
  for (r = 0; r < 2; r++) /* For first and last round keys */
    {
      for (s = 0; s < 8; s++)	/* For 8 sub-keys */
	{
	  for (g = 0; g < 64; g++)	/* For 64 guesses */
	    {
	      free (r2r[r][s][g]);
	    }			/* For 64 guesses */
	}			/* For 8 sub-keys */
    } /* For first and last round keys */
  free (traces_list);
  for (i = 0; i < 56; i++)	/* For bits of 56 bits secret key */
    {
      for (r = 0; r < 2; r++)	/* For first (r=0) and last (r=1) round keys */
	{
	  free (sf2s[i][r].val);
	}			/* For first (r=0) and last (r=1) round keys */
    }				/* For bits of 56 bits secret key */
  cop_free (cop_ctx);
  tr_free_context (tr_ctx);
  free (fname);
}
