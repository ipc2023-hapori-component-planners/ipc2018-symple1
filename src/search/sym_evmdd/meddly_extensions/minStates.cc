// Copyright 22.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

const MEDDLY::unary_opname* MINSTATES;


class minStates: public MEDDLY::unary_operation {
  public:
  minStates(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
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

  //TODO(speckd): write this one correct
  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle* data) const {
    strm << "[" << getName() << "(" << long(data[0]) << "): " << long(data[1])
    << "]";
  }
  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    MEDDLY::node_handle result;
    int av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    b.set(result, ev);
  }
  protected:
  ~minStates() {
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
    //std::cout << "Node: " << ap << std::endl;
    //std::cout << "(" << av << ", " << ap << ", " <<  rv << ", " << rp << ")" << std::endl;

    // is terminal node -> nothing to do
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
      if ((rv < 0 && av > 0 && rv > 0) || rv == std::numeric_limits<int>::max()
      || av == std::numeric_limits<int>::max())
        rv = std::numeric_limits<int>::max();
      return;
    }

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
      //std::cout << ap << "." << i << " :" << cur << " ~ " << A->ei(i)
      //<< std::endl;
      if (A->ei(i) > 0) {
        //std::cout << "set it" << std::endl;

        /// HERE SET THIS TO link to 0 for suppression
        ///ep = 0;
        ev = std::numeric_limits<int>::max();
        ep = -1;
      } else {
        compute(A->ei(i), A->d(i), ev, ep);
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
    saveResult(Key, ap, rv, rp);


    if ((rv < 0 && av > 0 && rv > 0) || rv == std::numeric_limits<int>::max()
    || av == std::numeric_limits<int>::max()) {
      rv = std::numeric_limits<int>::max();
    } else {
      rv += av;
    }
  }
};
class minStates_opname: public MEDDLY::unary_opname {
  public:
  minStates_opname() :
  unary_opname("MinStates") {
  }
  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    return new minStates(this, arg1, res);
  }
};
void initializeMinStates() {
  MINSTATES = new minStates_opname;
}



