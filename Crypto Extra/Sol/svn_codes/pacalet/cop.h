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

/** \file cop.h
 *  The \b cop library, a software library dedicated to cooperative optimization
 *  for boolean variables.
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 *
 * The cooperative optimization algorithm implemented by the following functions
 * is based on the theory introduced by Xiaofei Huang (see, for instance,
 * "Cooperative Optimization for Energy Minimization: A Case Study of Stereo
 * Matching", in Pattern Recognition, 26th DAGM Symposium, Springer-Verlag, LNCS
 * 3175, 2004, pp. 302–309.
 * \attention
 * The number of agents must be between 1 and 64. Larger sets are not yet
 * supported.
 */

#ifndef COP_H
#define COP_H

#include <stdint.h>

#include <utils.h>

/** The data structure for a single agent */
struct agent_s
{
  int *nbs; /**< The set of its neighbors (including itself) */
  int nn; /**< The number of neighbors (including itself) */
  double w; /**< The weight of this agent's choice when used by other agents */
  /** The agent's choice for value 0 of its variable (phy[.][0]) and for value 1
   * (phy[.][1]). The two choices sets phy[0][.] and phi[1][.] are used to hold
   * old and new choices, in a circular buffer way. */
  double phi[2][2];
  double *e; /**< The agent's subfunction (2^nn values) */
  int decision;	/**< Current decision */
  int consensus;	/**< Consensus counter */
};

/** Pointer to ::agent_s */
typedef struct agent_s *agent;

/** The main data structure */
struct cop_context_s
{
  agent a; /**< The array of all agents */
  int na; /**< Number of agents */
  int new; /**< Index (0 or 1) of the new choices. Toggles at each iteration */
  double cost; /**< Cost function */
  int consensus;	/**< Consensus counter */
  int stability;	/**< Stability counter */
};

/** Pointer to ::cop_context_s */
typedef struct cop_context_s *cop_context;

/** Initialize a ::cop_context for a given number of variables. \return An
 * initialized context. Agents are intialized in a fully disjoint way: each of
 * them has itself as a unique neighbor. Their local cooperation weights W are
 * set to 1.0 and their old and new choices are initialized to 0.0. */
cop_context cop_init (int n /**< Number of variables */ );

/** Return the current decision of agent ::idx of context ::ctx. \return The
 * current decision of agent ::idx of context ::ctx. */
int cop_decision (cop_context ctx,
		     /**< Context */
		  int idx
	    /**< Agent */
  );

/** Return the current confidence of agent ::idx of context ::ctx. \return The
 * current confidence of agent ::idx of context ::ctx. */
double cop_confidence (cop_context ctx,
		     /**< Context */
		       int idx
	    /**< Agent */
  );

/** Return the current consensus on agent ::idx of context ::ctx. \return The
 * current consensus on agent ::idx of context ::ctx: 0 = no consensus, 1 =
 * consensus */
int cop_consensus (cop_context ctx,
		     /**< Context */
		   int idx
	    /**< Agent */
  );

/** Initialize the phi[.][.] values, decision, consensus, ... of a ::cop_context
 * to zero. */
void cop_init_agents (cop_context ctx /**< Context */ );

/** Run one iteration of cooperative optimization. \return 0 when stability is
 * reached, else a non-0 value */
int cop_iterate (cop_context ctx /**< Context */ ,
		 double l /**< Lambda cooperation factor */ );

/** Ask each agent to make a decision concerning its variable. \return the
 * variables values as an ::uint64_t 64 bits unsigned integer. Decision on
 * variable #0 is bit #0 of the returned value. */
uint64_t cop_vote (cop_context ctx /**< Context */ );

/** Deallocate a cop_context. */
void cop_free (cop_context ctx /**< Context */ );

#endif /* not COP_H */
