// Copyright 10.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include <algorithm>
#include "./userDefiniedOperators.h"
#include "./genericBinaryOperator.h"

const MEDDLY::binary_opname* UNIONMIN;

int minInt(int a, int b) {
  return (b < a) ? b : a;
}

/**
 * Specific implementation for unionMin.
 */
class unionMin: public genericBinaryOperator {
  public:
  /**
   * Constructor to generate UNIONMIN operator.
   * @param code The code/name of the operator.
   * @param arg1 Input 1 forest.
   * @param arg2 Input 2 forest.
   * @param res Resulting forest.
   */
  unionMin(const MEDDLY::binary_opname* code, MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* arg2, MEDDLY::expert_forest* res) :
  genericBinaryOperator(code, arg1, arg2, res, &minInt) {
  }

  /**
   * Checks for special cases (terminals) of the two handled nodes.
   * This means all cases where not both are inner nodes.
   * @param aev Edge value of node a.
   * @param a Node a.
   * @param bev Edge value of node b.
   * @param b Node b.
   * @param cev Resulting edge value.
   * @param c Resulting node.
   * @return Is special case (terminals).
   */
  bool checkTerminals(int aev, MEDDLY::node_handle a, int bev,
  MEDDLY::node_handle b, int& cev, MEDDLY::node_handle& c) {
    {
      // both nodes are terminals - return min of both
      if (a == -1 && b == -1) {
        c = -1;
        cev = OpType(aev, bev);
        return true;
      }

      // Is 0 if nothing is specified for both trees at this value
      // e.g only one node with <0,t1> but two values for the var
      // TODO(speckd): Really nec =>
      // Also mix (if one time inf supress was unabeled we want it for every!
      if ((a == 0 && b == 0)
      || (a == 0 && b == -1 && bev == std::numeric_limits<int>::max())
      || (b == 0 && a == -1 && aev == std::numeric_limits<int>::max())) {
        c = 0;
        cev = std::numeric_limits<int>::max();
        return true;
      }

      // Case where one edge is infinity
      if (a != 0 && aev == std::numeric_limits<int>::max()) {
    	  if (a != -1) std::cout << "\n\n Why? " << a << "\n\n"  << std::endl;
        c = resF->linkNode(b);
        cev = bev;
        return true;
      }
      if (b != 0 && bev == std::numeric_limits<int>::max()) {
    	  if (b != -1) std::cout << "\n\n Why? \n\n" << std::endl;
        c = resF->linkNode(a);
        cev = aev;
        return true;
      }

      // a is empty means inf cost-> thus to b
      if (0 == a) {
    	  std::cout << "\n\n WHAT WHY?? \n\n" << std::endl;
        if (arg2F == resF) {
          if (b == -1)
            c = 0;
          else
            c = resF->linkNode(b);
          cev = bev;
          return true;
        }
        return false;
      }
      // b is empty means inf cost -> link to a
      if (0 == b) {
    	  std::cout << "\n\n WHAT WHY?? \n\n" << std::endl;
        if (arg1F == resF) {
          if (a == -1)
            a = 0;
          else
            c = resF->linkNode(a);
          cev = aev;
          return true;
        }
        return false;
      }

      return false;
    }
  }
};

class unionMin_opname: public MEDDLY::binary_opname {
  public:

  unionMin_opname() :
  binary_opname("UnionMin") {
  }

  MEDDLY::binary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* arg2, MEDDLY::expert_forest* res) const {
    return new unionMin(this, arg1, arg2, res);
  }
};

void initializeUnionMin() {
  UNIONMIN = new unionMin_opname;
}
