// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef SYMSEARCHENGINE_H_
#define SYMSEARCHENGINE_H_

#include "../search_engine.h"
#include "../utilities.h"
#include "resourcesHandle.h"
#include "search_engine/aStarFwd.h"
#include "search_engine/aStarBwd.h"

enum class SearchDirection {
  BIDIRECTIONAL, PROGRESSION, REGRESSION
};

enum class PlanType {
  OPT, SAT, AGL, CBO
};

class symple: public SearchEngine {
  protected:

  virtual int step();
  virtual void initialize();

  /// initialize functions
  void createDomain();
  void createForest();
  void createOperatorTransitions();
  void createMutexes(bool fwd);
  void createInitState();
  void createGoalState();

  void dumpPlannerInformation();

  /// Write information about the current search step to the console.
  void dumpStepInformation(int minFwdCost, int minBwdCost);

  // Clean up Meddly and print lib status
  void cleanUpMeddly();

  // Performs search step
  void forwardStep();
  void backwardStep();

  bool validatePlan(std::vector<const Operator *> &plan);

  /** Option of different transition relations for operators.
   * -1 = One big relation for all operators
   *  0 = One relation per operator
   *  x = if a transition has more nodes than x we create a new one
   */
  int transOption;

  /// Serach directions. 0 = Bidirectional search, 1 = Progression, 2 = Regression
  SearchDirection searchDirection;

  /// Planning type. 0 = OPT, 1 = SAT, 2 = AGILE, 3 = COSTBOUND
  PlanType planning_type;
  int plan_id;

  /// Symbolic representations
  MEDDLY::domain* dom;
  MEDDLY::forest* forest;
  MEDDLY::dd_edge initState;
  MEDDLY::dd_edge goalState;
  std::vector<shared_ptr<MEDDLY::dd_edge>> mutexesFWD;
  std::vector<shared_ptr<MEDDLY::dd_edge>> mutexesBWD;

  /// Current cheapest collision (goal)
  int collisionCost;
  MEDDLY::dd_edge collisionFound;

  /// Search status
  int nStepsFwd;
  int nStepsBwd;
  bool lastStepFwd;
  /// Stores the transitions of operators in the same order as g_operators
  std::vector<shared_ptr<MEDDLY::dd_edge>> transOps;
  int minActionCost;


  /// Serach engines
  AStarFwd astarFwd;
  AStarBwd astarBwd;

  /// Print helper
  unique_ptr<MEDDLY::FILE_output> meddlyout;

  public:
  symple(const Options &opts);

  ~symple() {
  }

};

#endif // SYMSEARCHENGINE_H_
