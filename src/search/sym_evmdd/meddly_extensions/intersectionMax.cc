// Copyright 16.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <meddly.h>
#include <meddly_expert.h>
#include <limits>
#include <algorithm>
#include "./intersectionMax.h"
#include "./genericBinaryOperator.h"

const MEDDLY::binary_opname* INTERSECTIONMAX;

int maxInt(int a, int b) {
  return (b > a) ? b : a;
}

class intersectionMax: public genericBinaryOperator {
  public:
  // Key table to enter
  intersectionMax(const MEDDLY::binary_opname* code,
  MEDDLY::expert_forest* arg1, MEDDLY::expert_forest* arg2,
  MEDDLY::expert_forest* res) :
  genericBinaryOperator(code, arg1, arg2, res, &maxInt) {
  }

  bool checkTerminals(int aev, MEDDLY::node_handle a, int bev,
  MEDDLY::node_handle b, int& cev, MEDDLY::node_handle& c) {
    {
      // both nodes are terminals - return min of both
      if (a == -1 && b == -1) {
        c = -1;
        cev = OpType(aev, bev);
        return true;
      }
      // Is 0 if nothing is specified for at least one tree.
      // Means at least one has inf cost!
      if (0 == a || 0 == b) {
        c = 0;
        cev = std::numeric_limits<int>::max();
        return true;
      }
      // Case where one (or both) edge is infinity
      if ((aev == std::numeric_limits<int>::max())
      || (bev == std::numeric_limits<int>::max())) {
        c = -1;
        cev = std::numeric_limits<int>::max();
        return true;
      }
      return false;
    }
  }
};

class intersectionMax_opname: public MEDDLY::binary_opname {
  public:

  intersectionMax_opname() :
  binary_opname("IntersectionMax") {
  }

  MEDDLY::binary_operation* buildOperation(MEDDLY::expert_forest* arg1,
  MEDDLY::expert_forest* arg2, MEDDLY::expert_forest* res) const {
    return new intersectionMax(this, arg1, arg2, res);
  }
};

void initializeIntersectionMax() {
  INTERSECTIONMAX = new intersectionMax_opname;
}

