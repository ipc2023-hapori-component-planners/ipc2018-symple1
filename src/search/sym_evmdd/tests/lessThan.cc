// Copyright 23.07.2017, University of Freiburg,
// Author: David Speck <speckd>.
#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

const MEDDLY::unary_opname* LESSTHAN;
/**
 * This class is to compute an edge which only contains all paths less than a value.
 */
class lessThan: public MEDDLY::unary_operation {
  public:
  void setLessThan(int val) {
    this->val = val;
  }
  lessThan(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
  MEDDLY::expert_forest* res, int val) :
  MEDDLY::unary_operation(code, 1, 3, arg, res), val(val) {
  }
  void discardEntry(const MEDDLY::node_handle* data) {
    argF->uncacheNode(data[0]);
    resF->uncacheNode(data[2]);
  }
  bool isStaleEntry(const MEDDLY::node_handle* data) {
    return argF->isStale(data[0]) || resF->isStale(data[2]);
  }

  //TODO(speckd): write this one correct
  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* data) const {
    strm << "[" << getName() << "(" << long(data[0]) << "): " << long(data[1])
    << "]";
  }
  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    MEDDLY::node_handle result;
    int av, ev;
    a.getEdgeValue(av);
    if (av >= val) {
      b.set(-1, std::numeric_limits<int>::max());
    }
    compute(av, a.getNode(), ev, result, av);
    b.set(result, ev);
  }
  protected:
  int val;
  ~lessThan() {
  }
  // HASH TABLE FUNCTIONS //
  MEDDLY::compute_table::search_key* findResult(MEDDLY::node_handle a, int &cv, // NOLINT for consistency
  MEDDLY::node_handle &c, int &sum) {
    MEDDLY::compute_table::search_key* CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    // Also write var and val -> another restrict on the same edges with different
    // var and val look up wrong values!
    CTsrch->write(val);
    CTsrch->writeNH(a);
    MEDDLY::compute_table::search_result &cacheFind = CT->find(CTsrch);
    if (!cacheFind)
      return CTsrch;
    cacheFind.read(cv);
    c = resF->linkNode(cacheFind.readNH());
    cacheFind.read(sum);
    doneCTkey(CTsrch);
    return 0;
  }
  void saveResult(MEDDLY::compute_table::search_key* Key, MEDDLY::node_handle a,
  int cv, MEDDLY::node_handle c, int sum) {
    // return;
    argF->cacheNode(a);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cv);
    entry.writeResultNH(resF->cacheNode(c));
    entry.writeResult(sum);
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
  MEDDLY::node_handle &rp, int sum) {
    //std::cout << "Node: " << ap << std::endl;
    //std::cout << "(" << av << ", " << ap << ", " <<  rv << ", " << rp << ")" << std::endl;

    // is terminal node -> nothing to do
    if (argF->isTerminalNode(ap)) {
      sum += av;
      if (rv == std::numeric_limits<int>::max()
      || av == std::numeric_limits<int>::max())
        sum = std::numeric_limits<int>::max();
      rv = av;
      rp = ap;
      return;
    }

    // If we already visited the node we stop here
    /*MEDDLY::compute_table::search_key* Key = findResult(ap, rv, rp, sum);
    if (0 == Key) {
      // rv here is the residual from the node reduction of rp
      rv += av;
      if ((rv < 0 && av > 0 && rv > 0) || rv == std::numeric_limits<int>::max()
      || av == std::numeric_limits<int>::max())
        rv = std::numeric_limits<int>::max();
      sum += av;
      if (sum == std::numeric_limits<int>::max() || av == std::numeric_limits<int>::max())
        rv = std::numeric_limits<int>::max();
      return;
    }*/

    // Get level information
    const int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);
    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
    true);

    // Initialize result
    MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(resF,
    resultLevel, resultSize);
    int ev;
    MEDDLY::node_handle ep;
    for (int i = 0; i < resultSize; i++) {
      int cur = sum + A->ei(i);
      if (sum == std::numeric_limits<int>::max()
      || A->ei(i) == std::numeric_limits<int>::max()) {
        cur = std::numeric_limits<int>::max();
      }
      //std::cout << ap << "." << i << " :" << cur << " ~ " << A->ei(i)
      //<< std::endl;
      if (cur >= val) {
        //std::cout << "set it" << std::endl;

        /// HERE SET THIS TO link to 0 for suppression
        ///ep = 0;
        ev = std::numeric_limits<int>::max();
        ep = -1;
      } else {
        compute(A->ei(i), A->d(i), ev, ep, cur);
      }

      nb->d_ref(i) = ep;
      nb->setEdge(i, ev);
      //std::cout << "Node: " << ap << " Edge: " << i << " SUM: " << cur << std::endl;
    }
    MEDDLY::unpacked_node::recycle(A);
    // Create a node handle...links cl to nb with cost cv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;
    // Add to CT
    // Finally add value av to current edge value
    //saveResult(Key, ap, rv, rp, sum);


    if ((rv < 0 && av > 0 && rv > 0) || rv == std::numeric_limits<int>::max()
    || av == std::numeric_limits<int>::max()) {
      rv = std::numeric_limits<int>::max();
    } else {
      rv += av;
    }
  }
};
class lessThan_opname: public MEDDLY::unary_opname {
  public:
  lessThan_opname() :
  unary_opname("LessThan") {
  }
  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    // by default we restrict var 1 to val 0
    return new lessThan(this, arg1, res, 0);
  }
};
void initializeLessThan() {
  LESSTHAN = new lessThan_opname;
}
void setLessThanVal(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int val) {
  lessThan* lessThanOP = reinterpret_cast<lessThan*>(MEDDLY::getOperation(
  LESSTHAN, arg, res));
  lessThanOP->setLessThan(val);
}

