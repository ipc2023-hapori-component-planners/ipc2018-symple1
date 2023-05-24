// Copyright 10.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef USERDEFINIEDOPERATORS_H_
#define USERDEFINIEDOPERATORS_H_

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./unionMin.h"
#include "./intersectionMax.h"
#include "./restrict.h"
#include "./minVar.h"
#include "./swapVarPrimed.h"
#include "./swapToPrimedVar.h"
#include "./partialComplement.h"
#include "./minStates.h"
#include "../resourcesHandle.h"

void initializeUserDefiniedOps();

/**
 * Returns the minimum path value of an Edge
 * (value of the root node)
 *
 * @param edge Edge to evalute.
 * @return Minimum path value.
 */
int minVal(MEDDLY::dd_edge& edge);

/**
 * Return the values of all variables which define the
 * path with minimum value. (-1 if the value of an var
 * does not care)
 *
 * @param edge Edge to evaluate.
 * @return Variable values of the minimum path.
 */
std::vector<int> minPath(MEDDLY::dd_edge& edge);

int evaluatePath(MEDDLY::dd_edge& edge, std::vector<int>& assignment);

void setInfintiyEdge(MEDDLY::dd_edge &res);


#endif  // USERDEFINIEDOPERATORS_H_
