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

/** \file utils.h
 *  Some utility functions.
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

/** The 64 bits 0 constant */
#define ZERO64 UINT64_C(0)
/** The 64 bits 1 constant */
#define ONE64 UINT64_C(1)

/** Returns the Hamming weight of a 64 bits word.
 * Note: the input's width can be anything between 0 and 64, as long as the
 * unused bits are all zeroes. \return The Hamming weight of the input as a 64
 * bits ::uint64_t. */
int hamming_weight (uint64_t val
		 /**< The 64 bits input. */
  );

/** Returns the Hamming distance between two 64 bits words.
 * Note: the width of the inputs can be anything between 0 and 64, as long as
 * they are the same, aligned and that the unused bits are all zeroes. \return
 * The Hamming distance between the two inputs as a 64 bits ::uint64_t. */
int hamming_distance (uint64_t val1 /**< The first 64 bits input. */ ,
		      uint64_t val2
		  /**< The second 64 bits input. */
  );

/** Prints an error message and exit with -1 status.
 * Takes a variable number of arguments, as printf. */
void error (
    /** Name of calling function (can be set automatically as __func__). */
	     const char *fnct,
	     char *frm /**< A printf-like formatting string. */ ,
	     ...
	 /**< Variable number of arguments. */
  );

/** Prints a warning message.
 * Takes a variable number of arguments, as printf. */
void warning (
    /** Name of calling function (can be set automatically as __func__). */
	       const char *fnct,
	       char *frm /**< A printf-like formatting string. */ ,
	       ...
	 /**< Variable number of arguments. */
  );

/** Open the log file fname::, prints a message and close log file.
 * Takes a variable number of arguments, as printf. */
void flog (
    /** Name of log file. */
	    const char *fname,
	    char *frm /**< A printf-like formatting string. */ ,
	    ...
	 /**< Variable number of arguments. */
  );

/** Wrapper around the regular malloc memory allocator. Errors are catched, no
 * need to check result. \return pointer to allocated memory area. */
void *xmalloc (size_t size /**< Size of an element. */ ,
	       const char *fname
		      /**< Name of calling function. */
  );

/** Wrapper around the regular calloc memory allocator. Errors are catched, no
 * need to check result. \return pointer to allocated memory area. */
void *xcalloc (size_t nmemb /**< Number of elements to allocate. */ ,
	       size_t size /**< Size of an element. */ ,
	       const char *fname
		      /**< Name of calling function. */
  );

/** Wrapper around the regular realloc memory allocator. Errors are catched, no
 * need to check result. \return pointer to allocated memory area. */
void *xrealloc (void *ptr /**< Source pointer. */ ,
		size_t size /**< Size of an element. */ ,
		const char *fname
		      /**< Name of calling function. */
  );

#endif /** not UTILS_H */
