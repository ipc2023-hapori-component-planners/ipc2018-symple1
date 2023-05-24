// Copyright 16.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef GENERICBINARYOPERATOR_H_
#define GENERICBINARYOPERATOR_H_

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>

class genericBinaryOperator: public MEDDLY::binary_operation {
  protected:
  int (*OpType)(int, int);

  bool checkForestCompatibility() const {
    return arg1F->isEVPlus() && arg2F->isEVPlus();
  }

  ~genericBinaryOperator() {
  }

  inline MEDDLY::compute_table::search_key* findResult(int aev,
  MEDDLY::node_handle a, int bev, MEDDLY::node_handle b, int& cev,
  MEDDLY::node_handle &c) {
    MEDDLY::compute_table::search_key* CTsrch = useCTkey();
    MEDDLY_DCASSERT(CTsrch);
    CTsrch->reset();
    if (can_commute && a > b) {
      CTsrch->write(bev - aev);
      CTsrch->writeNH(b);
      //CTsrch->write(aev);
      CTsrch->writeNH(a);
    } else {
      CTsrch->write(aev - bev);
      CTsrch->writeNH(a);
      // CTsrch->write(bev);
      CTsrch->writeNH(b);
    }
    MEDDLY::compute_table::search_result &cacheFind = CT->find(CTsrch);
    if (!cacheFind)
      return CTsrch;

    // Check for infinity bound
    cacheFind.read(cev);
    // std::cout << a << " - "<< aev << ", " << b << " - " << bev << " : " << cev << std::endl;
    if (cev == std::numeric_limits<int>::max()
    || OpType(aev, bev) == std::numeric_limits<int>::max()) {
      cev = std::numeric_limits<int>::max();
    } else {
      cev += OpType(aev, bev);
    }

    c = resF->linkNode(cacheFind.readNH());
    doneCTkey(CTsrch);
    return 0;
  }

  inline void saveResult(MEDDLY::compute_table::search_key* Key,
  MEDDLY::node_handle a, MEDDLY::node_handle b, int cev,
  MEDDLY::node_handle c) {
    arg1F->cacheNode(a);
    arg2F->cacheNode(b);
    MEDDLY::compute_table::entry_builder &entry = CT->startNewEntry(Key);
    entry.writeResult(cev);
    entry.writeResultNH(resF->cacheNode(c));
    CT->addEntry();
  }

  public:

  // Key table to enter + the corresponding operator
  genericBinaryOperator(const MEDDLY::binary_opname* code,
  MEDDLY::expert_forest* arg1, MEDDLY::expert_forest* arg2,
  MEDDLY::expert_forest* res, int (*OpType)(int, int)) :
  binary_operation(code, 3, 2, arg1, arg2, res), OpType(OpType) {
    // Seems to slow the 3 storage methode
    //this->can_commute = true;
  }

  void compute(const MEDDLY::dd_edge &ar1, const MEDDLY::dd_edge &ar2,
  MEDDLY::dd_edge &res);

  void computeDDEdge(const MEDDLY::dd_edge &ar1, const MEDDLY::dd_edge &ar2,
  MEDDLY::dd_edge &res);

  void compute(int av, MEDDLY::node_handle ap, int bv, MEDDLY::node_handle bp,
  int &cv, MEDDLY::node_handle &cp);

  void discardEntry(const MEDDLY::node_handle *entryData);

  void showEntry(MEDDLY::output &strm, const MEDDLY::node_handle *data) const;

  bool isStaleEntry(const MEDDLY::node_handle* data);

  // Check if something is a terminal
  virtual bool checkTerminals(int aev, MEDDLY::node_handle a, int bev,
  MEDDLY::node_handle b, int& cev, MEDDLY::node_handle& c) = 0;
};

#endif  // GENERICBINARYOPERATOR_H_
