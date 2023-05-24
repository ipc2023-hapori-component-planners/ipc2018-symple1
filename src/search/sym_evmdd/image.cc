// Copyright 24.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef IMAGE_CPP_
#define IMAGE_CPP_

#include <limits>
#include <chrono>
#include <map>
#include <stdio.h>
#include "./image.h"
#include "meddly_extensions/userDefiniedOperators.h"

//////////////////////////////////////////////////
//////////////////// create a state //////////////
//////////////////////////////////////////////////
void createInfEdges(MEDDLY::expert_forest* f, int av, MEDDLY::node_handle ap,
int &rv, MEDDLY::node_handle &rp) { // NOLINT for consistency
  // is terminal node -> nothing to do
  if (f->isTerminalNode(ap)) {
    rv = av;
    rp = ap;
    return;
  }
  // Get level information
  const int resultLevel = f->getNodeLevel(ap);
  const int resultSize = f->getLevelSize(resultLevel);
  MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(f, ap, true);
  // Initialize result
  MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(f, resultLevel,
  resultSize);
  int ev;
  MEDDLY::node_handle ep;
  for (int i = 0; i < resultSize; i++) {
    // set this path to inf cost
    if (A->d(i) == 0) {
      nb->d_ref(i) = -1;
      nb->setEdge(i, std::numeric_limits<int>::max());
    } else {
      // recusive call to get all other paths
      createInfEdges(f, A->ei(i), A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
    }
  }
  MEDDLY::unpacked_node::recycle(A);
  // Create a node handle...links cl to nb with cost rv
  MEDDLY::node_handle cl;
  f->createReducedNode(-1, nb, rv, cl);
  rp = cl;
  rv += av;
}

void stateToEVMDD(MEDDLY::forest* f, std::vector<int> &vals, int cost,
MEDDLY::dd_edge& res) {
  assert(vals.size() == g_variable_domain.size() * 2);
  // create normal state path
  int* stateAssignment = new int[vals.size() + 1];
  stateAssignment[0] = 0;
  for (int i = 0; i < vals.size(); i++) {
    stateAssignment[i + 1] = vals.at(i);
  }
  int* assignments[] = { stateAssignment };
  int costs[] = { cost };
  f->createEdge(assignments, costs, 1, res);

  // create for every other path a link to the terminal node with cost inf
  // no need o create a lot of small nodes and combine them subsequently
  MEDDLY::node_handle result;
  MEDDLY::node_handle a = res.getNode();
  int av, ev;
  res.getEdgeValue(av);
  createInfEdges(static_cast<MEDDLY::expert_forest*>(f), av, a, ev, result);
  res.set(result, ev);

  // Clear memory
  delete stateAssignment;
}

//////////////////////////////////////////////////
////////// create operator transitions ///////////
//////////////////////////////////////////////////
void preVarEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res,
bool inverted) {
  int nVars = g_variable_domain.size() * 2;
  std::vector<int> vals;
  for (int i = 0; i < nVars; i++) {
    vals.push_back(MEDDLY::DONT_CARE);
  }
  for (int i = 0; i < op.get_prevail().size(); i++) {
    int var = op.get_prevail()[i].var;
    int val = op.get_prevail()[i].prev;
    // Unprimed version
    vals[2 * var + 1] = val;
  }
  for (int i = 0; i < op.get_pre_post().size(); i++) {
    /*if (op.get_pre_post()[i].cond.size() > 0) {
      continue;
    }*/
    int var = op.get_pre_post()[i].var;
    int val = op.get_pre_post()[i].pre;
    vals[2 * var + 1] = val;
    // Note: -1 stands for no precodition which is exactly MEDDLY::DONT_CARE -> nothing to do

    // Handles conditional effects
    // (only for axioms-> not really conditional)
    /*for (int c = 0; c < op.get_pre_post()[i].cond.size(); c++) {
     var = op.get_pre_post()[i].cond[c].var;
     val = op.get_pre_post()[i].cond[c].prev;
     vals[2 * var + 1] = val;
     }*/
  }
  if (!inverted)
    stateToEVMDD(f, vals, 0, res);
  if (inverted) {
    stateToEVMDD(f, vals, 0, res);
    MEDDLY::apply(PARTIALCOMPLEMENT, res, res);
  }
}

void effVarEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res) {
  int nVars = g_variable_domain.size() * 2;
  std::vector<int> vals;
  for (int i = 0; i < nVars; i++) {
    vals.push_back(MEDDLY::DONT_CARE);
  }
  for (int i = 0; i < op.get_pre_post().size(); i++) {
    if (op.get_pre_post()[i].cond.size() > 0) {
      continue;
    }
    int var = op.get_pre_post()[i].var;
    int val = op.get_pre_post()[i].post;
    // Primed version
    vals[2 * var] = val;
  }
  stateToEVMDD(f, vals, 0, res);
}

void condEffVarEdge(MEDDLY::forest* f, std::vector<PrePost> condV,
MEDDLY::dd_edge& res) {
  MEDDLY::dd_edge fires(f);
  fires.set(-1, std::numeric_limits<int>::max());
  /// Conditional effect fires:
  for (int i = 0; i < condV.size(); i++) {
    PrePost pp = condV[i];
    int nVars = g_variable_domain.size() * 2;
    std::vector<int> vals;
    for (int i = 0; i < nVars; i++) {
      vals.push_back(MEDDLY::DONT_CARE);
    }
    // Condition
    for (int c = 0; c < pp.cond.size(); c++) {
      int var = pp.cond[c].var;
      int val = pp.cond[c].prev;
      vals[2 * var + 1] = val;
    }
    // Effect
    int var = pp.var;
    int val = pp.post;
    vals[2 * var] = val;

    MEDDLY::dd_edge tmp(f);
    stateToEVMDD(f, vals, 0, tmp);
    MEDDLY::apply(UNIONMIN, fires, tmp, fires);
  }

  /// Conditonal effect does not fire;
  MEDDLY::dd_edge notFires(f);
  notFires.set(-1, std::numeric_limits<int>::max());
  /// Conditional effect fires:
  for (int i = 0; i < condV.size(); i++) {
    PrePost pp = condV[i];
    int nVars = g_variable_domain.size() * 2;
    std::vector<int> vals;
    for (int i = 0; i < nVars; i++) {
      vals.push_back(MEDDLY::DONT_CARE);
    }
    // Condition
    for (int c = 0; c < pp.cond.size(); c++) {
      int var = pp.cond[c].var;
      int val = pp.cond[c].prev;
      vals[2 * var + 1] = val;
    }
    MEDDLY::dd_edge tmp(f);
    stateToEVMDD(f, vals, 0, tmp);
    MEDDLY::apply(UNIONMIN, notFires, tmp, notFires);
  }
  MEDDLY::apply(PARTIALCOMPLEMENT, notFires, notFires);

  /// frame
  int var = condV[0].var;
  MEDDLY::dd_edge frame(f);
  MEDDLY::node_handle result = -1;
  valEqualsValEdge(static_cast<MEDDLY::expert_forest*>(f), 2 * var + 1, 2 * var,
  result);
  frame.set(result, 0);

  /// combine pre, eff and frame
  MEDDLY::apply(INTERSECTIONMAX, notFires, frame, notFires);
  MEDDLY::apply(UNIONMIN, fires, notFires, res);
}

void condEffVarsEdge(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res) {
  // empty conditional effect => return blank
  if (!op.has_conditional_effects()) {
    res.set(-1, 0);
    return;
  }

  // We sort it according to effect variables
  std::map<int, std::vector<PrePost>> cEffVars;
  for (int i = 0; i < op.get_pre_post().size(); i++) {
    if (op.get_pre_post()[i].cond.size() > 0) {
      // We have a conditional effect:
      int var = op.get_pre_post()[i].var;

      if (cEffVars.find(var) == cEffVars.end()) {
        cEffVars[var] = std::vector<PrePost>();
      }
      cEffVars[var].push_back(op.get_pre_post()[i]);
    }
  }

  res.set(-1, 0);
  for (auto elem : cEffVars) {
    //std::cout << g_variable_name[elem.first] << " " << elem.second.size()
    //<< std::endl;
    MEDDLY::dd_edge curCondEffect(f);
    condEffVarEdge(f, elem.second, curCondEffect);
    MEDDLY::apply(INTERSECTIONMAX, res, curCondEffect, res);
  }

  //std::cout << "Full one: " << std::endl;
  //MEDDLY::FILE_output out(stdout);
  //res.show(out, 3);
}

void valEqualsValEdge(MEDDLY::expert_forest* f, int var1, int var2,
MEDDLY::node_handle &r) {
  assert(var1 > var2);

  // Initialize result (+ 1 beacuse root is level 0)
  const int resultLevel1 = var1 + 1;
  const int resultLevel2 = var2 + 1;
  const int resultSize1 = f->getLevelSize(resultLevel1);
  const int resultSize2 = f->getLevelSize(resultLevel2);

  MEDDLY::unpacked_node* nb1 = MEDDLY::unpacked_node::newFull(f, resultLevel1,
  resultSize1);
  MEDDLY::node_handle cur_var2;
  int dummy;
  for (int i = 0; i < resultSize1; i++) {
    MEDDLY::unpacked_node* nb2 = MEDDLY::unpacked_node::newFull(f, resultLevel2,
    resultSize2);
    for (int j = 0; j < resultSize2; j++) {
      // 0 cost path!
      if (i == j) {
        nb2->d_ref(j) = -1;
        nb2->setEdge(j, 0);
      } else {
        nb2->d_ref(j) = -1;
        nb2->setEdge(j, std::numeric_limits<int>::max());
      }
    }
    f->createReducedNode(-1, nb2, dummy, cur_var2);
    nb1->d_ref(i) = cur_var2;
    nb1->setEdge(i, 0);
    // cleanup
  }
  f->createReducedNode(-1, nb1, dummy, r);
}

void valsEqualsPrimedValsEdge(MEDDLY::forest* f, Operator& op,
MEDDLY::dd_edge& res) {
  // TODO(speckd): is there an more efficient way to find the eff vars?
  std::vector<int> noEffVars;
  for (int i = 0; i < g_variable_domain.size(); i++) {
    bool varIsInEff = false;
    for (int j = 0; j < op.get_pre_post().size(); j++) {
      if (op.get_pre_post()[j].var == i) {
        varIsInEff = true;
        break;
      }
    }
    if (!varIsInEff) {
      noEffVars.push_back(i);
    }
  }

  // if all variables are effvars we return a terminal node with 0 cost
  MEDDLY::node_handle result = -1;
  int cost = 0;
  res.set(result, cost);
  // create assignments and combine the edges with each other
  for (int i = 0; i < noEffVars.size(); i++) {
    MEDDLY::dd_edge tmp(f);
    valEqualsValEdge(static_cast<MEDDLY::expert_forest*>(f),
    noEffVars[i] * 2 + 1, noEffVars[i] * 2, result);
    tmp.set(result, cost);
    MEDDLY::apply(INTERSECTIONMAX, res, tmp, res);
  }
}

void createOpTrans(MEDDLY::forest* f, Operator& op, MEDDLY::dd_edge& res,
bool withCost) {
  // pre vars
  MEDDLY::dd_edge preVar(f);
  preVarEdge(f, op, preVar);
  // eff vars
  MEDDLY::dd_edge effVar(f);
  effVarEdge(f, op, effVar);
  // vars not effected stay the same
  MEDDLY::dd_edge valsEqualPrimedVals(f);
  valsEqualsPrimedValsEdge(f, op, valsEqualPrimedVals);
  // create conditional effect
  MEDDLY::dd_edge condEffects(f);
  condEffVarsEdge(f, op, condEffects);

  // combine the resulting one
  MEDDLY::apply(INTERSECTIONMAX, preVar, effVar, res);
  MEDDLY::apply(INTERSECTIONMAX, res, valsEqualPrimedVals, res);
  MEDDLY::apply(INTERSECTIONMAX, res, condEffects, res);

  if (withCost)
    MEDDLY::apply(MEDDLY::PLUS, res, *op.get_SDAC_cost(), res);
}

//////////////////////////////////////////////////
///////////////// File Handling //////////////////
//////////////////////////////////////////////////

bool writeEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& edge) {
  FILE * pFile;
  pFile = fopen((fileName).c_str(), "w");
  MEDDLY::FILE_output fileout(pFile);
  f->writeEdges(fileout, &edge, 1);
  fclose(pFile);
  return true;
}

bool readEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& res) {
  FILE * pFile;
  pFile = fopen(fileName.c_str(), "r");
  MEDDLY::FILE_input fileout(pFile);
  f->readEdges(fileout, &res, 1);
  fclose(pFile);
  return true;
}

bool deleteFile(std::string fileName) {
  if (remove(fileName.c_str()) != 0) {
    perror(("Error deleting file: " + fileName).c_str());
    return false;
  }
  return true;
}

#endif  // IMAGE_CPP_
