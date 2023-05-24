// Copyright 01.09.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include <vector>
#include <map>
#include <queue>
#include <utility>
#include "./userDefiniedOperators.h"
#include "../resourcesHandle.h"

const MEDDLY::unary_opname* LESSTHANAPPROX;
/**
 * This class is to compute an edge which only contains all paths less than a value.
 */
class lessThanApprox: public MEDDLY::unary_operation {
  public:
  void setLessThanApprox(int val) {
    this->val = val;
  }
  lessThanApprox(const MEDDLY::unary_opname* code, MEDDLY::expert_forest* arg,
  MEDDLY::expert_forest* res, int val) :
  MEDDLY::unary_operation(code, 2, 2, arg, res), val(val) {
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
    int av, ev;
    MEDDLY::node_handle result;
    a.getEdgeValue(av);
    fillMinVals(av, a.getNode());
    if (av >= val) {
      b.set(-1, std::numeric_limits<int>::max());
    }
    compute(av, a.getNode(), ev, result);
    b.set(result, ev);
    minValues.clear();
  }

  protected:
  int val;
  std::map<MEDDLY::node_handle, int> minValues;

  ~lessThanApprox() {
  }

  void fillMinVals(int av, MEDDLY::node_handle ap) {
    // maybe use a sorted data struc to speed up
    minValues[ap] = av;

    // Compare function
    auto lessThanByLevel = [this](const MEDDLY::node_handle& lnh,
    const MEDDLY::node_handle& rnh)
    { return argF->getNodeLevel(lnh) < argF->getNodeLevel(rnh);};

    std::priority_queue<MEDDLY::node_handle, std::vector<MEDDLY::node_handle>,
    decltype(lessThanByLevel)> queue(lessThanByLevel);

    queue.push(ap);

    while (!queue.empty()) {
      MEDDLY::node_handle q_cur = queue.top();
      queue.pop();
      MEDDLY::unpacked_node *A = MEDDLY::unpacked_node::newFromNode(argF, q_cur,
      true);
      const int resultSize = resF->getLevelSize(argF->getNodeLevel(q_cur));
      for (int i = 0; i < resultSize; i++) {
        MEDDLY::node_handle child = A->d(i);
        if (argF->isTerminalNode(child))
          continue;
        int childValue = minValues[q_cur] + A->ei(i);
        if (minValues.find(child) == minValues.end())
          minValues[child] = std::numeric_limits<int>::max();
        minValues[child] = std::min(minValues[child], childValue);
        //std::cout << "Node: " << child << " set to " << minValues[child]
        //<< std::endl;
        queue.push(child);
      }
    }
  }

  // HASH TABLE FUNCTIONS //
  MEDDLY::compute_table::search_key* findResult(MEDDLY::node_handle a, int &cv, // NOLINT for consistency
  MEDDLY::node_handle &c) {
    MEDDLY::compute_table::search_key* CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
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
    if (argF->isTerminalNode(ap)) {
      //std::cout << "triggerd " << rv << "," << av << " = "   << rv + av << std::endl;
      rv = av;
      rp = ap;
      return;
    }

    // If we already visited the node we stop here
    MEDDLY::compute_table::search_key* Key = findResult(ap, rv, rp);
    if (0 == Key) {
      // rv here is the residual from the node reduction of rp
      //std::cout << "triggerd " << rv << "," << av << " = "   << rv + av << std::endl;
      rv += av;
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
    for (int i = 0; i < resultSize; i++) {
      // std::cout << A->d(i) << " : " << minValues[A->d(i)] << std::endl;
      if ((!argF->isTerminalNode(A->d(i)) && minValues[A->d(i)] >= val) || (argF->isTerminalNode(A->d(i)) &&
      (A->ei(i) == std::numeric_limits<int>::max() || A->ei(i) + minValues[ap] >= val))) {
        // Node is more expensive
        //std::cout << ap << ":" << i << " - " << A->d(i) << " reduced!" << std::endl;
        ep = -1;
        ev = std::numeric_limits<int>::max();
      } else {
        // recursive call
        compute(A->ei(i), A->d(i), ev, ep);
      }
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
    if (rv >= 0 && av >= 0 && rv + av < 0) {
      //std::cout << "triggerd " << rv << "," << av << " = "   << rv + av << std::endl;
      rv = std::numeric_limits<int>::max();
    } else {
      rv += av;
    }
  }
};
class lessThanApprox_opname: public MEDDLY::unary_opname {
  public:
  lessThanApprox_opname() :
  unary_opname("LessThanApprox") {
  }
  MEDDLY::unary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* res) const {
    // by default we restrict var 1 to val 0
    return new lessThanApprox(this, arg1, res, 0);
  }
};
void initializeLessThanApprox() {
  LESSTHANAPPROX = new lessThanApprox_opname;
}
void setLessThanApproxVal(const MEDDLY::dd_edge& arg,
const MEDDLY::dd_edge& res, int val) {
  lessThanApprox* lessThanApproxOP =
  reinterpret_cast<lessThanApprox*>(MEDDLY::getOperation(LESSTHANAPPROX, arg,
  res));
  lessThanApproxOP->setLessThanApprox(val);
}

