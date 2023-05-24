// Copyright 23.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

const MEDDLY::unary_opname* SWAPVARPRIMED;

class swapVarPrimed: public MEDDLY::unary_operation {
  public:
  swapVarPrimed(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
  MEDDLY::expert_forest* res) :
  MEDDLY::unary_operation(code, 1, 2, arg, res) {
  }

  void discardEntry(const MEDDLY::node_handle* data) {
    argF->uncacheNode(data[0]);
    resF->uncacheNode(data[2]);
  }

  bool isStaleEntry(const MEDDLY::node_handle* data) {
    return argF->isStale(data[0]) || resF->isStale(data[2]);
  }

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* data) const {
    strm << "[" << getName() << "(" << long(data[0]) << "): "
    << long(data[1]) << "]";
  }

  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    // Copy tree and add to all uneven levels +1 assuming var oder
    // (a, a' b, b', ..., n, n')
    MEDDLY::node_handle result;
    int av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    b.set(result, ev);
  }

  protected:
  ~swapVarPrimed() {
  }

  // HASH TABLE FUNCTIONS //
  MEDDLY::compute_table::search_key* findResult(MEDDLY::node_handle a, int &cv, // NOLINT for consistency
  MEDDLY::node_handle &c) {
    MEDDLY::compute_table::search_key* CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
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
    argF->cacheNode(a);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cv);
    entry.writeResultNH(resF->cacheNode(c));
    CT->addEntry();
  }

  // UTILITY FUNCTIONS //

  //TODO(speckd): add compute table functionality such that nodes are not marked for
  // garbage collection
  void compute(int av, MEDDLY::node_handle ap, int &rv, // NOLINT for consistency
  MEDDLY::node_handle &rp) {
    if (argF->isTerminalNode(ap)) {
      rv = av;
      rp = ap;
      return;
    }

    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key* Key = findResult(ap, rv, rp);
    if (0 == Key) {
      // rv here is the residual from the node reduction of rp
      rv += av;
      return;
    }

    // Get level information
    int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);

    // is primed var -> push one level up to unprimed version
    if (resultLevel % 2 == 1) {
      resultLevel += 1;
    }

    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
    true);

    // Initialize result
    MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(resF,
    resultLevel, resultSize);

    // Recursive call
    int ev;
    MEDDLY::node_handle ep;
    for (int i = 0; i < resultSize; i++) {
      compute(A->ei(i), A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
    }

    MEDDLY::unpacked_node::recycle(A);

    // Create a node handle...links cl to nb with cost rv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;

    // Add to CT
    saveResult(Key, ap, rv, rp);

    // Finally add value av to current edge value
    if (rv > 0 && av > 0 && rv + av < 0) {
      rv = std::numeric_limits<int>::max();
    } else {
      rv += av;
    }
  }
};

class swapVarPrimed_opname: public MEDDLY::unary_opname {
  public:

  swapVarPrimed_opname() :
  unary_opname("SwapVarPrimed") {
  }

  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    return new swapVarPrimed(this, arg1, res);
  }
};

void initializeSwapVarPrimed() {
  SWAPVARPRIMED = new swapVarPrimed_opname;
}
