// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <stdio.h>
#include <limits>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "meddly_extensions/userDefiniedOperators.h"
#include "symple.h"
#include "image.h"
#include "operationTransitionCreater.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../state.h"
#include "../axioms.h"

symple::symple(const Options &opts) :
SearchEngine(opts) {

  // Parse transition relation node limit
  if (transOption < -1)
    transOption = 0;
  transOption = opts.get<int>("transition");

  // Parse search direction
  int dir = opts.get<int>("direction");
  if (dir < 0 || dir > 2)
    dir = 0;
  searchDirection = static_cast<SearchDirection>(dir);

  int type = opts.get<int>("planning_type");
  if (type < 0 || type > 3)
    type = 0;
  planning_type = static_cast<PlanType>(type);

  dumpPlannerInformation();

  /// Init members
  dom = nullptr;
  forest = nullptr;
  meddlyout = unique_ptr<MEDDLY::FILE_output>(new MEDDLY::FILE_output(stdout));
  collisionCost = std::numeric_limits<int>::max();
  nStepsFwd = nStepsBwd = 0;
  lastStepFwd = true;
}

void symple::initialize() {
  // Set up Meddly
  initializeUserDefiniedOps();
  MEDDLY::initializer_list* L = MEDDLY::defaultInitializerList(0);
  MEDDLY::ct_initializer::setMaxSize(16777216);
  MEDDLY::initialize(L);

  // Set up symbolic representations
  createDomain();
  createForest();
  createMutexes(true);
  createMutexes(false);
  createInitState();
  createGoalState();
  createOperatorTransitions();

  // Set up serach engines
  astarFwd.initialize(forest, initState);
  astarBwd.initialize(forest, goalState);

  // Set up minimal action costs
  minActionCost = 0;
  if (!domain_has_sdac()) {
    minActionCost = std::numeric_limits<int>::max();
    for (int i = 0; i < g_operators.size(); i++) {
      if (g_operators[i].get_cost() < minActionCost)
        minActionCost = g_operators[i].get_cost();
    }
  }
  std::cout << "Min action cost: " << minActionCost << std::endl;
  plan_id = 1;

  // Print some final stats
  std::cout << "--- After Creating Symbolic Data Structures ---" << std::endl;
  std::cout << "- #Nodes: " << forest->getCurrentNumNodes() << std::endl;
  std::cout << "- Mem: " << forest->getCurrentMemoryUsed() / 1000.00 / 1000.00
  << "MB" << std::endl;
  std::cout << "-----------------------------------------------" << std::endl;
}

int symple::step() {
  try {
    // Check if a new collision is found and obtain cost
    int minFwdCost = minVal(*astarFwd.getOpen());
    int minBwdCost = minVal(*astarBwd.getOpen());
    int minCost = minFwdCost + minBwdCost;
    MEDDLY::dd_edge curCollision(forest);

    // Compare the open list (new generated states) with the already
    // explored states of the opposite search direction
    /*if (lastStepFwd) {
     MEDDLY::dd_edge tmp(forest);
     MEDDLY::apply(UNIONMIN, *astarBwd.getClosed(), *astarBwd.getOpen(), tmp);
     MEDDLY::apply(MEDDLY::PLUS, *astarFwd.getOpen(), tmp,
     curCollision);
     } else {
     MEDDLY::apply(UNIONMIN, *astarBwd.getClosed(), *astarBwd.getOpen(), tmp);
     MEDDLY::apply(MEDDLY::PLUS, *astarFwd.getClosed(), *astarBwd.getOpen(),
     curCollision);
     }*/

    //TODO(speckd): is that really sound and is nothing crashing with reconstruct plan
    MEDDLY::dd_edge states_fwd(forest);
    MEDDLY::dd_edge states_bwd(forest);
    MEDDLY::apply(UNIONMIN, *astarFwd.getClosed(), *astarFwd.getOpen(),
    states_fwd);
    MEDDLY::apply(UNIONMIN, *astarBwd.getClosed(), *astarBwd.getOpen(),
    states_bwd);
    MEDDLY::apply(MEDDLY::PLUS, states_fwd, states_bwd, curCollision);

    // Check if current collision is cheaper than the old collison (cheaper goal)
    if (minVal(curCollision) < collisionCost) {
      collisionFound = curCollision;
      collisionCost = minVal(curCollision);

      /// Only for sat planning
      if (planning_type == PlanType::SAT) {
        std::cout << "\n\t*** " << g_plan_filename.c_str() << "." << plan_id
        << " ***" << std::endl;
        Plan plan;
        double startRec = g_timer();
        if (!astarFwd.reconstructSolution(collisionFound, plan)) {
          return FAILED;
        }
        if (!astarBwd.reconstructSolution(collisionFound, plan)) {
          return FAILED;
        }

        if (!validatePlan(plan)) {
          std::cerr << "Plan not valid!" << std::endl;
          return FAILED;
        } 

        save_plan(plan, plan_id);
        std::cout << "Plan reconstruction took: " << g_timer() - startRec
        << "sec" << std::endl;
        plan_id++;
        std::cout << "\n" << std::endl;
      }

      /// Only for Agile and costbound planning
      if (planning_type == PlanType::AGL
      || (planning_type == PlanType::CBO && collisionCost <= bound)) {
        // Reconstruct plan
        Plan plan;
        double startRec = g_timer();
        if (!astarFwd.reconstructSolution(collisionFound, plan)) {
          return FAILED;
        }
        if (!astarBwd.reconstructSolution(collisionFound, plan)) {
          return FAILED;
        }
        
       if (!validatePlan(plan)) {
          std::cerr << "Plan not valid!" << std::endl;
          return FAILED;
        }
 
        //if (sat_planning)
        //  save_plan(plan, plan_id);
        this->set_plan(plan);
        std::cout << "Plan reconstruction took: " << g_timer() - startRec
        << "sec" << std::endl;

        // Clean up
        printMemoryState(forest);
        cleanUpMeddly();
        std::cout << "NOT RELEASED MEMORY: " << std::flush;
        print_used_memory();
        return SOLVED;
      }
    }

    /// Only for costbounded
    if (planning_type == PlanType::CBO && minCost + minActionCost > bound)
      return FAILED;

    // There exist no cheaper goal => return optimal plan
    if (collisionCost <= minCost + minActionCost) {
      if (planning_type == PlanType::CBO)
        return FAILED;
      // Some outputs
      std::cout << "Found solution!" << std::endl;
      std::cout << "Solution cost: " << collisionCost << std::endl;
      std::cout << "Solution found in step: " << nStepsFwd + nStepsBwd
      << std::endl;

      // Reconstruct plan
      Plan plan;
      double startRec = g_timer();
      if (!astarFwd.reconstructSolution(collisionFound, plan)) {
        return FAILED;
      }
      if (!astarBwd.reconstructSolution(collisionFound, plan)) {
        return FAILED;
      }
      if (!validatePlan(plan)) {
        std::cerr << "Plan not valid!" << std::endl;
        return FAILED;
      } 
      //if (sat_planning)
      //  save_plan(plan, plan_id);
      this->set_plan(plan);
      std::cout << "Plan reconstruction took: " << g_timer() - startRec << "sec"
      << std::endl;

      // Clean up
      printMemoryState(forest);
      cleanUpMeddly();
      std::cout << "NOT RELEASED MEMORY: " << std::flush;
      print_used_memory();
      return SOLVED;
    }

    /// Start stepping
    dumpStepInformation(minFwdCost, minBwdCost);

    // Bidirectional Search
    if (searchDirection == SearchDirection::BIDIRECTIONAL) {
      // Time comparision of last steps
      if (astarFwd.getTimeLastStep() <= astarBwd.getTimeLastStep()) {
        forwardStep();
      } else {
        backwardStep();
      }
    }
    // Progression
    if (searchDirection == SearchDirection::PROGRESSION)
      forwardStep();
    // Regression
    if (searchDirection == SearchDirection::REGRESSION)
      backwardStep();
    return IN_PROGRESS;
  } catch (const MEDDLY::error& e) {
    std::cout << "MEDDLY ERROR (MEMORY LIMIT REACHED)!!!" << std::endl;
    exit_with(EXIT_OUT_OF_MEMORY);
  }

// We should never reach this part of code =)
  return FAILED;
}

void symple::createDomain() {
  // Create vars explicitly in oder to name all of them
  const int nVars = g_variable_domain.size() * 2;
  MEDDLY::variable** vars = new MEDDLY::variable*[g_variable_domain.size() * 2
  + 1];
  vars[0] = 0;
  for (int i = 0; i < g_variable_domain.size(); i++) {
    // create primed version
    vars[2 * i + 1] = MEDDLY::createVariable(g_variable_domain[i],
    strdup((g_variable_name[i] + "'").c_str()));
    // create unprimed version
    vars[2 * i + 2] = MEDDLY::createVariable(g_variable_domain[i],
    strdup((g_variable_name[i]).c_str()));
  }
  this->dom = MEDDLY::createDomain(vars, nVars);
}

void symple::createForest() {
  this->forest = this->dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
//forest->getPolicies().compactAfterGC = true;
//forest->getPolicies().setNeverDelete();
  forest->getPolicies().setPessimistic();
}

void symple::createOperatorTransitions() {
// First we create for every operator the costFunction EVMDD
  std::cout << "Creating cost functions..." << std::flush;
  for (int i = 0; i < g_operators.size(); i++) {
    createOperationCostFunction(forest, &g_operators[i]);
  }
  std::cout << "Done." << std::endl;

// Create transition for all operators
  std::cout << "Creating transition relations..." << std::flush;
  // Different options for transition relations
  if (transOption == -1) {
    createOperationTransition(forest, g_operators, transOps,
    std::numeric_limits<unsigned>::max());
  } else if (transOption == 0) {
    createOperationTransition(forest, g_operators, transOps, 0);
  } else {
    createOperationTransition(forest, g_operators, transOps, transOption);
  }
  std::cout << "Done. => #TR's: " << transOps.size() << std::endl;
  //transOps.back()->show(*meddlyout, 3);
}

void symple::createMutexes(bool fwd) {
  std::cout << "Creating mutexes..." << std::flush;
  std::vector<shared_ptr<MEDDLY::dd_edge>> mutexes;
  auto mutex = std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(forest));
  mutex->set(-1, 0);
  //std::cout << "#MUTEX: " << g_mutex_groups.size() << std::flush;

  // for every mutex create an EVMDD and combine their complement until node limit is exceeded
  for (int i = 0; i < g_mutex_groups.size(); i++) {
    if (!fwd && g_mutex_groups[i].detectedFW())
      continue;
    if (fwd && !g_mutex_groups[i].detectedFW())
      continue;
    std::vector<int> vals(g_variable_domain.size() * 2);
    std::fill(vals.begin(), vals.end(), -1);
    for (int j = 0; j < g_mutex_groups[i].getFacts().size(); j++) {
      int var = g_mutex_groups[i].getFacts()[j].first;
      int val = g_mutex_groups[i].getFacts()[j].second;
      vals[2 * var + 1] = val;
    }
    MEDDLY::dd_edge cur(forest);
    stateToEVMDD(forest, vals, 0, cur);
    MEDDLY::apply(PARTIALCOMPLEMENT, cur, cur);
    MEDDLY::apply(INTERSECTIONMAX, *mutex, cur, *mutex);

    // Node limit exceeded => start new one
    if (unsigned(mutex->getNodeCount()) > transOption) {
      mutexes.push_back(mutex);
      mutex = std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(forest));
      mutex->set(-1, 0);
    }
  }
  // Add the last mutex EVMDD if it is not empty
  if (mutex->getNode() != -1)
    mutexes.push_back(mutex);
  //std::cout << "\n=> #MUTEX EVMDDs: " << mutexes.size() << std::endl;
  std::cout << "Done. => #MR's: " << mutexes.size() << std::endl;
  if (fwd)
    mutexesFWD = mutexes;
  if (!fwd)
    mutexesBWD = mutexes;
}

void symple::createInitState() {
  // create inital_state and add it to vector
  std::vector<int> initVals;
  for (int i = 0; i < g_variable_domain.size(); i++) {
    initVals.push_back(MEDDLY::DONT_CARE);
    initVals.push_back(static_cast<int>(g_initial_state->get_buffer()[i]));
  }
  // create symbolic representation
  initState = MEDDLY::dd_edge(forest);
  stateToEVMDD(forest, initVals, 0, initState);
}

void symple::createGoalState() {
// create goal_state
  const int nVars = g_variable_domain.size() * 2;
  goalState = MEDDLY::dd_edge(forest);
  std::vector<int> goalVals;
  for (int i = 0; i < nVars; i++) {
    goalVals.push_back(MEDDLY::DONT_CARE);
  }
  for (int i = 0; i < g_goal.size(); i++) {
    int var = g_goal[i].first * 2 + 1;
    int val = g_goal[i].second;
    goalVals[var] = val;
  }
  stateToEVMDD(forest, goalVals, 0, goalState);

  // Remove all states from the goal states which do not satify the mutexes
  for (int i = 0; i < mutexesFWD.size(); i++) {
    MEDDLY::apply(INTERSECTIONMAX, goalState, *mutexesFWD[i], goalState);
  }
}

void symple::forwardStep() {
  astarFwd.step(transOps, mutexesBWD);
  nStepsFwd++;
  lastStepFwd = true;
}

void symple::backwardStep() {
  astarBwd.step(transOps, mutexesFWD);
  nStepsBwd++;
  lastStepFwd = false;
}

bool symple::validatePlan(std::vector<const Operator *> &plan) {
  State cur = *g_initial_state;
  int step = 0;

  for (auto dummy_op : plan) {
    Operator op("dummy", 0);
    for (auto real_op : g_operators) {
      if (real_op.get_name() == dummy_op->get_name()) {
        op = real_op;
        break;
      }
    }

    if (!op.is_applicable(cur)) {
      return false;
    }

    cur = State(cur, op);
    step++;
  }
  return test_goal(cur);
}

void symple::dumpPlannerInformation() {
  std::string dir = "Bidirectional";
  if (searchDirection == SearchDirection::PROGRESSION)
    dir = "Progression";
  if (searchDirection == SearchDirection::REGRESSION)
    dir = "Regression";

  std::string type = "OPT";
  if (planning_type == PlanType::SAT)
    type = "SAT";
  if (planning_type == PlanType::AGL)
    type = "AGL";
  if (planning_type == PlanType::CBO)
    type = "CBO[" + std::to_string(bound) + "]";

  std::cout << "\n\t******************* Symple *******************"
  << std::endl;
  std::cout << "\t => Search Direction: " << dir << std::endl;
  std::cout << "\t => ~" << transOption << " Nodes per Transtion Relation (TR)"
  << std::endl;
  std::cout << "\t => ~" << transOption << " Nodes per Mutex Relation (MR)"
  << std::endl;
  std::cout << "\t => " << type << "-Planning" << std::endl;
  std::cout << "\t**********************************************\n"
  << std::endl;
}

void symple::dumpStepInformation(int minFwdCost, int minBwdCost) {
  std::cout << "g: " << minFwdCost + minBwdCost << " [" << minFwdCost << "+"
  << minBwdCost << "]";

  std::cout << " => Goal: "
  << (collisionCost == std::numeric_limits<int>::max() ? -1 : collisionCost);

  std::cout << " / Step: " << nStepsFwd + nStepsBwd + 1 << " [" << nStepsFwd
  << "+" << nStepsBwd << "]";

  std::cout << " / Time: " << g_timer;
  std::cout << " / Used memory: " << get_used_memory_in_kb() / 1024.00 << " MB"
  << std::endl;
}

void symple::cleanUpMeddly() {
  using namespace MEDDLY;
  expert_forest* ef = (expert_forest*) forest;
  ef->reportStats(*meddlyout, "\t",
  expert_forest::HUMAN_READABLE_MEMORY | expert_forest::BASIC_STATS
  | expert_forest::EXTRA_STATS | expert_forest::STORAGE_STATS
  | expert_forest::HOLE_MANAGER_STATS);
  operation::showAllComputeTables(*meddlyout, 3);
  cleanup();
}

static SearchEngine *_parse(OptionParser &parser) {
  if (domain_has_axioms()) {
    cerr << "Error, axioms not supported by symple (yet)." << endl;
    exit_with(EXIT_UNSUPPORTED);
  }

  parser.add_option<int>("transition", 0,
  "Option to toggle between number of operation transition.");
  parser.add_option<int>("direction", 0,
  "Option to toggle between search directions (0: bidirectional, 1: fwd, 2: bwd).");
  parser.add_option<int>("planning_type", 0,
  "Toggle between OPT(0), SAT(1), AGILE(2), CBO(3) planning");
  symple::add_options_to_parser(parser);

  Options opts = parser.parse();
  SearchEngine *policy = 0;
  if (!parser.dry_run()) {
    policy = new symple(opts);
  }
  return policy;
}

static Plugin<SearchEngine> _plugin("symple", _parse);
