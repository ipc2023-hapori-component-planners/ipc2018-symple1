// Copyright 19.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <algorithm>
#include "aStarBwd.h"
#include "../image.h"

void AStarBwd::image(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res) {
  MEDDLY::dd_edge primedStateSet(forest);
  MEDDLY::apply(SWAPTOPRIMEDVAR, stateSet, primedStateSet);
  MEDDLY::apply(MEDDLY::PLUS, primedStateSet, transOp, res);
  for (int i = 1; i < g_variable_domain.size() * 2 + 1; i = i + 2) {
    setMinVar(res, res, i);
    MEDDLY::apply(MINVAR, res, res);
  }
}

void AStarBwd::imageInv(MEDDLY::dd_edge& stateSet, MEDDLY::dd_edge& transOp,
MEDDLY::dd_edge &res) {
  // No plus => we want to preserve the costs here
  MEDDLY::apply(INTERSECTIONMAX, stateSet, transOp, res);
  for (int i = 2; i < g_variable_domain.size() * 2 + 1; i = i + 2) {
    setMinVar(res, res, i);
    MEDDLY::apply(MINVAR, res, res);
  }
  MEDDLY::apply(SWAPVARPRIMED, res, res);
}

bool AStarBwd::reconstructSolution(
MEDDLY::dd_edge& target, std::vector<const Operator *>& plan) {
  //std::cout << "BWD REConstruction" << std::endl;
  std::vector<const Operator *> planBwd;
  bool valid = SymSearch::reconstructSolution(target, planBwd);
  for (int i = planBwd.size() - 1; i >= 0; i--)
    plan.push_back(planBwd[i]);
  //plan.insert(plan.end(), planBwd.begin(), planBwd.end());
  return valid;
}

int AStarBwd::selectAnyState(MEDDLY::dd_edge& targetState) {
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

Operator* AStarBwd::reconstructPlanStep(MEDDLY::dd_edge& targetState) {
  MEDDLY::FILE_output out(stdout);
  int costAfter = selectAnyState(targetState);
  //targetState.show(out, 3);
  for (int op_i = 0; op_i < g_operators.size(); op_i++) {
    MEDDLY::dd_edge successors(forest);
    imageInv(targetState, *g_operators[op_i].get_op_evmdd(), successors);

    for (int c_i = 0; c_i < curClosedList; c_i++) {
      MEDDLY::dd_edge tmp(forest);
      MEDDLY::apply(INTERSECTIONMAX, successors, *closedLists[c_i], tmp);

      // We need to add the cost to target and then check if it fits
      MEDDLY::dd_edge costDummy(forest);
      MEDDLY::apply(MEDDLY::PLUS, *g_operators[op_i].get_SDAC_cost(), targetState, costDummy);
      int cost_tmp = minVal(costDummy);
      costDummy.set(-1, cost_tmp);
      MEDDLY::apply(MEDDLY::PLUS, tmp, costDummy, costDummy);
      //std::cout << minVal(costDummy) << ", " << costAfter << std::endl;

      // Cost Fits
      //std::cout << "Cost after: " << costAfter << ", minval" << minVal(tmp) << std::endl;
      if (minVal(costDummy) == costAfter) {
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
        //std::cout << "\n" << curClosedList << std::endl;
        curClosedList = c_i;
        return dummy;
      }
    }
  }
  std::cout << "null operation - something went wrong - no op found!"
  << std::endl;
  return nullptr;
}


// Old code used for paper submission
//Operator* AStarBwd::reconstructPlanStep2(
//std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans,
//MEDDLY::dd_edge& statesBefore, MEDDLY::dd_edge& targetState) {
//  std::vector<int> stateValsFull = minPath(targetState);
//  int costAfter = minVal(targetState);
//  std::vector<int> stateVals;
//
//  for (int i = 0; i < stateValsFull.size(); i++) {
//    // Value don't care => we set it to a value which always exists
//    if (i % 2 == 1 && stateValsFull[i] == -1) {
//      stateValsFull[i] = 0;
//    }
//    if (i % 2 == 1) {
//      stateVals.push_back(stateValsFull[i]);
//    }
//
//  }
//
//  // We generated one explicit state and use it from now on
//  State target(stateVals);
//  stateToEVMDD(forest, stateValsFull, costAfter, targetState);
//  //MEDDLY::FILE_output out(stdout);
//  //targetState.show(out, 3);
//
//  // We remove the target states
//  MEDDLY::dd_edge comp(forest);
//  MEDDLY::apply(PARTIALCOMPLEMENT, targetState, comp);
//  MEDDLY::apply(INTERSECTIONMAX, closed, comp, closed);
//
//  /// TARGET STATE GENERATION
//  // create cur state
//  MEDDLY::dd_edge sStarPrimed(forest);
//  setInfintiyEdge(sStarPrimed);
//  for (int i = 0; i < opTrans.size(); i++) {
//    MEDDLY::dd_edge cur(forest);
//    imageInv(targetState, *opTrans[i], cur);
//    MEDDLY::apply(UNIONMIN, sStarPrimed, cur, sStarPrimed);
//  }
//  MEDDLY::apply(INTERSECTIONMAX, sStarPrimed, statesBefore, sStarPrimed);
//
//  // just a helper!
//  int targetLayer = -1;
//
//  //sStarPrimed.show(out, 3);
//  for (int op_i = 0; op_i < g_operators.size(); op_i++) {
//
//    Operator op = g_operators[op_i];
//
//    /// Our goal state reachable with that cost!!!! really important otherwise the cost does not fit.
//    /// Took me hours to find that bug :^)
//    /// We search the cost of the current op and add it then to the states from before
//    MEDDLY::dd_edge cost_tmp(forest);
//    MEDDLY::apply(MEDDLY::PLUS, targetState, *op.get_SDAC_cost(), cost_tmp);
//    cost_tmp.set(-1, minVal(cost_tmp) - minVal(targetState));
//
//    MEDDLY::dd_edge costReachable(forest);
//    MEDDLY::apply(MEDDLY::PLUS, statesBefore, cost_tmp, costReachable);
//
//    //preCondition.show(out, 3);
//
//    // Build the intersectionmax -> such that sStar only has the correct
//    // if it is reachable with this cost
//
//    MEDDLY::apply(INTERSECTIONMAX, costReachable, sStarPrimed, costReachable);
//    //std::cout << g_operators[op_i].get_name() << " :" << minVal(costReachable)
//    //<< std::endl;
//
//    // We need only states which really have the same effects!
//    MEDDLY::dd_edge eff(forest);
//    effVarEdge(forest, op, eff);
//    MEDDLY::apply(SWAPVARPRIMED, eff, eff);
//    MEDDLY::apply(INTERSECTIONMAX, eff, costReachable, costReachable);
//
//    /// check how the result state look like...we should replace it with the normal state
//    /// function and apply an operator => thats basically what we do here...
//    if (op.has_conditional_effects()) {
//      MEDDLY::dd_edge cond_removal(forest);
//      cond_removal.set(-1, 0);
//
//      for (auto &pp : op.get_pre_post()) {
//        if (pp.cond.size() == 0)
//          continue;
//
//        // Here we check if all! conditional prev are fitting with the target state
//        // If yes we know that the effect trigger!
//        bool allfitting = true;
//        for (auto &ce : pp.cond) {
//          if (static_cast<int>(target.get_buffer()[ce.var]) != ce.prev) {
//            allfitting = false;
//            break;
//          }
//        }
//        if (!allfitting)
//          continue;
//
//        // now we add the conditional effects to the cond removal because it triggers!
//        int nVars = g_variable_domain.size() * 2;
//        std::vector<int> vals;
//        for (int i = 0; i < nVars; i++) {
//          vals.push_back(MEDDLY::DONT_CARE);
//        }
//        vals[2 * pp.var + 1] = pp.post;
//        MEDDLY::dd_edge cur_cond(forest);
//        stateToEVMDD(forest, vals, 0, cur_cond);
//
//        // Here they have to trigger! INTERSECTIONMAX usage
//        MEDDLY::apply(INTERSECTIONMAX, cond_removal, cur_cond, cond_removal);
//
//      }
//      // Now we know how the state has to look like...
//      MEDDLY::apply(INTERSECTIONMAX, costReachable, cond_removal,
//      costReachable);
//
//      /// Old hack: We do an inverse image...with the cond effect
//      /*createOpTrans(forest, op, op_e);
//       imageInv(targetState, op_e, op_e);
//       MEDDLY::apply(INTERSECTIONMAX, costReachable, op_e, costReachable);*/
//    }
//
//    vector<int> newStateVals = stateValsFull;
//    stateValsFull = minPath(costReachable);
//    stateVals.clear();
//
//    //costReachable.show(out, 3);
//
//    // get min path of cur state
//    for (int i = 0; i < stateValsFull.size(); i++) {
//      // Value don't care => we set it to a value which always exists
//      if (i % 2 == 1 && stateValsFull[i] == -1) {
//        stateValsFull[i] = 0;
//      }
//      if (i % 2 == 1) {
//        stateVals.push_back(stateValsFull[i]);
//      }
//    }
//
//    State curState(stateVals);
//
//    stateValsFull.insert(stateValsFull.begin(), 0);
//
//    int costBefore = evaluatePath(statesBefore, stateValsFull);
//    stateValsFull.erase(stateValsFull.begin());
//
//    if (!g_operators[op_i].is_applicable(target)) {
//      //std::cout << g_operators[op_i].get_name() + " not app" << std::endl;
//      continue;
//    }
//    if (minVal(costReachable) != costAfter) {
//      /*std::cout << g_operators[op_i].get_name() + " cost not fitting "
//       << minVal(costReachable) << " != " << costAfter << " -- before: "
//       << costBefore << std::endl;*/
//      continue;
//    }
//    State s(target, g_operators[op_i]);
//    if (s == curState) {
//      int costOfOperator = costAfter - costBefore;
//
//      /*std::cout << costAfter << " - " << evaluatePath(*g_operators[op_i].get_SDAC_cost(), stateValsFull) << " = "
//       << costBefore << std::endl;
//       std::cout << g_operators[op_i].get_name() << " selected" << std::endl;*/
//
//      // Important to minimize cost of targetState
//      // Here we need to swap target and current state...
//      MEDDLY::dd_edge newTargetState(forest);
//      stateToEVMDD(forest, stateValsFull, costBefore, newTargetState);
//
//      Operator* dummy = new Operator(g_operators[op_i].get_name(),
//      costOfOperator);
//      if (costOfOperator > 0) {
//        targetState = newTargetState;
//        return dummy;
//      } else {
//        // we search for a state in a lower Layer (same cost)
//        auto costLayers = closedLayers[costBefore];
//        if (targetLayer == -1) {
//          for (int j = 0; j < costLayers.size(); j++) {
//            MEDDLY::dd_edge res(forest);
//            MEDDLY::apply(INTERSECTIONMAX, targetState, *costLayers[j], res);
//            if (minVal(res) == costBefore) {
//              targetLayer = j;
//              //std::cout << "Target Layer is: " << j << std::endl;
//              break;
//            }
//          }
//        }
//
//        // First reconstruction step => goal still in open...
//        if (targetLayer == -1)
//          targetLayer = costLayers.size();
//
//        // If we find a state in a smaller Layer we can return it!
//        for (int j = 0; j < targetLayer; j++) {
//          MEDDLY::dd_edge res(forest);
//          MEDDLY::apply(INTERSECTIONMAX, newTargetState, *costLayers[j], res);
//          if (minVal(res) == costBefore) {
//            //std::cout << j << " vs. " << targetLayer << std::endl;
//            targetState = newTargetState;
//            return dummy;
//            break;
//          }
//        }
//      }
//    }
//  }
//  std::cout << "null operation - something went wrong - no op found!"
//  << std::endl;
//  return nullptr;
//}

