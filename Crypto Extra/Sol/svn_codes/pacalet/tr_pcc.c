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
#include <tr_pcc.h>

tr_pcc_context
tr_pcc_init (int l, int ny)
{
  int i;
  tr_pcc_context ctx;

  if (l < 1)
    {
      error (__func__, "invalid vector length: %d", l);
    }
  if (ny < 1)
    {
      error (__func__, "invalid number of Y random variables: %d", ny);
    }
  ctx = xcalloc (1, sizeof (struct tr_pcc_context_s), __func__);
  ctx->ny = ny;
  ctx->nr = 0;
  ctx->l = l;
  ctx->rx = tr_new_trace (l);
  ctx->x = tr_new_trace (l);
  tr_init_trace (l, ctx->x, 0.0);
  ctx->x2 = tr_new_trace (l);
  tr_init_trace (l, ctx->x2, 0.0);
  ctx->y = tr_new_trace (ny);
  tr_init_trace (ny, ctx->y, 0.0);
  ctx->y2 = tr_new_trace (ny);
  tr_init_trace (ny, ctx->y2, 0.0);
  ctx->xy = xcalloc (ny, sizeof (double *), __func__);
  ctx->pcc = xcalloc (ny, sizeof (double *), __func__);
  for (i = 0; i < ny; i++)
    {
      ctx->xy[i] = tr_new_trace (l);
      tr_init_trace (l, ctx->xy[i], 0.0);
      ctx->pcc[i] = tr_new_trace (l);
      tr_init_trace (l, ctx->pcc[i], 0.0);
    }
  ctx->state = 0;
  ctx->cnt = ny;
  ctx->flags = xcalloc (ny, sizeof (char), __func__);
  for (i = 0; i < ny; i++)
    {
      ctx->flags[i] = 0;
    }
  ctx->tmp1 = tr_new_trace (l);
  ctx->tmp2 = tr_new_trace (l);
  ctx->tmp3 = tr_new_trace (l);
  return ctx;
}

int
tr_pcc_number (tr_pcc_context ctx)
{
  return ctx->ny;
}

int
tr_pcc_length (tr_pcc_context ctx)
{
  return ctx->l;
}

void
tr_pcc_insert_x (tr_pcc_context ctx, double *x)
{
  if (ctx->cnt != ctx->ny)
    {
      error (__func__, "missing %d Y realizations", ctx->ny - ctx->cnt);
    }
  ctx->cnt = 0;
  ctx->state = 1 - ctx->state;
  tr_copy_trace (ctx->l, ctx->rx, x);
  tr_acc (ctx->l, ctx->x, x);
  tr_sqr (ctx->l, ctx->tmp1, x);
  tr_acc (ctx->l, ctx->x2, ctx->tmp1);
  ctx->nr += 1;
}

void
tr_pcc_insert_y (tr_pcc_context ctx, int ny, double y)
{
  if (ny < 0 || ny >= ctx->ny)
    {
      error (__func__, "invalid Y index: %d", ny);
    }
  if (ctx->flags[ny] == ctx->state)
    {
      error (__func__, "Y realization #%d inserted twice", ny);
    }
  ctx->y[ny] += y;
  ctx->y2[ny] += y * y;
  tr_scalar_mul (ctx->l, ctx->tmp1, ctx->rx, y);
  tr_acc (ctx->l, ctx->xy[ny], ctx->tmp1);
  ctx->cnt += 1;
  ctx->flags[ny] = ctx->state;
}

void
tr_pcc_consolidate (tr_pcc_context ctx, int mode)
{
  double n, nb, vary;
  int i;

  if (ctx->cnt != ctx->ny)
    {
      error (__func__, "missing %d Y realizations", ctx->ny - ctx->cnt);
    }
  if (ctx->nr < 2)
    {
      error (__func__, "not enough realizations (%d, min 2)", ctx->nr);
    }
  n = (double) (ctx->nr);
  tr_scalar_mul (ctx->l, ctx->tmp1, ctx->x2, n);	/* TMP1 = N.X2 */
  tr_sqr (ctx->l, ctx->tmp2, ctx->x);	/* TMP2 = X^2 */
  tr_sub (ctx->l, ctx->tmp1, ctx->tmp1, ctx->tmp2);	/* TMP1 = N.X2-X^2 */
  if ((mode == UNBIASED) || (mode == SQUAREUNBIASED))
    {
      nb = n / (n - 1.0);
      nb = nb * nb;
      tr_scalar_mul (ctx->l, ctx->tmp1, ctx->tmp1, nb);
      /* TMP1 = (N/(N-1))^2.(N.X2-X^2) */
    }
  if ((mode != SQUAREBIASED) && (mode != SQUAREUNBIASED))
    {
      tr_sqrt (ctx->l, ctx->tmp1, ctx->tmp1);
    }
  for (i = 0; i < ctx->ny; i++)
    {
      tr_scalar_mul (ctx->l, ctx->tmp2, ctx->xy[i], n);	/* TMP2 = N.XY */
      tr_scalar_mul (ctx->l, ctx->tmp3, ctx->x, ctx->y[i]);	/* TMP3 = X.Y */
      tr_sub (ctx->l, ctx->tmp2, ctx->tmp2, ctx->tmp3);	/* TMP2 = N.XY-X.Y */
      vary = n * ctx->y2[i] - ctx->y[i] * ctx->y[i];
      if ((mode == SQUAREBIASED) || (mode == SQUAREUNBIASED))
	{
	  tr_sqr (ctx->l, ctx->tmp2, ctx->tmp2);
	}
      else
	{
	  vary = sqrt (vary);
	}
      tr_div (ctx->l, ctx->tmp2, ctx->tmp2, ctx->tmp1);
      tr_scalar_div (ctx->l, ctx->pcc[i], ctx->tmp2, vary);
    }
}

double *
tr_pcc_get_pcc (tr_pcc_context ctx, int ny)
{
  if (ny < 0 || ny >= ctx->ny)
    {
      error (__func__, "invalid Y index: %d", ny);
    }
  return ctx->pcc[ny];
}

void
tr_pcc_free (tr_pcc_context ctx)
{
  int i;

  tr_free_trace (ctx->rx);
  tr_free_trace (ctx->x);
  tr_free_trace (ctx->x2);
  tr_free_trace (ctx->y);
  tr_free_trace (ctx->y2);
  for (i = 0; i < ctx->ny; i++)
    {
      tr_free_trace (ctx->xy[i]);
      tr_free_trace (ctx->pcc[i]);
    }
  tr_free_trace (ctx->tmp1);
  tr_free_trace (ctx->tmp2);
  tr_free_trace (ctx->tmp3);
  free (ctx->flags);
  free (ctx->xy);
  free (ctx->pcc);
  free (ctx);
}
