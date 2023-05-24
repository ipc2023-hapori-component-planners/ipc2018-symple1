// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <vector>
#include <algorithm>
#include <limits>
#include "./userDefiniedOperators.h"

void initializeUserDefiniedOps() {
  initializeRestrict();
  initializeMinVar();
  initializeSwapVarPrimed();
  initializeSwapToPrimedVar();
  initializeUnionMin();
  initializeIntersectionMax();
  initializeLessThan();
  initializeLessThanApprox();
  initializePartialComplement();
  initializeMinStates();
}

int minVal(MEDDLY::dd_edge& edge) {
  int res;
  edge.getEdgeValue(res);
  return res;
}

bool minPathRecursive(MEDDLY::expert_forest* f, MEDDLY::node_handle node,
std::vector<int>& res) {
  assert(res.size() == f->getNumVariables());

  if (f->isTerminalNode(node)) {
    return true;
  }

  // Get level information
  const int resultLevel = f->getNodeLevel(node);
  const int resultSize = f->getLevelSize(resultLevel);

  // Get a node reader
  MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(f, node, true);

  for (int i = 0; i < resultSize; i++) {
    // 0 cost path
    if (A->d(i) == 0)
      continue;
    if (A->ei(i) == 0) {
      res.at(resultLevel - 1) = i;
      MEDDLY::node_handle next = A->d(i);
      MEDDLY::unpacked_node::recycle(A);
      return minPathRecursive(f, next, res);
      break;
    }
  }
  return false;
}

std::vector<int> minPath(MEDDLY::dd_edge& edge) {
  auto expertF = static_cast<MEDDLY::expert_forest*>(edge.getForest());
  std::vector<int> res;
  for (int i = 0; i < expertF->getNumVariables(); i++) {
    res.push_back(MEDDLY::DONT_CARE);
  }
  bool correct = minPathRecursive(expertF, edge.getNode(), res);
  if (!correct)
    assert(false);
  return res;
}

int evaluatePath(MEDDLY::dd_edge& edge, std::vector<int>& assignment) {
  int val = 0;
  MEDDLY::node_handle node = edge.getNode();
  auto expertF = static_cast<MEDDLY::expert_forest*>(edge.getForest());
  edge.getEdgeValue(val);
  while (!expertF->isTerminalNode(node)) {
    // Get level information
    const int resultLevel = expertF->getNodeLevel(node);
    // Get a node reader
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(expertF, node,
    true);
    if (A->d(assignment[resultLevel]) != 0) {
      // Normal addition + check for inf!
      if ((val >= 0 && A->ei(assignment[resultLevel]) >= 0
      && val + A->ei(assignment[resultLevel]) < 0)
      || val == std::numeric_limits<int>::max()
      || A->ei(assignment[resultLevel]) == std::numeric_limits<int>::max()) {
        val = std::numeric_limits<int>::max();
      } else {
        val += A->ei(assignment[resultLevel]);
      }
    } else {
      val = std::numeric_limits<int>::max();
    }
    node = A->d_ref(assignment[resultLevel]);
    MEDDLY::unpacked_node::recycle(A);
  }
  //if (val < 0)
  //  val = std::numeric_limits<int>::max();
  return val;
}

void setInfintiyEdge(MEDDLY::dd_edge &res) {
  //res.set(SUPRESSINF ? 0 : -1, std::numeric_limits<int>::max());
  res.set(-1, std::numeric_limits<int>::max());
}

