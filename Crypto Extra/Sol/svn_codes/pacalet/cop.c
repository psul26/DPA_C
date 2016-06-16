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
#include <stdint.h>
#include <inttypes.h>

#include <utils.h>
#include <cop.h>

cop_context
cop_init (int n)
{
  cop_context ctx;
  int i;

  if ((n < 1) || (n > 64))
    {
      error (__func__,
	     "invalid number of agents: %d (should be between 1 and 64)", n);
    }
  ctx = xcalloc (1, sizeof (struct cop_context_s), __func__);
  ctx->na = n;
  ctx->a = xcalloc (n, sizeof (struct agent_s), __func__);
  for (i = 0; i < n; i++)
    {
      ctx->a[i].nbs = NULL;
      ctx->a[i].nn = 0;
      ctx->a[i].w = 0.0;
      ctx->a[i].e = NULL;
    }
  cop_init_agents (ctx);
  return ctx;
}

int
cop_decision (cop_context ctx, int idx)
{
  if ((idx < 0) || (idx > ctx->na))
    {
      error (__func__, "invalid agent index: %d", idx);
    }
  return ctx->a[idx].decision;
}

double
cop_confidence (cop_context ctx, int idx)
{
  agent a;
  int d, new;

  if ((idx < 0) || (idx > ctx->na))
    {
      error (__func__, "invalid agent index: %d", idx);
    }
  a = ctx->a + idx;
  d = a->decision;
  new = ctx->new;
  if (a->phi[new][1 - d] == a->phi[new][d])
    {
      return 0.0;
    }
  else
    {
      return a->phi[new][d] / a->phi[new][1 - d];
    }
}


int
cop_consensus (cop_context ctx, int idx)
{
  agent a;

  if ((idx < 0) || (idx > ctx->na))
    {
      error (__func__, "invalid agent index: %d", idx);
    }
  a = ctx->a + idx;
  return a->consensus;
}

void
cop_init_agents (cop_context ctx)
{
  int i, j, k;

  for (i = 0; i < ctx->na; i++)
    {
      for (j = 0; j < 2; j++)
	{
	  for (k = 0; k < 2; k++)
	    {
	      ctx->a[i].phi[j][k] = 0.0;
	    }
	}
      ctx->a[i].decision = -1;
      ctx->a[i].consensus = 0;
    }
  ctx->new = 0;
  ctx->cost = 0.0;
  ctx->consensus = 0;
  ctx->stability = 0;
}

int
cop_iterate (cop_context ctx, double l)
{
  int i, *nbs, val, my_val, min_set[2], new, old;
  int j, nb, decision;
  uint64_t valuation, v, min_val[2];
  double tmp, new_cost, old_cost, min[2];
  agent ca, na;

  if ((l < 0.0) || (l > 1.0))
    {
      error (__func__,
	     "invalid lambda factor: %f (should be between 0.0 and 1.0)", l);
    }
  new = ctx->new;
  old = 1 - new;
  new_cost = 0.0;
  old_cost = ctx->cost;
  ctx->stability = 0;
  for (i = 0; i < ctx->na; i++)
    {
      ca = ctx->a + i;
      ca->consensus = 0;
    }
  for (i = 0; i < ctx->na; i++)
    {
      ca = ctx->a + i;
      nbs = ca->nbs;
      min_set[0] = 0;
      min_set[1] = 0;
      my_val = 0;
      for (valuation = ZERO64; valuation < (ONE64 << ca->nn); valuation++)
	{
	  tmp = 0.0;
	  v = valuation;
	  for (j = ca->nn - 1; j >= 0; j--)
	    {
	      nb = nbs[j];
	      val = (int) (v & ONE64);
	      if (nb == i)
		{
		  my_val = val;
		}
	      else
		{
		  na = ctx->a + nb;
		  tmp += na->phi[old][val] * na->w;
		}
	      v >>= 1;
	    }
	  tmp *= l;
	  tmp += ca->e[valuation] * (1.0 - l);
	  if ((min_set[my_val] == 0) || (tmp < min[my_val]))
	    {
	      min[my_val] = tmp;
	      min_set[my_val] = 1;
	      min_val[my_val] = valuation;
	    }
	}
      ca->phi[new][0] = min[0];
      ca->phi[new][1] = min[1];
      decision = ca->phi[new][0] > ca->phi[new][1];
      if (decision == ca->decision)
	{
	  ctx->stability += 1;
	}
      ca->decision = decision;
      v = min_val[decision];
      for (j = ca->nn - 1; j >= 0; j--)
	{
	  nb = nbs[j];
	  val = (int) (v & ONE64);
	  na = ctx->a + nb;
	  na->consensus += val;
	  v >>= 1;
	}
    }
  ctx->new = old;
  ctx->consensus = 0;
  for (i = 0; i < ctx->na; i++)
    {
      ca = ctx->a + i;
      new_cost += ca->phi[new][ca->decision];
      if ((ca->consensus == 0) || (ca->consensus == ca->nn))
	{
	  ca->consensus = 1;
	  ctx->consensus += 1;
	}
      else
	{
	  ca->consensus = 0;
	}
    }
  ctx->cost = new_cost;
/*  return new_cost > old_cost; */
  return ctx->stability != ctx->na;
}

uint64_t
cop_vote (cop_context ctx)
{
  uint64_t vote;
  int i;

  vote = ZERO64;
  for (i = 0; i < 56; i++)
    {
      vote <<= 1;
      if (ctx->a[i].decision)
	{
	  vote |= ONE64;
	}
    }
  return vote;
}

void
cop_free (cop_context ctx)
{
  int i;

  for (i = 0; i < ctx->na; i++)
    {
      free (ctx->a[i].nbs);
      free (ctx->a[i].e);
    }
  free (ctx->a);
  free (ctx);
}
