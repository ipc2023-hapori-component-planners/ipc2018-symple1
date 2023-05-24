// Copyright 16.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <limits>
#include <algorithm>
#include "./genericBinaryOperator.h"
#include "../resourcesHandle.h"

// high-level front-end
void genericBinaryOperator::compute(const MEDDLY::dd_edge &ar1,
const MEDDLY::dd_edge &ar2, MEDDLY::dd_edge &res) {
  if (ar1 == ar2) {
    res = ar1;
    return;
  }
  computeDDEdge(ar1, ar2, res);
}

void genericBinaryOperator::computeDDEdge(const MEDDLY::dd_edge &ar1,
const MEDDLY::dd_edge &ar2, MEDDLY::dd_edge &res) {
  MEDDLY::node_handle result;
  int ev, a1ev, a2ev;
  ar1.getEdgeValue(a1ev);
  ar2.getEdgeValue(a2ev);
  compute(a1ev, ar1.getNode(), a2ev, ar2.getNode(), ev, result);

  // Check if we have only terminal node and set correct value
  if (result == 0) {
    ev = std::numeric_limits<int>::max();
  }
  res.set(result, ev);
}

/// Low-level compute on EV edges (av, ap) and (bv, bp), return result.
void genericBinaryOperator::compute(int av, MEDDLY::node_handle ap, int bv,
MEDDLY::node_handle bp, int &cv, MEDDLY::node_handle &cp) {
  //std::cout << ap << ", " << av << " and " << bp << ", " << bv << std::endl;
  if (checkTerminals(av, ap, bv, bp, cv, cp)) {
    return;
  }
  MEDDLY::compute_table::search_key* Key = findResult(av, ap, bv, bp, cv, cp);
  if (0 == Key) {
    return;
  }

  // Get level information
  const int aLevel = arg1F->getNodeLevel(ap);
  const int bLevel = arg2F->getNodeLevel(bp);

  const int resultLevel = aLevel > bLevel ? aLevel : bLevel;
  const int resultSize = resF->getLevelSize(resultLevel);

  // Initialize result
  MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(resF, resultLevel,
  resultSize);

  // Initialize readers : create a node_reader for either the current
  // level of a or for the lower (b)
  MEDDLY::unpacked_node *A =
  (aLevel < resultLevel) ?
  MEDDLY::unpacked_node::newRedundant(arg1F, resultLevel, 0, ap, true) :
  MEDDLY::unpacked_node::newFromNode(arg1F, ap, true);

  MEDDLY::unpacked_node *B =
  (bLevel < resultLevel) ?
  MEDDLY::unpacked_node::newRedundant(arg2F, resultLevel, 0, bp, true) :
  MEDDLY::unpacked_node::newFromNode(arg2F, bp, true);

  for (int i = 0; i < resultSize; i++) {
    int ev;
    MEDDLY::node_handle ed;

    // Check for + inf problem
    int alpha =
    A->ei(i) == std::numeric_limits<int>::max() ?
    std::numeric_limits<int>::max() : av - OpType(av, bv) + A->ei(i);

    int beta =
    B->ei(i) == std::numeric_limits<int>::max() ?
    std::numeric_limits<int>::max() : bv - OpType(av, bv) + B->ei(i);

    // Recursive call with carry
    compute(alpha, A->d(i), beta, B->d(i), ev, ed);
    nb->d_ref(i) = ed;
    nb->setEdge(i, ev);
  }

  // cleanup
  MEDDLY::unpacked_node::recycle(B);
  MEDDLY::unpacked_node::recycle(A);

  // Create a node handle...links cl to nb with cost cv
  //MEDDLY::node_handle cl;
  resF->createReducedNode(-1, nb, cv, cp);

  // Add to CT
  saveResult(Key, ap, bp, cv, cp);

  int tmp_cv = cv + OpType(av, bv);
  /*// Last call -> set cv and add the reduced value from above!!
   // Necessary for max -> if e.g. 1,1 edge values -> normalized to 1
   if (cv > 0 && OpType(av, bv) > 0 && cv + OpType(av, bv) < 0) {
   cv = std::numeric_limits<int>::max();
   } else {

   }*/
  if (tmp_cv < 0 && cv >= 0 && OpType(av, bv) >= 0) {
    cv = std::numeric_limits<int>::max();
  } else if (cv == std::numeric_limits<int>::max()
  && OpType(av, bv) == std::numeric_limits<int>::max()) {
    cv = std::numeric_limits<int>::max();
  } else {
    cv = tmp_cv;
  }
}

void genericBinaryOperator::discardEntry(const MEDDLY::node_handle *entryData) {
  arg1F->uncacheNode(entryData[1]);
  arg2F->uncacheNode(entryData[2]);
  resF->uncacheNode(entryData[4]);
}

void genericBinaryOperator::showEntry(MEDDLY::output &strm,
const MEDDLY::node_handle *data) const {
  strm << "[" << getName() << "(<" << long(data[0]) << ":" << long(data[1])
  << ">, <" << long(data[2]) << ":" << long(data[3]) << ">): <" << long(data[4])
  << ":" << long(data[5]) << ">]";
}

bool genericBinaryOperator::isStaleEntry(const MEDDLY::node_handle* data) {
  return arg1F->isStale(data[1]) || arg2F->isStale(data[2])
  || resF->isStale(data[4]);
}

