// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <algorithm>
#include <functional>
#include "symSearch.h"
#include "../image.h"

bool higherCost(Operator& op1, Operator& op2) {
  return op1.get_cost() > op2.get_cost();
}

void SymSearch::buildImage(
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& transOps,
MEDDLY::dd_edge& cheapest, MEDDLY::dd_edge& newStates) {
  newStates.set(-1, std::numeric_limits<int>::max());
  for (int i = 0; i < transOps.size(); i++) {
    MEDDLY::dd_edge cur(forest);
    this->image(cheapest, *transOps[i], cur);
    MEDDLY::apply(UNIONMIN, newStates, cur, newStates);
  }
}

bool SymSearch::reconstructSolution(MEDDLY::dd_edge& target,
std::vector<const Operator *>& plan) {
  MEDDLY::dd_edge curTargetStates(forest);

  std::vector<int> targetState = minPath(target);
  stateToEVMDD(forest, targetState, 0, curTargetStates);

  // Due to architecture we need to search for the target state in open or closed
  MEDDLY::dd_edge allReachabelStates(forest);
  MEDDLY::apply(UNIONMIN, open, closed, allReachabelStates);
  MEDDLY::apply(INTERSECTIONMAX, curTargetStates, allReachabelStates,
  curTargetStates);
  int planCost = minVal(curTargetStates);

  // We need to sort the operators according to there application cost due to 0 cost ops!
  //using namespace std::placeholders;
  if (!domain_has_sdac()) {
    std::sort(g_operators.begin(), g_operators.end(),
    std::bind(higherCost, std::placeholders::_1, std::placeholders::_2));
  }

  std::cout << "Beginning plan reconstruction: " << std::endl;
  std::cout << "Cur cost left: " << planCost << " => " << std::flush;

  MEDDLY::dd_edge startReached(forest);
  MEDDLY::apply(INTERSECTIONMAX, curTargetStates, start, startReached);

  curClosedList = closedLists.size();

  while (minVal(startReached) == std::numeric_limits<int>::max()) {
    Operator* curOp = reconstructPlanStep(curTargetStates);
    if (curOp == nullptr)
      return false;
    plan.insert(plan.begin(), curOp);

    planCost -= curOp->get_cost();
    std::cout << planCost << " => " << std::flush;
    MEDDLY::apply(INTERSECTIONMAX, curTargetStates, start, startReached);
  }
  std::cout << "DONE!" << std::endl;
  return true;
}

void SymSearch::step(std::vector<std::shared_ptr<MEDDLY::dd_edge>>& transOps,
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& mutexes) {
  double start = g_timer();

  MEDDLY::dd_edge cheapest(forest);
  MEDDLY::apply(MINSTATES, open, cheapest);

  MEDDLY::dd_edge newStates(forest);
  buildImage(transOps, cheapest, newStates);
  updateOpen(cheapest, newStates);

  MEDDLY::apply(UNIONMIN, closed, cheapest, closed);

  removeMutexStatesFromOpenStates(mutexes);

  timeLastStep = g_timer() - start;
}

void SymSearch::updateOpen(MEDDLY::dd_edge& cheapest,
MEDDLY::dd_edge& newStates) {
  MEDDLY::apply(UNIONMIN, closed, cheapest, closed);
  MEDDLY::dd_edge tmp(forest);
  MEDDLY::apply(PARTIALCOMPLEMENT, closed, tmp);
  MEDDLY::apply(UNIONMIN, open, newStates, open);
  MEDDLY::apply(INTERSECTIONMAX, open, tmp, open);

  // added to closed list
  closedLists.push_back(
  std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(cheapest)));

}

void SymSearch::removeMutexStatesFromOpenStates(
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& mutexes) {
  for (int i = 0; i < mutexes.size(); i++) {
    MEDDLY::apply(INTERSECTIONMAX, open, *mutexes[i], open);
  }
}
