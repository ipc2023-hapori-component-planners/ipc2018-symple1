// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <map>
#include "../meddly_extensions/userDefiniedOperators.h"
#include "../../operator.h"
#include "../../timer.h"

#ifndef SYMSEARCH_H_
#define SYMSEARCH_H_

class SymSearch {
  public:
  SymSearch() {
    timeLastStep = 0;
    curClosedList = -1;
    forest = nullptr;
  }

  virtual ~SymSearch() {
  }

  void initialize(MEDDLY::forest* forest, MEDDLY::dd_edge& start) {
    this->start = start;
    this->open = start;
    this->forest = forest;
    closed = MEDDLY::dd_edge(forest);
    closed.set(-1, std::numeric_limits<int>::max());
    MEDDLY::apply(UNIONMIN, closed, start, closed);
    //closed.set(-1, 0);
  }

  void step(std::vector<std::shared_ptr<MEDDLY::dd_edge>>& transOps,
  std::vector<std::shared_ptr<MEDDLY::dd_edge>>& mutexes);

  /**
   * Handles the complete backward search.
   * Loops over all reached states and fills the plan.
   *
   * @param transOps Transition relations representing operations.
   * @param plan A plan which will be filled with operations.
   */
  bool reconstructSolution(MEDDLY::dd_edge& target,
  std::vector<const Operator *>& plan);

  double getTimeLastStep() {
    return timeLastStep;
  }

  MEDDLY::dd_edge* getOpen() {
    return &open;
  }

  MEDDLY::dd_edge* getClosed() {
    return &closed;
  }

  protected:
  MEDDLY::forest* forest;
  MEDDLY::dd_edge start;
  MEDDLY::dd_edge open;
  MEDDLY::dd_edge closed;
  std::map<int, std::vector<std::shared_ptr<MEDDLY::dd_edge>>>closedLayers;
  std::vector<std::shared_ptr<MEDDLY::dd_edge>> closedLists;
  int curClosedList;

  double timeLastStep;

  virtual void image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res) = 0;

  virtual void imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
  MEDDLY::dd_edge &res) = 0;

  virtual Operator* reconstructPlanStep(MEDDLY::dd_edge& targetState) = 0;

  private:

  void updateOpen(MEDDLY::dd_edge& cheapest, MEDDLY::dd_edge& newStates);

  void removeMutexStatesFromOpenStates(
  std::vector<std::shared_ptr<MEDDLY::dd_edge>>& mutexes);

  void buildImage(std::vector<std::shared_ptr<MEDDLY::dd_edge>>& transOps,
  MEDDLY::dd_edge& cheapest, MEDDLY::dd_edge& newStates);
};

bool higherCost(Operator& op1, Operator& op2);

#endif // SYMSEARCH_H_
