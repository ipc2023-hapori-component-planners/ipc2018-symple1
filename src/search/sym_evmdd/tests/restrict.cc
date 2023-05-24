// Copyright 18.05.2017, University of Freiburg,
// Author: David Speck <speckd>.
#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

const MEDDLY::unary_opname* RESTRICT;
/**
 * This class restricts a variable of an EVMDD to a certain value.
 * Due to the interface of MEDDLY if you call this operator via
 * apply(...) a fixed variable and value gets restrict (var1 to value 0).
 * If you want to set the restriction variable call it with getOperator(...)
 * and then you can call function computeDDEdge(..) with necessary inputs.
 */
class restrictOp: public MEDDLY::unary_operation {
  public:
  void setRestrict(int var, int val) {
    this->var = var;
    this->val = val;
  }

  MEDDLY::compute_table* getComputeTable() {
    return CT;
  }

  restrictOp(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
  MEDDLY::expert_forest* res, int var, int val) :
  MEDDLY::unary_operation(code, 3, 2, arg, res), var(var), val(val) {
  }
  void discardEntry(const MEDDLY::node_handle* data) {
    argF->uncacheNode(data[2]);
    resF->uncacheNode(data[4]);
  }
  bool isStaleEntry(const MEDDLY::node_handle* data) {
    return argF->isStale(data[2]) || resF->isStale(data[4]);
  }

  //TODO(speckd): write this one correct
  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* data) const {
    strm << "[" << getName() << "(" << long(data[0]) << "): "
    << long(data[1]) << "]";
  }
  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    MEDDLY::node_handle result;
    int av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    if (result == 0) {
      ev = std::numeric_limits<int>::max();
    }
    b.set(result, ev);
  }
  protected:
  // Variable and value we restirct to the edge to.
  int var;
  int val;
  ~restrictOp() {
  }
  // HASH TABLE FUNCTIONS //
  MEDDLY::compute_table::search_key* findResult(MEDDLY::node_handle a, int &cv, // NOLINT for consistency
  MEDDLY::node_handle &c) {
    MEDDLY::compute_table::search_key* CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    // Also write var and val -> another restrict on the same edges with different
    // var and val look up wrong values!
    CTsrch->write(var);
    CTsrch->write(val);
    CTsrch->writeNH(a);
    MEDDLY::compute_table::search_result &cacheFind = CT->find(CTsrch);
    if (!cacheFind)
      return CTsrch;
    cacheFind.read(cv);
    c = resF->linkNode(cacheFind.readNH());
    doneCTkey(CTsrch);
    return 0;
  }
  void saveResult(MEDDLY::compute_table::search_key* Key, MEDDLY::node_handle a,
  int cv, MEDDLY::node_handle c) {
    // return;
    argF->cacheNode(a);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cv);
    entry.writeResultNH(resF->cacheNode(c));
    CT->addEntry();
  }
  // UTILITY FUNCTIONS //
  /**
   *
   * @param av
   * @param ap
   * @param rv
   * @param rp
   */
  void compute(int av, MEDDLY::node_handle ap, int &rv, // NOLINT for consistency
  MEDDLY::node_handle &rp) {
    // is terminal node -> nothing to do
    if (argF->isTerminalNode(ap)) {
      rv = av;
      rp = ap;
      if (ap == 0)
        rv = std::numeric_limits<int>::max();
      return;
    }

    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key* Key = findResult(ap, rv, rp);
    if (0 == Key) {
      // rv here is the residual from the node reduction of rp
      rv += av;
      if (rv < 0)
        rv = std::numeric_limits<int>::max();
      return;
    }

    // Get level information
    const int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
    true);
    if (resultLevel < var) {
      rp = resF->linkNode(ap);
      rv = av;
      MEDDLY::unpacked_node::recycle(A);
      delete Key;
      return;
    }
    // Is a node which we want to restrict to a value
   if (resultLevel == var) {

      // this link is important
	  if (A->d(val) == -1) rp = -1;
	  else rp = resF->linkNode(A->d(val));
      rv = av + A->ei(val);
      /*int tmp = av + A->ei(val);
      if (av > 0 && A->ei(val) > 0 && tmp < 0) {
        rv = std::numeric_limits<int>::max();
      } else {
        rv = tmp;
      }*/
      if (rv < 0)
        rv = std::numeric_limits<int>::max();
      MEDDLY::unpacked_node::recycle(A);
      delete Key;
      return;
    }

    // Last If: resultLevel > var
    // Initialize result
    MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(resF,
    resultLevel, resultSize);
    int ev;
    MEDDLY::node_handle ep;
    for (int i = 0; i < resultSize; i++) {
      compute(A->ei(i), A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
    }
    MEDDLY::unpacked_node::recycle(A);
    // Create a node handle...links cl to nb with cost cv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;
    // Add to CT
    // Finally add value av to current edge value
    saveResult(Key, ap, rv, rp);
    int tmp = rv + av;
    if (tmp < 0) {
      rv = std::numeric_limits<int>::max();
    } else {
      rv = tmp;
    }
  }
};
class restrict_opname: public MEDDLY::unary_opname {
  public:
  restrict_opname() :
  unary_opname("Restrict") {
  }
  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    // by default we restrict var 1 to val 0
    return new restrictOp(this, arg1, res, 1, 0);
  }
};
void initializeRestrict() {
  RESTRICT = new restrict_opname;
}
void setRestrictVarVal(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int var, int val) {
  restrictOp* resOP = reinterpret_cast<restrictOp*>(MEDDLY::getOperation(
  RESTRICT, arg, res));
  resOP->setRestrict(var, val);
  resOP = nullptr;
}

MEDDLY::compute_table* getComputeTable(MEDDLY::forest* f) {
  MEDDLY::dd_edge dummy(f);
  restrictOp* resOP = reinterpret_cast<restrictOp*>(MEDDLY::getOperation(
  RESTRICT, 0, 0));
  return resOP->getComputeTable();
}
