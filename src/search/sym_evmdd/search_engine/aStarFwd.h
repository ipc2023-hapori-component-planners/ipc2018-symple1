// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include "symSearch.h"
#include "../meddly_extensions/userDefiniedOperators.h"
#include "../../operator.h"
#include "../../timer.h"

#ifndef ASTARFWD_H_
#define ASTARFWD_H_

class AStarFwd: public SymSearch {
  public:
  AStarFwd() :
  SymSearch() {
  }

  ~AStarFwd() {
  }

  protected:
  void image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res);

  void imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res);

  /**
   * One "backward" search step to find one operation which leads from on state set to
   * a given state.
   *
   * @param opTrans Transition relations representing operations.
   * @param statesBefore State set of the previous step.
   * @param statesAfter The state which the operation should lead to.
   * We use this state also to return the state which leads to the current
   * stateAfter.
   * @return The operator leading from the selected stateBefore
   * (now stored in statesAfter) to the current target state (statesAfter).
   */
  Operator* reconstructPlanStep(MEDDLY::dd_edge& targetState);

  /*Operator* reconstructPlanStep2(
  std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans,
  MEDDLY::dd_edge& statesBefore, MEDDLY::dd_edge& targetState);*/

  int selectAnyState(MEDDLY::dd_edge& targetState);
};

#endif // ASTARFWD_H_
