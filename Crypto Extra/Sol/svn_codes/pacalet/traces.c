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
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <utils.h>
#include <traces.h>

tr_context
tr_init_context (void)
{
  tr_context ctx;
  ctx = xcalloc (1, sizeof (struct tr_context_s), __func__);
  ctx->n = 0;
  ctx->l = 0;
  ctx->k = UINT64_C (0);
  ctx->p = NULL;
  ctx->c = NULL;
  ctx->t = NULL;
  return ctx;
}

void
tr_load_context (tr_context ctx, char *filename, int max)
{
  char *dummy;
  int magic_number_length, i, j, n, l;
  FILE *fp;
  uint64_t k;
  float *t;

  if (max < 0)
    {
      error (__func__, "invalid maximum number of traces: %d", max);
    }
  fp = fopen (filename, "rb");
  if (fp == NULL)
    {
      error (__func__, "cannot open file %s", filename);
    }
  magic_number_length = strlen (HWSECMAGICNUMBER);
  dummy = xcalloc (magic_number_length, sizeof (char), __func__);
  if ((fread (dummy, sizeof (char), magic_number_length, fp) !=
       magic_number_length) ||
      (strncmp (dummy, HWSECMAGICNUMBER, magic_number_length) != 0))
    {
      error (__func__,
	     "wrong magic number; is this a real HWSec trace file?");
    }
  free (dummy);
  if (fread (&n, sizeof (uint32_t), 1, fp) != 1)
    {
      error (__func__,
	     "cannot read number of traces; is this a real HWSec trace file?");
    }
  if (max == 0)
    {
      max = n;
    }
  else if (n >= max)
    {
      n = max;
    }
  else
    {
      error (__func__, "not enough traces in trace file (%d < %d)", n, max);
    }
  if (fread (&l, sizeof (uint32_t), 1, fp) != 1)
    {
      error (__func__,
	     "cannot read traces length; is this a real HWSec trace file?");
    }
  if (fread (&k, sizeof (uint64_t), 1, fp) != 1)
    {
      error (__func__,
	     "cannot read secret key; is this a real HWSec trace file?");
    }
  if (ctx->n == 0)
    {
      ctx->l = l;
      ctx->k = k;
      ctx->p = xcalloc (n, sizeof (uint64_t), __func__);
      ctx->c = xcalloc (n, sizeof (uint64_t), __func__);
      ctx->t = xcalloc (n, sizeof (double *), __func__);
    }
  else
    {
      if (l != ctx->l)
	{
	  error (__func__,
		 "traces length differs from length of previous traces (%d != %d)",
		 l, ctx->l);
	}
      if (k != ctx->k)
	{
	  error (__func__,
		 "secret key differs from previous secret key (%016" PRIx64
		 " != %016" PRIx64 ")", k, ctx->k);
	}
      n += ctx->n;
      ctx->p = xrealloc (ctx->p, n * sizeof (uint64_t), __func__);
      ctx->c = xrealloc (ctx->c, n * sizeof (uint64_t), __func__);
      ctx->t = xrealloc (ctx->t, n * sizeof (double *), __func__);
    }
  t = xcalloc (l, sizeof (float *), __func__);
  for (i = ctx->n; i < n; i++)
    {
      if (fread (&(ctx->p[i]), sizeof (uint64_t), 1, fp) != 1)
	{
	  error (__func__,
		 "cannot read plaintext #%d; is this a real HWSec trace file?",
		 i);
	}
      if (fread (&(ctx->c[i]), sizeof (uint64_t), 1, fp) != 1)
	{
	  error (__func__,
		 "cannot read ciphertext #%d; is this a real HWSec trace file?",
		 i);
	}
      ctx->t[i] = xcalloc (l, sizeof (double), __func__);
      if (fread (t, sizeof (float), l, fp) != l)
	{
	  error (__func__,
		 "cannot read trace #%d; is this a real HWSec trace file?",
		 i);
	}
      for (j = 0; j < l; j++)
	{
	  ctx->t[i][j] = (double) (t[j]);
	}
    }
  free (t);
  ctx->n = n;
  fclose (fp);
}

tr_context
tr_copy_context (tr_context ctx, int n)
{
  tr_context res;
  int i;

  if ((n < 0) || (n > ctx->n))
    {
      error (__func__,
	     "invalid number of traces: %d (context contains %d traces)", n,
	     ctx->n);
    }
  res = xcalloc (1, sizeof (struct tr_context_s), __func__);
  res->n = n;
  res->l = ctx->l;
  res->k = ctx->k;
  res->p = xcalloc (n, sizeof (uint64_t), __func__);
  res->c = xcalloc (n, sizeof (uint64_t), __func__);
  res->t = xcalloc (n, sizeof (double *), __func__);
  for (i = 0; i < n; i++)
    {
      res->p[i] = ctx->p[i];
      res->c[i] = ctx->c[i];
      res->t[i] = xcalloc (res->l, sizeof (double), __func__);
      memcpy (res->t[i], ctx->t[i], res->l * sizeof (double));
    }
  return res;
}

void
tr_free_context (tr_context ctx)
{
  int i;

  free (ctx->p);
  free (ctx->c);
  for (i = 0; i < ctx->n; i++)
    {
      free (ctx->t[i]);
    }
  free (ctx->t);
  free (ctx);
}

void
tr_trim_context (tr_context ctx, int from, int to)
{
  int i, newl;
  double *t;

  newl = to - from + 1;
  if (from < 0 || newl < 1 || to >= ctx->l)
    {
      error (__func__,
	     "invalid parameters value: from=%d, to=%d (traces length=%d)",
	     from, to, ctx->l);
    }
  for (i = 0; i < ctx->n; i++)
    {
      t = xcalloc (newl, sizeof (double), __func__);
      memcpy (t, ctx->t[i] + from, newl * sizeof (double));
      free (ctx->t[i]);
      ctx->t[i] = t;
    }
  ctx->l = newl;
}

void
tr_puncture_context (tr_context ctx, int from, int to)
{
  int i;
  double *t;
  int newl;

  if (from < 0 || to < from || to > ctx->l)
    {
      error (__func__,
	     "invalid parameters value: from=%d, to=%d (traces length=%d)",
	     from, to, ctx->l);
    }
  newl = ctx->l - (to - from + 1);
  for (i = 0; i < ctx->n; i++)
    {
      t = xcalloc (newl, sizeof (double), __func__);
      memcpy (t, ctx->t[i], from * sizeof (double));
      memcpy (t + from, ctx->t[i] + to + 1, (newl - from) * sizeof (double));
      free (ctx->t[i]);
      ctx->t[i] = t;
    }
  ctx->l = newl;
}

void
tr_fold_context (tr_context ctx, int length)
{
  int i, j, k;
  double *t;

  if (length < 0 || length > ctx->l)
    {
      error (__func__,
	     "invalid parameter value: length=%d (traces length=%d)",
	     length, ctx->l);
    }
  for (i = 0; i < ctx->n; i++)
    {
      t = xcalloc (length, sizeof (double), __func__);
      memcpy (t, &(ctx->t[i][0]), length * sizeof (double));
      for (j = 1; j < ctx->l / length; j++)
	{
	  for (k = 0; k < length; k++)
	    {
	      t[k] += ctx->t[i][j * length + k];
	    }
	}
      free (ctx->t[i]);
      ctx->t[i] = t;
    }
  ctx->l = length;
}

void
tr_select_context (tr_context ctx, int from, int to)
{
  int i, newn;
  double **t;
  uint64_t *p, *c;

  newn = to - from + 1;
  if (from < 0 || newn > ctx->n || to >= ctx->n)
    {
      error (__func__,
	     "invalid parameters value: from=%d, to=%d (number of traces=%d)",
	     from, to, ctx->n);
    }
  t = xcalloc (newn, sizeof (double *), __func__);
  p = xcalloc (newn, sizeof (uint64_t), __func__);
  c = xcalloc (newn, sizeof (uint64_t), __func__);
  for (i = 0; i < from; i++)
    {
      free (ctx->t[i]);
    }
  for (i = 0; i < newn; i++)
    {
      t[i] = ctx->t[i + from];
      p[i] = ctx->p[i + from];
      c[i] = ctx->c[i + from];
    }
  for (i = from + newn; i < ctx->n; i++)
    {
      free (ctx->t[i]);
    }
  free (ctx->p);
  ctx->p = p;
  free (ctx->c);
  ctx->c = c;
  free (ctx->t);
  ctx->t = t;
  ctx->n = newn;
}

void
tr_shrink_context (tr_context ctx, int chunk_size)
{
  int i, j, k, l;
  double *t, *ot;

  if (chunk_size < 1 || chunk_size > ctx->l)
    {
      error (__func__,
	     "invalid parameters value: chunk_size=%d (traces length=%d)",
	     chunk_size, ctx->l);
    }
  l = ctx->l / chunk_size;
  for (i = 0; i < ctx->n; i++)
    {
      t = xcalloc (l, sizeof (double), __func__);
      ot = ctx->t[i];
      for (j = 0; j < l; j++)
	{
	  t[j] = 0.0;
	  for (k = 0; k < chunk_size; k++)
	    {
	      t[j] += *ot;
	      ot += 1;
	    }
	}
      free (ctx->t[i]);
      ctx->t[i] = t;
    }
  ctx->l = l;
}

void
tr_dump_context (tr_context ctx, char *filename, int from, int to)
{
  int magic_number_length, i, j;
  uint32_t n;
  FILE *fp;
  float *t;

  if ((from < 0) || (to < from) || (to >= ctx->n))
    {
      error (__func__,
	     "invalid traces selection: [%d..%d] (context contains %d traces)",
	     from, to, ctx->n);
    }
  fp = fopen (filename, "wb");
  if (fp == NULL)
    {
      error (__func__, "cannot open file %s for writing", filename);
    }
  magic_number_length = strlen (HWSECMAGICNUMBER);
  if (fwrite (HWSECMAGICNUMBER, sizeof (char), magic_number_length, fp) !=
      magic_number_length)
    {
      error (__func__, "write error");
    }
  n = to - from + 1;
  if (fwrite (&n, sizeof (uint32_t), 1, fp) != 1)
    {
      error (__func__, "write error");
    }
  if (fwrite (&(ctx->l), sizeof (uint32_t), 1, fp) != 1)
    {
      error (__func__, "write error");
    }
  if (fwrite (&(ctx->k), sizeof (uint64_t), 1, fp) != 1)
    {
      error (__func__, "write error");
    }
  t = xcalloc (ctx->l, sizeof (float *), __func__);
  for (i = from; i < to; i++)
    {
      if (fwrite (&(ctx->p[i]), sizeof (uint64_t), 1, fp) != 1)
	{
	  error (__func__, "write error");
	}
      if (fwrite (&(ctx->c[i]), sizeof (uint64_t), 1, fp) != 1)
	{
	  error (__func__, "write error");
	}
      for (j = 0; j < ctx->l; j++)
	{
	  t[j] = (float) (ctx->t[i][j]);
	}
      if (fwrite (t, sizeof (float), ctx->l, fp) != ctx->l)
	{
	  error (__func__, "write error");
	}
    }
  fclose (fp);
}

int
tr_number (tr_context ctx)
{
  return ctx->n;
}

int
tr_length (tr_context ctx)
{
  return ctx->l;
}

uint64_t
tr_key (tr_context ctx)
{
  return ctx->k;
}

uint64_t
tr_plaintext (tr_context ctx, int i)
{
  if (i < 0 || i > ctx->n)
    {
      error (__func__,
	     "no plaintext #%d in context (number of plaintexts=%d)", i,
	     ctx->n);
    }
  return ctx->p[i];
}

uint64_t
tr_ciphertext (tr_context ctx, int i)
{
  if (i < 0 || i > ctx->n)
    {
      error (__func__,
	     "no ciphertext #%d in context (number of ciphertexts=%d)", i,
	     ctx->n);
    }
  return ctx->c[i];
}

double *
tr_trace (tr_context ctx, int i)
{
  if (i < 0 || i > ctx->n)
    error (__func__, "no trace #%d in context (number of traces=%d)", i,
	   ctx->n);
  return ctx->t[i];
}

double *
tr_new_trace (int length)
{
  return xcalloc (length, sizeof (double), __func__);
}

void
tr_free_trace (double *t)
{
  free (t);
}

void
tr_init_trace (int length, double *t, double val)
{
  int i;

  for (i = 0; i < length; i++)
    {
      t[i] = val;
    }
}

void
tr_copy_trace (int length, double *dest, double *src)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src[i];
    }
}

void
tr_acc (int length, double *dest, double *src)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] += src[i];
    }
}

void
tr_add (int length, double *dest, double *src1, double *src2)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src1[i] + src2[i];
    }
}

void
tr_sub (int length, double *dest, double *src1, double *src2)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src1[i] - src2[i];
    }
}

void
tr_scalar_mul (int length, double *dest, double *src, double val)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src[i] * val;
    }
}

void
tr_scalar_div (int length, double *dest, double *src, double val)
{
  int i;

  if (val == 0.0)
    {
      error (__func__, "division by zero");
    }
  for (i = 0; i < length; i++)
    {
      dest[i] = src[i] / val;
    }
}

void
tr_mul (int length, double *dest, double *src1, double *src2)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src1[i] * src2[i];
    }
}

void
tr_div (int length, double *dest, double *src1, double *src2)
{
  int i;

  for (i = 0; i < length; i++)
    {
      if (src2[i] == 0.0)
	{
	  error (__func__, "division by zero");
	}
      dest[i] = src1[i] / src2[i];
    }
}

void
tr_sqr (int length, double *dest, double *src)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = src[i] * src[i];
    }
}

void
tr_sqrt (int length, double *dest, double *src)
{
  int i;

  for (i = 0; i < length; i++)
    {
      if (src[i] < 0.0)
	{
	  error (__func__, "negative value");
	}
      dest[i] = sqrt (src[i]);
    }
}

void
tr_abs (int length, double *dest, double *src)
{
  int i;

  for (i = 0; i < length; i++)
    {
      dest[i] = fabs (src[i]);
    }
}

double
tr_min (int length, double *t, int *idx)
{
  return tr_bounded_min (0, length - 1, t, idx);
}

double
tr_bounded_min (int from, int to, double *t, int *idx)
{
  int i;
  double min;

  min = t[from];
  *idx = from;
  for (i = from + 1; i <= to; i++)
    {
      if (t[i] < min)
	{
	  min = t[i];
	  *idx = i;
	}
    }
  return min;
}


double
tr_max (int length, double *t, int *idx)
{
  return tr_bounded_max (0, length - 1, t, idx);
}

double
tr_bounded_max (int from, int to, double *t, int *idx)
{
  int i;
  double max;

  max = t[from];
  *idx = from;
  for (i = from + 1; i <= to; i++)
    {
      if (t[i] > max)
	{
	  max = t[i];
	  *idx = i;
	}
    }
  return max;
}

void
tr_print (int length, double *t)
{
  int i;

  for (i = 0; i < length; i++)
    {
      printf ("%e\n", t[i]);
    }
}

void
tr_fprint (int length, FILE * fp, double *t)
{
  int i;

  for (i = 0; i < length; i++)
    {
      fprintf (fp, "%e\n", t[i]);
    }
}
