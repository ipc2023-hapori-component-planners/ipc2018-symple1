// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include "symSearch.h"
#include "../meddly_extensions/userDefiniedOperators.h"
#include "../../operator.h"
#include "../../timer.h"

#ifndef ASTARBWD_H_
#define ASTARBWD_H_

class AStarBwd: public SymSearch {
  public:
  AStarBwd() :
  SymSearch() {
  }

  ~AStarBwd() {
  }

  /**
   * Handles the complete backward search.
   * Loops over all reached states and fills the plan.
   *
   * @param transOps Transition relations representing operations.
   * @param plan A plan which will be filled with operations.
   */
  bool reconstructSolution(
  MEDDLY::dd_edge& target, std::vector<const Operator *>& plan);

  protected:

  void image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res);

  void imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res);

  Operator* reconstructPlanStep(MEDDLY::dd_edge& targetState);

  /*Operator* reconstructPlanStep2(
  std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans,
  MEDDLY::dd_edge& statesBefore, MEDDLY::dd_edge& targetState);*/

  int selectAnyState(MEDDLY::dd_edge& targetState);
};

#endif // ASTARBWD_H_
