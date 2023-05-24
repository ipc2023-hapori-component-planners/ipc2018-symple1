// Copyright 29.06.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <limits>
#include <algorithm>
#include "operationTransitionCreater.h"
#include "image.h"
#include "meddly_extensions/userDefiniedOperators.h"

// Can be used during creation for debugging
void dumpFullInformation(int it, MEDDLY::forest* f, vector<Operator>& ops,
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans) {

  std::cout << "Iteration " << it << "/" << ops.size() << " ("
  << opTrans.size() + 1 << " T's)" << ":" << std::endl;

  std::cout << " - MemUsed: " << f->getCurrentMemoryUsed() / 1000.00 / 1000.00
  << "MB" << std::endl;

  std::cout << " - " << std::flush;
  print_used_memory();
  printMemoryState(f);
}

void createOperationTransition(MEDDLY::forest* f, vector<Operator>& ops,
std::vector<std::shared_ptr<MEDDLY::dd_edge>>& opTrans, int nodeLimit) {

  // new transition relation
  auto transOps = std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(f));
  setInfintiyEdge(*transOps);
  try {
    for (int i = 0; i < ops.size(); i++) {
      // create new transition relation and merge it
      MEDDLY::dd_edge curOp(f);
      createOpTrans(f, ops[i], curOp, false);
      ops[i].set_op_evmdd(
      std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(curOp)));
      MEDDLY::apply(MEDDLY::PLUS, curOp, *ops[i].get_SDAC_cost(), curOp);
      MEDDLY::apply(UNIONMIN, *transOps, curOp, *transOps);
      curOp.clear();

      // merge transition relations
      // TODO(speckd): Node count takes time!! We should fix it somehow (maybe time limit)
      if (unsigned(transOps->getNodeCount()) > nodeLimit) {
        opTrans.push_back(transOps);
        transOps = std::shared_ptr<MEDDLY::dd_edge>(new MEDDLY::dd_edge(f));
        setInfintiyEdge(*transOps);
      }
    }
  } catch (const MEDDLY::error& e) {
    // Some meddly error occurs
    std::cout << "- Mem: " << f->getCurrentMemoryUsed() << std::endl;
    std::cout << e.getName()
    << " => MEDDLY error while building the big transition" << std::endl;
    exit_with(EXIT_OUT_OF_MEMORY);
  }
  // Add last transition relation of not empty
  if (transOps->getNode() != -1)
    opTrans.push_back(transOps);
}

void createOperationCostFunction(MEDDLY::forest* f, Operator* op) {
  // Parse the cost functions
  Parser sdacParser;
  shared_ptr<LogicalExpression> logExp = sdacParser.parse(
  op->get_SDAC_cost_function());
  logExp = logExp->simplify();
  std::set<std::shared_ptr<const StateFluent>> flu;
  logExp->fluents(flu);
  map<int, int> fluentIndex;
  vector<MEDDLY::dd_edge> result;
  for (auto const& fluentPtr : flu) {
    auto it = std::find(g_variable_name.begin(), g_variable_name.end(),
    fluentPtr->name);
    if (it == g_variable_name.end()) {
      // name not in vector
      std::cerr << "Fluent: " << fluentPtr->name << " is not a variable!"
      << std::endl;
      assert(false);
    } else {
      auto level = std::distance(g_variable_name.begin(), it);
      // unprimed vars are always above the primed (*2 + 1)
      // and terminal level is 0 (+1)
      fluentIndex[fluentPtr->index] = level * 2 + 2;
    }
  }
  // create the edge
  logExp->createEdge(result, f, fluentIndex);

  // set the function
  op->set_SDAC_cost(make_shared<MEDDLY::dd_edge>(result[0]));
}

