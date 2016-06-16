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

/** \file tr_pcc.h
 *  The \b tr_pcc library, a software library dedicated to the computation of
 *  Pearson Correlation Coefficients (PCC).
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-29
 *
 * Defines a data structure and a set of functions used to compute and manage
 * Pearson Correlation Coefficients (PCC) between a floating point (double),
 * one-dimension, vector random variable and a set of floating point scalar
 * random variables.  These coefficients are a statistical tool to evaluate the
 * correlation between random variables. In this library, the vectors are
 * considered as a collection of scalar floating point random variables and each
 * component of the vectors is considered as independant. In the following,
 * vectors are denoted Z[.] and the i-th component of vector Z[.] is denoted
 * Z[i]. The formula of a PCC between the vector random variable X[.] and the
 * scalar random variable Y is:<br>
 *   PCC(X[.], Y)[.] = {E(X[.]*Y) - E(X[.])*E(Y)[.]} / {s(X[.]) * s(Y)[.]}<br>
 * where E(Z) is the expectation (average value) of random variable Z and s(Z)
 * is its unbiased standard deviation. E(Z[.])[.] is the expectation vector of
 * the vector random variable Z[.]: each component of E(Z[.])[.] is the
 * expectation of the corresponding component of Z[.]: E(Z[.])[i] = E(Z[i]).
 * Similarly, s(Z[.])[.] is the unbiased standard deviation vector of the vector
 * random variable Z[.]: s(Z[.])[i] = s(Z[i]). The product between a vector and
 * a scalar is defined as: (X[.]*Y)[i] = X[i]*Y. The PCC between a vector random
 * variable X and a scalar random variable Y is a vector defined by:<br>
 *   PCC(X[.], Y)[i] = PCC(X[i], Y)<br>
 * The \b tr_pcc library can be used to compute a set of PCCs between one vector
 * random variable (denoted X[.]), common to all PCCs, and a set of scalar
 * random variables (denoted Y0, Y1, ..., Yn-1). To compute such a set of PCCs
 * one must first initialize a PCC context (tr_pcc_init()), indicating the
 * number ny of Y random variables. Then, realizations of the random variables
 * must be accumulated into the context: first a realization of the X[.]
 * variable (tr_pcc_insert_x()), followed by realizations of each of the ny Y
 * variables (tr_pcc_insert_y()). Once a sufficient number of realizations are
 * accumulated in the PCC context, a call to tr_pcc_consolidate() computes the
 * ny PCCs. Calls to tr_pcc_get_pcc() return the different vector PCCs. Note:
 * more realizations can be accumulated after a call to tr_pcc_consolidate(),
 * another call to tr_pcc_consolidate() will take them into account altogether
 * with the previous ones and compute the new PCC values. A call to
 * tr_pcc_free() deallocates the PCC context and makes it reusable for the
 * computation of a new set of PCCs. Example of use with 10-components vectors
 * and ny=4 Y scalar variables, in which get_next_x and get_next_y are two
 * functions returning realizations of the random variables:
 * \code
 * tr_pcc_context ctx;
 * double *x, y, *pcc;
 * int i, j, nexp;
 * ...
 * ctx = tr_pcc_init(10, 4);
 * for(i = 0; i < nexp; i++) {
 *   x = get_next_x();
 *   tr_pcc_insert_x(ctx, x);
 *   for(j = 0; j < 4; j++) {
 *     y = get_next_y(j);
 *     tr_pcc_insert_y(ctx, j, y);
 *     }
 *   }
 * tr_pcc_consolidate(ctx);
 * for(i = 0; i < 4; i++) {
 *   pcc = tr_pcc_get_pcc(ctx, j);
     printf("PCC(X[.], Y%d)[.] =", i);
 *   for(j = 0; j < 10; j++)
 *     printf(" %lf", pcc[j]);
 *     printf("\n");
 *   }
 * tr_pcc_free(ctx);
 * ctx = tr_pcc_init(100, 12);
 * ...
 * tr_pcc_free(ctx);
 * \endcode
 * \attention 
 * It is an error to break the realization insertion scheme: if you initialized
 * your PCC context for ny Y variables, first insert a realization of X[.],
 * followed by one and only one realization of each of the ny Y variables. Then,
 * insert a new realization of X[.] and ny new realizations of the ny Y
 * variables, and so on.  Consolidate only after inserting the realization of
 * the last Y variable. Do not consolidate when in an intermediate state.
 * */

#ifndef TR_PCC_H
#define TR_PCC_H

 /** PCC biased computation mode: pcc=(E(x.y)-E(x).E(y))/(bstd(x).bstd(y))
  * where bstd(z) is the biased standard deviation E((z-E(z))^2). */
#define BIASED 1
 /** PCC unbiased computation mode: pcc=(E(x.y)-E(x).E(y))/(ubstd(x).ubstd(y))
  * where ubstd(z) is the unbiased standard deviation sum((z-E(z))^2)/(n-1). */
#define UNBIASED 2
 /** Squared PCC biased computation mode:
  * pcc=[(E(x.y)-E(x).E(y))/(bstd(x).bstd(y))]^2 where bstd(z) is the biased
  * standard deviation E((z-E(z))^2). */
#define SQUAREBIASED 3
 /** Squared PCC unbiased computation mode:
  * pcc=[(E(x.y)-E(x).E(y))/(ubstd(x).ubstd(y))]^2 where ubstd(z) is the unbiased
  * standard deviation sum((z-E(z))^2)/(n-1). */
#define SQUAREUNBIASED 4

/** The data structure used to compute and manage a set of Pearson correlation
 * coefficients. */
struct tr_pcc_context_s
{
  int ny;      /**< The number of Y random variables. */
  int nr;      /**< The current number of realizations of the random variables. */
  int l;       /**< The size of the vector random variable (number of components). */
  double *rx;	/**< The last inserted realization of X. */
  double *x;	/**< The sum of the realizations of X. */
  double *x2;	/**< The sum of the squares of the realizations of X. */
  double *y;	/**< The array of the sums of the realizations of the Ys. */
  double *y2;	/**< The array of the sums of the squares of the realizations of the Ys. */
  double **xy;	/**< The array of the sums of the products between realizations of X and Ys. */
  double **pcc;	/**< The array of the PCCs. */
  char state;  /**< Tracker for insertion of the realizations. */
  int cnt;     /**< Tracker for insertion of the realizations. */
  char *flags; /**< Tracker for insertion of the realizations. */
  double *tmp1;	/**< Temporary trace. */
  double *tmp2;	/**< Temporary trace. */
  double *tmp3;	/**< Temporary trace. */
};

/** Pointer to the tr_pcc_context_s data structure. */
typedef struct tr_pcc_context_s *tr_pcc_context;

/** Initializes a PCC context.
 * \return An initialized PCC context. */
tr_pcc_context tr_pcc_init (int l,
	   /**< The length of the X[.] vector random variable (number of components). */
			    int ny
	   /**< The number of Y random variables to manage. */
  );

/** Returns the number of Y random variables in a context. \return The number of
 * Y random variables in the context. */
int tr_pcc_number (tr_pcc_context ctx
		    /**< The context. */
  );

/** Returns the traces length in a context. \return The traces length
 * in the context. */
int tr_pcc_length (tr_pcc_context ctx
		    /**< The context. */
  );

/** Insert a realization of X[.] in a PCC context */
void tr_pcc_insert_x (tr_pcc_context ctx, /**< The context */
		      double *x
	     /**< The realization of X[.]. */
  );

/** Inserts a new Y realization in a PCC context. */
void tr_pcc_insert_y (tr_pcc_context ctx, /**< The context */
		      int ny,
	   /**< The index of the Y random variable (0 to ctx->ny - 1). */
		      double y
		      /**< The realization of the Y random variable. */ );

/** Consolidates a set of PCCs (computes all the PCCs from the already inserted
 * realizations). */
void tr_pcc_consolidate (tr_pcc_context ctx,
			/**< The context */
    /** The computation mode (BIASED, UNBIASED, SQUAREBIASED or SQUAREUNBIASED) */
			 int mode);

/** Returns a pointer to the PCC vector number ::ny. \return A pointer to the PCC
 * vector number ::ny */
double *tr_pcc_get_pcc (tr_pcc_context ctx, /**< The context */
			int ny
		     /**< The index of the PCC to get (0 to ctx->ny - 1). */
  );

/** Closes the manager and deallocates it. */
void tr_pcc_free (tr_pcc_context ctx);

#endif /* not TR_PCC_H */
