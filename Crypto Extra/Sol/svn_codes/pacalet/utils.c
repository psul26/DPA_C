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
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

int
hamming_weight (uint64_t val)
{
  int res;
  int i;

  res = 0;
  for (i = 0; i < 64; i++)
    {
      res += (int) (val & UINT64_C (1));
      val >>= 1;
    }
  return res;
}

int
hamming_distance (uint64_t val1, uint64_t val2)
{
  return hamming_weight (val1 ^ val2);
}

void
error (const char *fnct, char *frm, ...)
{
  va_list ap;

  va_start (ap, frm);
  fprintf (stderr, "%s error: ", fnct);
  vfprintf (stderr, frm, ap);
  fprintf (stderr, "\n");
  exit (-1);
}

void
warning (const char *fnct, char *frm, ...)
{
  va_list ap;

  va_start (ap, frm);
  fprintf (stderr, "%s error: ", fnct);
  vfprintf (stderr, frm, ap);
  fprintf (stderr, "\n");
}

void
flog (const char *fname, char *frm, ...)
{
  FILE *fp;
  va_list ap;

  fp = fopen (fname, "a");
  if (fp == NULL)
    {
      error (__func__, "cannot open log file %s", fname);
    }
  va_start (ap, frm);
  vfprintf (fp, frm, ap);
  fclose (fp);
}

void *
xmalloc (size_t size, const char *fname)
{
  void *ptr;

  if (fname == NULL)
    {
      fname = __func__;
    }
  ptr = malloc (size);
  if (ptr == NULL)
    {
      error (fname, "memory allocation");
    }
  return ptr;
}

void *
xcalloc (size_t nmemb, size_t size, const char *fname)
{
  void *ptr;

  if (fname == NULL)
    {
      fname = __func__;
    }
  ptr = calloc (nmemb, size);
  if (ptr == NULL)
    {
      error (fname, "memory allocation");
    }
  return ptr;
}

void *
xrealloc (void *ptr, size_t size, const char *fname)
{
  if (fname == NULL)
    {
      fname = __func__;
    }
  ptr = realloc (ptr, size);
  if (ptr == NULL)
    {
      error (fname, "memory allocation");
    }
  return ptr;
}
