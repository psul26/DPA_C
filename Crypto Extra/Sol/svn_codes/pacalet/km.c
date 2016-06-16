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

#include <stdlib.h>

#include <utils.h>
#include <des.h>
#include <km.h>

des_key_manager
des_km_init (void)
{
  des_key_manager km;

  km = xcalloc (1, sizeof (struct des_key_manager_s), __func__);
  km->mask = UINT64_C (0x0);
  km->key = UINT64_C (0x0);
  return km;
}

int
des_km_known (des_key_manager km)
{
  return hamming_weight (km->mask);
}

int
des_km_set_bit (des_key_manager km, int bit, int force, uint64_t val)
{
  uint64_t mask;

  if ((bit < 1) || (bit > 56))
    {
      error (__func__, "illegal bit number: %d", bit);
    }
  if (val >> 1)
    {
      error (__func__, "illegal bit value: %016" PRIx64, val);
    }
  mask = ONE64 << (56 - bit);
  val <<= (56 - bit);
  return des_km_set_c0d0 (km, force, mask, val);
}

int
des_km_set_sk (des_key_manager km, int rk, int sk, int force, uint64_t mask,
	       uint64_t val)
{
  if ((sk < 1) || (sk > 8))
    {
      error (__func__, "illegal subkey number: %d", sk);
    }
  if (mask >> 6)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  if (val >> 6)
    {
      error (__func__, "illegal subkey value: %016" PRIx64, val);
    }
  mask <<= 6 * (8 - sk);
  val <<= 6 * (8 - sk);
  return des_km_set_rk (km, rk, force, mask, val);
}

int
des_km_set_rk (des_key_manager km, int rk, int force, uint64_t mask,
	       uint64_t val)
{
  int i;

  if ((rk < 1) || (rk > 16))
    {
      error (__func__, "illegal round key number: %d", rk);
    }
  if (mask >> 48)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  if (val >> 48)
    {
      error (__func__, "illegal subkey value: %016" PRIx64, val);
    }
  mask = des_n_pc2 (mask);
  val = des_n_pc2 (val);
  for (i = rk; i < 16; i++)
    {
      mask = des_ls (mask);
      val = des_ls (val);
      if (left_shifts[i] == 1)
	{
	  mask = des_ls (mask);
	  val = des_ls (val);
	}
    }
  return des_km_set_c0d0 (km, force, mask, val);
}

uint64_t
des_km_set_c0d0 (des_key_manager km, int force, uint64_t mask, uint64_t val)
{
  uint64_t conflict;

  if (mask >> 56)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  if (val >> 56)
    {
      error (__func__, "illegal subkey value: %016" PRIx64, val);
    }
  val &= mask;
  conflict = (km->key ^ val) & km->mask & mask;
  if (conflict && (!force))
    {
      return UINT64_C (0x0);
    }
  km->mask |= mask;
  km->key = (km->key & (~mask)) | val;
  return !conflict;
}

int
des_km_set_key (des_key_manager km, int force, uint64_t mask, uint64_t val)
{
  mask = des_pc1 (mask);
  val = des_pc1 (val);
  return des_km_set_c0d0 (km, force, mask, val);
}

void
des_km_clear_bit (des_key_manager km, int bit)
{
  uint64_t mask;

  if ((bit < 1) || (bit > 56))
    {
      error (__func__, "illegal bit number: %d", bit);
    }
  mask = ONE64 << (56 - bit);
  des_km_clear_c0d0 (km, mask);
}

void
des_km_clear_sk (des_key_manager km, int rk, int sk, uint64_t mask)
{
  if ((sk < 1) || (sk > 8))
    {
      error (__func__, "illegal subkey number: %d", sk);
    }
  if (mask >> 6)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  mask <<= 6 * (8 - sk);
  des_km_clear_rk (km, rk, mask);
}

void
des_km_clear_rk (des_key_manager km, int rk, uint64_t mask)
{
  int i;

  if ((rk < 1) || (rk > 16))
    {
      error (__func__, "illegal round key number: %d", rk);
    }
  if (mask >> 48)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  mask = des_n_pc2 (mask);
  for (i = rk; i < 16; i++)
    {
      mask = des_ls (mask);
      if (left_shifts[i] == 1)
	{
	  mask = des_ls (mask);
	}
    }
  des_km_clear_c0d0 (km, mask);
}

void
des_km_clear_c0d0 (des_key_manager km, uint64_t mask)
{
  if (mask >> 56)
    {
      error (__func__, "illegal mask value: %016" PRIx64, mask);
    }
  km->mask &= (~mask);
  km->key = (km->key & (~mask));
}

void
des_km_clear_key (des_key_manager km, uint64_t mask)
{
  mask = des_pc1 (mask);
  des_km_clear_c0d0 (km, mask);
}

uint64_t
des_km_get_sk (des_key_manager km, int rk, int sk, uint64_t * mask)
{
  uint64_t val;

  if ((sk < 1) || (sk > 8))
    {
      error (__func__, "illegal subkey number: %d", sk);
    }
  val = des_km_get_rk (km, rk, mask);
  *mask = ((*mask) >> (6 * (8 - sk))) & UINT64_C (0x3f);
  val = (val >> (6 * (8 - sk))) & UINT64_C (0x3f);
  return val;
}

uint64_t
des_km_get_rk (des_key_manager km, int rk, uint64_t * mask)
{
  uint64_t val;
  int i;

  if ((rk < 1) || (rk > 16))
    {
      error (__func__, "illegal round key number: %d", rk);
    }
  val = des_km_get_c0d0 (km, mask);
  for (i = 0; i < rk; i++)
    {
      *mask = des_ls (*mask);
      val = des_ls (val);
      if (left_shifts[i] == 1)
	{
	  *mask = des_ls (*mask);
	  val = des_ls (val);
	}
    }
  *mask = des_pc2 (*mask);
  val = des_pc2 (val);
  return val;
}

uint64_t
des_km_get_c0d0 (des_key_manager km, uint64_t * mask)
{
  *mask = km->mask;
  return km->key;
}

uint64_t
des_km_get_key (des_key_manager km, uint64_t * mask)
{
  uint64_t val, tmp, mask2;
  int i;

  val = des_km_get_c0d0 (km, mask);
  *mask = des_n_pc1 (*mask);
  val = des_n_pc1 (val);
  tmp = *mask >> 1;
  mask2 = UINT64_C (0x1);
  for (i = 0; i < 8; i++)
    {
      if ((tmp & UINT64_C (0x7f)) != UINT64_C (0x7f))
	{
	  *mask &= (~mask2);
	}
      mask2 <<= 8;
      tmp >>= 8;
    }
  return val;
}

void
des_km_init_for_unknown (des_key_manager km)
{
  km->key &= km->mask;
}

uint64_t
des_km_for_unknown (des_key_manager km)
{
  uint64_t key;

  key = km->key & (~(km->mask));
  key += km->mask + UINT64_C (0x1);
  key &= ~(km->mask);
  key &= UINT64_C (0xffffffffffffff);
  km->key = (km->key & km->mask) | key;
  return key;
}

void
des_km_free (des_key_manager km)
{
  free (km);
}
