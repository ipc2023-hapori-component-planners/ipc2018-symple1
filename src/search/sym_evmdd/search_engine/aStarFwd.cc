// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <algorithm>
#include "aStarFwd.h"
#include "../image.h"

void AStarFwd::image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res) {
  MEDDLY::apply(MEDDLY::PLUS, stateSet, transOp, res);
  for (int i = 2; i < g_variable_domain.size() * 2 + 1; i = i + 2) {
    setMinVar(res, res, i);
    MEDDLY::apply(MINVAR, res, res);
  }
  MEDDLY::apply(SWAPVARPRIMED, res, res);
}

void AStarFwd::imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res) {
  MEDDLY::dd_edge primedStateSet(forest);
  MEDDLY::apply(SWAPTOPRIMEDVAR, stateSet, primedStateSet);
  MEDDLY::apply(INTERSECTIONMAX, primedStateSet, transOp, res);
  for (int i = 1; i < g_variable_domain.size() * 2 + 1; i = i + 2) {
    setMinVar(res, res, i);
    MEDDLY::apply(MINVAR, res, res);
  }
}

int AStarFwd::selectAnyState(MEDDLY::dd_edge& targetState) {
  std::vector<int> stateValsFull = minPath(targetState);
  int costAfter = minVal(targetState);
  std::vector<int> stateVals;

  for (int i = 0; i < stateValsFull.size(); i++) {
    // Value don't care => we set it to a value which always exists
    if (i % 2 == 1 && stateValsFull[i] == -1) {
      stateValsFull[i] = 0;
    }
    if (i % 2 == 1) {
      stateVals.push_back(stateValsFull[i]);
    }

  }

  // We generated one explicit state and use it from now on
  stateToEVMDD(forest, stateValsFull, 0, targetState);
  return costAfter;
}

Operator* AStarFwd::reconstructPlanStep(MEDDLY::dd_edge& targetState) {
  MEDDLY::FILE_output out(stdout);
  int costAfter = selectAnyState(targetState);
  //targetState.show(out, 3);
  for (int op_i = 0; op_i < g_operators.size(); op_i++) {
    MEDDLY::dd_edge predecessors(forest);
    imageInv(targetState, *g_operators[op_i].get_op_evmdd(), predecessors);
    /*if (minVal(predecessors) == std::numeric_limits<int>::max())
      continue;*/
    //predecessors.show(out, 3);
    //TODO(speckd): add guard such that it only searches for states with lower i
    // Not really necessary but if something goes wrong...
    for (int c_i = 0; c_i < curClosedList; c_i++) {
      MEDDLY::dd_edge tmp(forest);
      MEDDLY::apply(INTERSECTIONMAX, predecessors, *closedLists[c_i], tmp);
      MEDDLY::apply(MEDDLY::PLUS, tmp, *g_operators[op_i].get_SDAC_cost(), tmp);
      // Fits!
      //std::cout << "Cost after: " << costAfter << ", minval" << minVal(tmp) << std::endl;
      if (minVal(tmp) == costAfter) {
        //tmp.show(out, 3);
        MEDDLY::apply(MINSTATES, tmp, tmp);
        // add cost to it
        selectAnyState(tmp);
        MEDDLY::apply(INTERSECTIONMAX, tmp, *closedLists[c_i], targetState);

        // create dummy op with correct cost
        Operator* dummy = new Operator(g_operators[op_i].get_name(),
        costAfter - minVal(targetState));
        //targetState.show(out, 3);
        //std::cout << "-----------------------" << std::endl;
        curClosedList = c_i;
        return dummy;
      }
    }
  }
  std::cout << "null operation - something went wrong - no op found!"
  << std::endl;
  return nullptr;
}

//TODO(speckd): Man we need to refactor this => ...
/*Operator* AStarFwd::reconstructPlanStep2(
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans,
MEDDLY::dd_edge& statesBefore, MEDDLY::dd_edge& targetState) {
  std::vector<int> stateValsFull = minPath(targetState);
  int costAfter = minVal(targetState);
  std::vector<int> stateVals;

  for (int i = 0; i < stateValsFull.size(); i++) {
    // Value don't care => we set it to a value which always exists
    if (i % 2 == 1 && stateValsFull[i] == -1) {
      stateValsFull[i] = 0;
    }
    if (i % 2 == 1) {
      stateVals.push_back(stateValsFull[i]);
    }

  }

  // We generated one explicit state and use it from now on
  State target(stateVals);
  stateToEVMDD(forest, stateValsFull, costAfter, targetState);

  // We remove the target states => against empty operators with cost 0!
  MEDDLY::dd_edge comp(forest);
  MEDDLY::apply(PARTIALCOMPLEMENT, targetState, comp);
  MEDDLY::apply(INTERSECTIONMAX, closed, comp, closed);

  /// TARGET STATE GENERATION

  // create cur state
  MEDDLY::dd_edge sStarPrimed(forest);
  setInfintiyEdge(sStarPrimed);
  for (int i = 0; i < opTrans.size(); i++) {
    MEDDLY::dd_edge cur(forest);
    imageInv(targetState, *opTrans[i], cur);
    MEDDLY::apply(UNIONMIN, sStarPrimed, cur, sStarPrimed);
  }
  MEDDLY::apply(INTERSECTIONMAX, sStarPrimed, statesBefore, sStarPrimed);

  // We need to sort the operators according to there application cost due to 0 cost ops!
  //using namespace std::placeholders;
  if (!domain_has_sdac()) {
    std::sort(g_operators.begin(), g_operators.end(),
    std::bind(higherCost, std::placeholders::_1, std::placeholders::_2));
  }

  //sStarPrimed.show(out, 3);

  // just a helper!
  int targetLayer = -1;

  for (int op_i = 0; op_i < g_operators.size(); op_i++) {

    Operator op = g_operators[op_i];

    // All states rechable with this cost!
    MEDDLY::dd_edge costReachable(forest);
    MEDDLY::apply(MEDDLY::PLUS, statesBefore, *op.get_SDAC_cost(),
    costReachable);

    //preCondition.show(out, 3);

    // Build the intersectionmax -> such that sStar only has the correct
    // if it is reachable with this cost

    MEDDLY::apply(INTERSECTIONMAX, costReachable, sStarPrimed, costReachable);
    //std::cout << g_operators[op_i].get_name() << " :" << minVal(costReachable)
    //<< std::endl;

    // We need only states which are really applicable!
    MEDDLY::dd_edge preCondition(forest);
    preVarEdge(forest, op, preCondition);

    MEDDLY::apply(INTERSECTIONMAX, preCondition, costReachable, costReachable);

    /// Remove all pre facts where the effect differs from the target state
    /// -- Similar for backwards just that the remove all eff vars where the pre triggers
    MEDDLY::dd_edge op_e(forest);
    if (op.has_conditional_effects()) {
      MEDDLY::dd_edge cond_removal(forest);
      cond_removal.set(-1, std::numeric_limits<int>::max());

      for (auto &pp : op.get_pre_post()) {
        if (pp.cond.size() == 0)
          continue;

        if (static_cast<int>(target.get_buffer()[pp.var]) != pp.post) {

          // now we add the conditional effects to the cond removal
          int nVars = g_variable_domain.size() * 2;
          std::vector<int> vals;
          for (int i = 0; i < nVars; i++) {
            vals.push_back(MEDDLY::DONT_CARE);
          }
          for (auto &ce : pp.cond) {
            vals[2 * ce.var + 1] = ce.prev;
          }
          MEDDLY::dd_edge cur_cond(forest);
          stateToEVMDD(forest, vals, 0, cur_cond);
          MEDDLY::apply(UNIONMIN, cond_removal, cur_cond, cond_removal);
        }

      }
      MEDDLY::apply(PARTIALCOMPLEMENT, cond_removal, cond_removal);
      MEDDLY::apply(INTERSECTIONMAX, costReachable, cond_removal,
      costReachable);

      /// Old hack which uses an inversed image...but we have to build up
      /// a op transition which we want to avoid
      *//*std::cout << "func_name" << std::endl;
       MEDDLY::FILE_output out(stdout);
       createOpTrans(forest, op, op_e);
       imageInv(targetState, op_e, op_e);
       MEDDLY::apply(INTERSECTIONMAX, costReachable, op_e, costReachable);*//*
    }

    stateValsFull = minPath(costReachable);
    stateVals.clear();

    //costReachable.show(out, 3);

    // get min path of cur state
    for (int i = 0; i < stateValsFull.size(); i++) {
      // Value don't care => we set it to a value which always exists
      if (i % 2 == 1 && stateValsFull[i] == -1) {
        stateValsFull[i] = 0;
      }
      if (i % 2 == 1) {
        stateVals.push_back(stateValsFull[i]);
      }
    }

    State curState(stateVals);

    stateValsFull.insert(stateValsFull.begin(), 0);

    int costBefore = evaluatePath(statesBefore, stateValsFull);
    stateValsFull.erase(stateValsFull.begin());

    if (!g_operators[op_i].is_applicable(curState)) {
      //std::cout << g_operators[op_i].get_name() + " not app" << std::endl;
      continue;
    }
    if (minVal(costReachable) != costAfter) {
      *//*std::cout << g_operators[op_i].get_name() + " cost not fitting "
       << minVal(costReachable) << " != " << costAfter << " -- before: "
       << costBefore << std::endl;*//*
      continue;
    }
    State s(curState, g_operators[op_i]);
    if (s == target) {
      int costOfOperator = costAfter - costBefore;

      *//*std::cout << costAfter << " - " << g_operators[op_i].get_cost() << " = "
       << costBefore << std::endl;*//*

      // Important to minimize cost of targetState
      MEDDLY::dd_edge newTargetState(forest);
      stateToEVMDD(forest, stateValsFull, costBefore, newTargetState);

      Operator* dummy = new Operator(g_operators[op_i].get_name(),
      costOfOperator);
      if (costOfOperator > 0) {
        targetState = newTargetState;
        return dummy;
      } else {
        // we search for a state in a lower Layer (same cost)
        auto costLayers = closedLayers[costBefore];
        if (targetLayer == -1) {
          for (int j = 0; j < costLayers.size(); j++) {
            MEDDLY::dd_edge res(forest);
            MEDDLY::apply(INTERSECTIONMAX, targetState, *costLayers[j], res);
            if (minVal(res) == costBefore) {
              targetLayer = j;
              break;
            }
          }
        }

        // First reconstruction step => goal still in open...
        if (targetLayer == -1)
          targetLayer = costLayers.size();

        // If we find a state in a smaller Layer we can return it!
        for (int j = 0; j < targetLayer; j++) {
          MEDDLY::dd_edge res(forest);
          MEDDLY::apply(INTERSECTIONMAX, newTargetState, *costLayers[j], res);
          if (minVal(res) == costBefore) {
            //std::cout << j << " vs. " << targetLayer << std::endl;
            targetState = newTargetState;
            return dummy;
          }
        }
        //std::cout << "none" << " vs. " << targetLayer << std::endl;
      }
    }
  }
  std::cout << "null operation - something went wrong - no op found!"
  << std::endl;
  return nullptr;
}*/

