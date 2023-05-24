// Copyright 05.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include "./userDefiniedOperators.h"

const MEDDLY::unary_opname* PARTIALCOMPLEMENT;

class partialComplement: public MEDDLY::unary_operation {
  public:
  partialComplement(const MEDDLY::unary_opname* code,
  MEDDLY::expert_forest* arg, MEDDLY::expert_forest* res) :
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
    strm << "[" << getName() << "(" << long(data[0]) << "): " << long(data[1])
    << "]";
  }

  void computeDDEdge(const MEDDLY::dd_edge& a, MEDDLY::dd_edge& b) {
    // std::cerr << "STILL BUGGY SEE OP TEST" << std::endl;
    MEDDLY::node_handle result;
    int av, ev;
    a.getEdgeValue(av);
    compute(av, a.getNode(), ev, result);
    b.set(result, ev);
  }

  protected:
  ~partialComplement() {
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

  void compute(int av, MEDDLY::node_handle ap, int &rv, // NOLINT for consistency
  MEDDLY::node_handle &rp) {
    if (argF->isTerminalNode(ap)) {
      // Infinity cost become 0 and vise versa
      if (av == std::numeric_limits<int>::max()) {
        rv = 0;
      } else {
        rv = std::numeric_limits<int>::max();
      }
      rp = ap;
      return;
    }

    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key* Key = findResult(ap, rv, rp);
    if (0 == Key) {
      // If it is a terminal case we already flipped => flip again :^)

      // rv here is the residual from the node reduction of rp
      //std::cout << ap << " with " << rp << " - " << rv << std::endl;
      if (rp == -1) {
        if (av == std::numeric_limits<int>::max()) {
          rv = 0;
        } else {
          rv = std::numeric_limits<int>::max();
        }
      } else {
        rv += av;
      }
      return;
    }

    // Get level information
    int resultLevel = argF->getNodeLevel(ap);
    const int resultSize = resF->getLevelSize(resultLevel);

    MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, ap,
    true);

    // Initialize result
    MEDDLY::unpacked_node* nb = MEDDLY::unpacked_node::newFull(resF,
    resultLevel, resultSize);

    // Recursive call
    int ev;
    MEDDLY::node_handle ep;
    //std::cout << "[" << std::flush;
    for (int i = 0; i < resultSize; i++) {
      compute(A->ei(i), A->d(i), ev, ep);
      nb->d_ref(i) = ep;
      if (ev != std::numeric_limits<int>::max())
        ev = 0;
      nb->setEdge(i, ev);

      //std::cout << i << " : " << ev << "|" << std::flush;
    }
    //std::cout << std::endl;

    MEDDLY::unpacked_node::recycle(A);

    // Create a node handle...links cl to nb with cost rv
    MEDDLY::node_handle cl;
    resF->createReducedNode(-1, nb, rv, cl);
    rp = cl;

    // Add to CT
    saveResult(Key, ap, rv, rp);
    //std::cout << "Stored: " << ap << "=>" << rp << " (" << rv << ")" << std::endl;
    // Finally add value av to current edge value

    if (rv == std::numeric_limits<int>::max()
    || av == std::numeric_limits<int>::max()) {
      rv = std::numeric_limits<int>::max();
    } else {
      rv = 0;
    }
  }
};

class partialComplement_opname: public MEDDLY::unary_opname {
  public:

  partialComplement_opname() :
  unary_opname("ParitalComplement") {
  }

  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    return new partialComplement(this, arg1, res);
  }
};

void initializePartialComplement() {
  PARTIALCOMPLEMENT = new partialComplement_opname;
}

