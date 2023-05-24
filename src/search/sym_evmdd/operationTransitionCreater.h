// Copyright 29.06.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef OPERATIONTRANSITIONCREATER_H_
#define OPERATIONTRANSITIONCREATER_H_

#include <meddly.h>
#include <meddly_expert.h>
#include "sdac_parser/parser.h"
#include "../operator.h"

// Create transition relations for a list of operations with a certain node limit
void createOperationTransition(MEDDLY::forest* f, vector<Operator>& ops,
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans, int nodeLimit);

// Creates and stores the cost function of an operator as EVMDD
void createOperationCostFunction(MEDDLY::forest* f, Operator* op);

#endif // OPERATIONTRANSITIONCREATER_H_
