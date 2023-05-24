// Copyright 23.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

int timeMinVar;
const MEDDLY::unary_opname* MINVAR;

class minVar: public MEDDLY::unary_operation {
  public:
  void setMinVar(int var) {
    this->var = var;
  }


  minVar(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
  MEDDLY::expert_forest* res, int var) :
  MEDDLY::unary_operation(code, 0, 0, arg, res), var(var) {
  }

  void discardEntry(const MEDDLY::node_handle* /*data*/) {

  }

  bool isStaleEntry(const MEDDLY::node_handle* /*data*/) {
    return false;
  }

  void showEntry(MEDDLY::output& /*strm*/, const MEDDLY::node_handle* /*data*/) const {
  }

  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    // TODO(speckd): implement a tree to minimize intersectionMAX!
    /*int limit = argF->getDomain()->getVariableBound(var, false);
    // Note: if we directly use b we overwrite a in case of a = b
    MEDDLY::dd_edge b_curVal(argF);
    MEDDLY::dd_edge tmp(argF);
    setInfintiyEdge(tmp);
    for (int i = 0; i < limit; i++) {
      MEDDLY::dd_edge curVal(a.getForest());
      setRestrictVarVal(a, curVal, var, i);
      MEDDLY::apply(RESTRICT, a, curVal);
      MEDDLY::apply(UNIONMIN, curVal, tmp, tmp);
      curVal.clear();
    }
    b = tmp;*/

    /// Tree implementation seems to be slower!!!
    int limit = argF->getDomain()->getVariableBound(var, false);
    std::vector<std::shared_ptr<MEDDLY::dd_edge>> tree;
     for (int i = 0; i < limit; i++) {
     tree.push_back(
     std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(b.getForest())));
     setRestrictVarVal(a, *tree.at(i), var, i);
     MEDDLY::apply(RESTRICT, a, *tree.at(i));
     }
     while (tree.size() > 1) {
     std::shared_ptr<MEDDLY::dd_edge> cur1 = tree.back();
     tree.pop_back();
     std::shared_ptr<MEDDLY::dd_edge> cur2 = tree.back();
     tree.pop_back();
     MEDDLY::apply(UNIONMIN, *cur1, *cur2, *cur1);
     tree.insert(tree.begin(), cur1);
     //printMemoryState(argF);
     }
     b = *tree.at(0);

    /*MEDDLY::dd_edge a0(a.getForest());
     MEDDLY::dd_edge a1(a.getForest());
     setRestrictVarVal(a, a0, var, 0);
     MEDDLY::apply(RESTRICT, a, a0);
     setRestrictVarVal(a, a1, var, 1);
     MEDDLY::apply(RESTRICT, a, a1);
     MEDDLY::apply(UNIONMIN, a0, a1, b);*/
  }

  protected:

  // Variable we minimize.
  int var;

  ~minVar() {
  }
};

// UTILITY FUNCTIONS //

class minVar_opname: public MEDDLY::unary_opname {
  public:

  minVar_opname() :
  unary_opname("MinVar") {
  }

  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    // by default we minimize var 1
    return new minVar(this, arg1, res, 1);
  }
};

void initializeMinVar() {
  MINVAR = new minVar_opname;
}

void setMinVar(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int var) {
  minVar* minOP = reinterpret_cast<minVar*>(MEDDLY::getOperation(MINVAR, arg,
  res));
  minOP->setMinVar(var);
  minOP = nullptr;
}
