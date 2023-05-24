// Copyright 24.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef IMAGE_H_
#define IMAGE_H_

#include <meddly.h>
#include <meddly_expert.h>
#include "../operator.h"
#include <vector>
#include <limits>

void stateToEVMDD(MEDDLY::forest* f, std::vector<int> &vals, int cost,
MEDDLY::dd_edge& res);

void createOpTrans(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res, bool withCost=true);

void image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res);

void imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res);

void preVarEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res, bool inverted=false);

void effVarEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res);

void valEqualsValEdge(MEDDLY::expert_forest* f, int var1, int var2,
MEDDLY::node_handle &r);

void condEffVarsEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res);

bool writeEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& edge);

bool readEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& res);

bool deleteFile(std::string fileName);

#endif  // IMAGE_H_
