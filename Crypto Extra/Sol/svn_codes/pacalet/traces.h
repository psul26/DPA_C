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

/** \file traces.h
 *  The \b traces library, a software library dedicated to processing of power
 *  traces in relation with the Data Encryption Standard (DES).
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-29
 *
 * Traces are one dimension arrays of doubleing point numbers. They are stored in
 * a binary file along with some parameters and their corresponding 64 bits (8
 * bytes) ciphertexts. The format of the trace files is the following:
 * <ol>
 * <li>"HWSec" (a 5 bytes magic number)</li>
 * <li>N, the number of traces in the file (a 4 bytes unsigned integer)</li>
 * <li>L, the number of points per trace (a 4 bytes unsigned integer)</li>
 * <li>K, the 64 bits secret key (a 8 bytes unsigned integer)</li>
 * <li>N * (8 + 8 + L * 4) bytes corresponding to the N traces. The 8 bytes of
 * the plaintext come first, then the 8 bytes of the cycphertext and the L
 * doubleing point numbers (L * 4 bytes) of the trace.</li>
 * </ol>
 * A trace file containing 50 traces, 100 points each will thus be 20813 bytes
 * long: 5 + 4 + 4 + 50 * (8 + 8 + 100 * 4).
 *
 * Reading trace files is done first by a call to tr_init_context() which
 * initializes and returns a tr_context, and then by successive calls to
 * tr_load_context(): \code
 * #include <traces.h>
 * ...
 * tr_context ctx;
 * ctx = tr_init_context();
 * tr_load_context(ctx, "MyTraceFile.hws", 1000);
 * \endcode
 * where "MyTraceFile.hws" is a regular trace file in HWSec format and 1000 the
 * maximum number of traces to read from it. Once initialized and loaded, the
 * context contains:
 * <ol>
 * <li>the number of traces in context</li>
 * <li>the number of points per trace</li>
 * <li>the secret key</li>
 * <li>the plaintexts</li>
 * <li>the ciphertexts</li>
 * <li>the power traces</li>
 * </ol>
 * 
 * At the end of the trace processing the context can be closed (freed) by a
 * call to tr_free_context(): \code
 * tr_free_context(ctx);
 * \endcode
 *
 * \attention
 * <ol>
 * <li>Most functions of the \b traces library check their input parameters and
 * issue warnings or errors when they carry illegal values. Warnings are
 * printed on the standard error output. Errors are also printed on the
 * standard error output and the program exits with a -1 exit status.</li>
 * <li>The \b traces library uses a single data type to represent all the data of
 * the DES standard: ::uint64_t. It is a 64 bits unsigned integer.</li>
 * <li>Data are always right aligned: when the data width is less than 64 bits,
 * the meaningful bits are always the rightmost bits of the ::uint64_t.</li>
 * </ol>
 */

#ifndef TRACES_H
#define TRACES_H

/** Magic number identifying trace files in HWSec format. */
#define HWSECMAGICNUMBER "HWSec"

/** The data structure used to manage a set of traces */
struct tr_context_s
{
  int n;			/**< Number of traces in the context */
  int l;			/**< Number of points per trace */
  uint64_t k;			/**< Secret key */
  uint64_t *p;			/**< Plaintexts */
  uint64_t *c;			/**< Ciphertexts */
  double **t;			/**< Power traces */
};

/** Pointer to the data structure */
typedef struct tr_context_s *tr_context;

/******************************************************
 * Trace context initialization, edition and deletion *
 ******************************************************/

/** Allocate a new context and initialize it. \return The allocated context.
 * \return the initialized context. */
tr_context tr_init_context (void);

/** Reads the trace file ::filename and stores the content into context ::ctx.
 * */
void tr_load_context (tr_context ctx /**< Context */ ,
		      char *filename
		      /**< Name of the trace file in HWSec format */ ,
    /** Maximum number of traces to read from the file (read all traces if 0) */
		      int max);

/** Closes and deallocates the previously initialized context. */
void tr_free_context (tr_context ctx /**< The context. */ );

/** Clone context ::ctx by allocating a new context and copying ::n entries of
 * ::ctx to it. \return The new context. */
tr_context tr_copy_context (tr_context ctx /**< The context. */ ,
			    int n
	  /**< Number of traces to copy */
  );

/** Trim all the traces of the context, keeping only ::length
 * points, starting from point number ::first_index */
void tr_trim_context (tr_context ctx /**< The context. */ ,
		      int from /**< The index of first point to keep. */ ,
		      int to
		    /**< The index of last point to keep. */
  );

/** Puncture all the traces of the context, removing samples between ::from and
 * ::to (both included). */
void tr_puncture_context (tr_context ctx /**< The context. */ ,
			  int from
			  /**< The index of first point to puncture. */ ,
			  int to
		    /**< The index of last point to puncture.*/
  );

/** Folds all the traces of the context by splitting them in ::length chunks and
 * accumulating them all, keeping only ::length points. The last incomplete
 * chunk, if any, is discarded. The first chunk start at sample 0 of the
 * original trace. */
void tr_fold_context (tr_context ctx /**< The context. */ ,
		      int length
		    /**< The number of points per chunk.*/
  );

/** Selects ::n traces of the context, starting from trace number ::first_trace,
 * and discards the others */
void tr_select_context (tr_context ctx /**< The context. */ ,
			int from /**< Index of first trace to keep. */ ,
			int to
		     /**< Index of last trace to keep. */
  );

/** Srink all the traces of the context, by replacing each chunk of
 * ::chunk_size points by their sum. If incomplete, the last chunk is
 * discarded */
void tr_shrink_context (tr_context ctx /**< The context. */ ,
			int chunk_size
		   /**< Number of points per chunk. */
  );

/** Writes the context in a HWSec trace file ::filename */
void tr_dump_context (tr_context ctx /**< The context. */ ,
		      char *filename /**< Name of output HWSec trace file. */
		      , int from /**< First trace to dump. */ ,
		      int to
	   /**< Last trace to dump. */
  );

/********************************************************************
 * Functions used to get information about a context or to retreive *
 * ciphertexts and power traces from it                             *
 ********************************************************************/

/** Returns the number of traces in a context. \return The number of traces in
 * the context. */
int tr_number (tr_context ctx
		    /**< The context. */
  );

/** Returns the number of points per trace of the context. \return The number of
 * points per trace in the context. */
int tr_length (tr_context ctx
		    /**< The context. */
  );

/** Returns the secret key the context. \return The secret key of the context.
 * */
uint64_t tr_key (tr_context ctx
		    /**< The context. */
  );

/** Returns the plaintext #::i of the context. \return The plaintext #::i of
 * the context. */
uint64_t tr_plaintext (tr_context ctx /**< The context. */ ,
		       int i
	  /**< Index of plaintext to return. */
  );

/** Returns the ciphertext #::i of the context. \return The ciphertext #::i of
 * the context. */
uint64_t tr_ciphertext (tr_context ctx /**< The context. */ ,
			int i
	  /**< Index of ciphertext to return. */
  );

/** Returns the power trace #::i of the context. \return The power trace #::i of
 * the context */
double *tr_trace (tr_context ctx /**< The context. */ ,
		  int i
      /**< Index of trace to return. */
  );

/***********************************************************************
 * Functions used to create, destroy, initialize and copy power traces *
 ***********************************************************************/

/** Allocates a new power trace. \return A pointer to the allocated trace */
double *tr_new_trace (int length /**< Trace length */ );

/** Deallocates a power trace previously allocated by tr_new_trace. */
void tr_free_trace (double *t /**< The trace to deallocate. */ );

/** Initializes the trace ::t with the scalar value ::val (each point is
 * assigned the value ::val). */
void tr_init_trace (int length /**< Trace length */ ,
		    double *t /**< The trace. */ ,
		    double val /**< The initialization value. */ );

/** Copies trace ::src to ::dest. dest[i] = src[i]. The traces ::dest and ::src
 * must be existing traces. */
void tr_copy_trace (int length /**< Traces length */ ,
		    double *dest /**< The destination trace. */ ,
		    double *src /**< The source trace. */ );

/**********************************
 * Arithmetic functions on traces *
 **********************************/

/** Adds traces ::src and ::dest and stores the result in ::dest: dest[i] =
 * dest[i] + src[i].  The traces ::dest and ::src must be existing traces. */
void tr_acc (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src /**< The source trace. */ );

/** Adds traces ::src1 and ::src2 and stores the result in ::dest: dest[i] =
 * src1[i] + src2[i]. The traces ::dest, ::src1 and ::src2 must be existing
 * traces. */
void tr_add (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src1 /**< The first source trace. */ ,
	     double *src2  /**< The second source trace. */
  );

/** Substracts traces ::src1 and ::src2 and stores the result in ::dest: dest[i]
 * = src1[i] - src2[i] The traces ::dest, ::src1 and ::src2 must be existing
 * traces. */
void tr_sub (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src1 /**< The first source trace. */ ,
	     double *src2  /**< The second source trace. */
  );

/** Multiplies the trace ::src by the scalar ::val: dest[i] = src[i] * val The
 * traces ::dest and ::src must be existing traces. */
void tr_scalar_mul (int length /**< Traces length */ ,
		    double *dest /**< The destination trace. */ ,
		    double *src /**< The source trace. */ ,
		    double val/**< The scalar value. */
  );

/** Divides the trace ::src by the scalar ::val: dest[i] = src[i] / val The
 * traces ::dest and ::src must be existing traces. An error is raised on
 * divisions by zero. */
void tr_scalar_div (int length /**< Traces length */ ,
		    double *dest /**< The destination trace. */ ,
		    double *src /**< The source trace. */ ,
		    double val/**< The scalar value. */
  );

/** Multiplies traces ::src1 and ::src2 and stores the result in ::dest: dest[i] =
 * src1[i] * src2[i] The traces ::dest, ::src1 and ::src2 must be existing
 * traces. */
void tr_mul (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src1 /**< The first source trace. */ ,
	     double *src2  /**< The second source trace. */
  );

/** Divides traces ::src1 and ::src2 and stores the result in ::dest: dest[i] =
 * src1[i] / src2[i] The traces ::dest, ::src1 and ::src2 must be existing
 * traces. An error is raised on divisions by zero. */
void tr_div (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src1 /**< The first source trace. */ ,
	     double *src2  /**< The second source trace. */
  );

/** Computes the square of trace ::src: dest[i] = src[i] * src[i] The traces
 * ::dest and ::src must be an existing traces. */
void tr_sqr (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src /**< The source trace. */ );

/** Computes the square root of trace ::src: dest[i] = sqrt(src[i]) The traces
 * ::dest and ::src must be an existing traces. An error is raised on negative
 * values. */
void tr_sqrt (int length /**< Traces length */ ,
	      double *dest /**< The destination trace. */ ,
	      double *src /**< The source trace. */ );

/** Computes the absolute value of trace ::src: dest[i] = (src[i] < 0.0) ?
 * -src[i] : src[i] The traces ::dest and ::src must be an existing traces. */
void tr_abs (int length /**< Traces length */ ,
	     double *dest /**< The destination trace. */ ,
	     double *src /**< The source trace. */ );

/** Returns the minimum value of trace ::t and stores its index in ::*idx.
\return The minimum value of trace ::t and the corresponding index in ::*idx. */
double tr_min (int length /**< Traces length */ ,
	       double *t /**< The trace. */ ,
	       int *idx /**< The argmin. */ );

/** Returns the minimum value of trace ::t between given bounds and stores its
 * index in ::*idx. \return The minimum value of trace ::t between given bounds
 * and the corresponding index in ::*idx. */
double tr_bounded_min (int from /**< Starting index. */ ,
		       int to /**< Ending index. */ ,
		       double *t /**< The trace. */ ,
		       int *idx /**< The argmin. */ );

/** Returns the maximum value of trace ::t and stores its index in ::*idx.
\return The maximum value of trace ::t and the corresponding index in ::*idx. */
double tr_max (int length /**< Traces length */ ,
	       double *t /**< The trace. */ ,
	       int *idx /**< The argmax. */ );

/** Returns the maximum value of trace ::t between given bounds and stores its
 * index in ::*idx. \return The maximum value of trace ::t between given bounds
 * and the corresponding index in ::*idx. */
double tr_bounded_max (int from /**< Starting index. */ ,
		       int to /**< Ending index. */ ,
		       double *t /**< The trace. */ ,
		       int *idx /**< The argmax. */ );

/** Prints trace ::t in ascii form, one point per line on standard output. */
void tr_print (int length /**< Traces length */ ,
	       double *t/**< The trace. */
  );

/** Print trace ::t in ascii form, one point per line in file ::fp. */
void tr_fprint (int length /**< Traces length */ ,
		FILE * fp /**< The descriptor of the output file. */ ,
		double *t/**< The trace. */
  );

#endif /* not TRACES_H */
