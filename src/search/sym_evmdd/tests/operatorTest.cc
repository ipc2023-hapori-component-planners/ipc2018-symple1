// Copyright 13.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include <random>
#include <gtest/gtest.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <iterator>
#include <vector>
#include "./userDefiniedOperators.h"
#include "./testHelper.h"

TEST(unionMin, examplePaper) {
  // Setting up lib
  initializeUserDefiniedOps();
  MEDDLY::initialize();

  // actual test
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  int results_union[] = { 0, 2, 2, max, 2, 4, max, 1, 1, 3, max, 2 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge state_union(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);
  MEDDLY::apply(UNIONMIN, state1, state2, state_union);
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(state_union, ass_v);
    ASSERT_EQ(results_union[i], result);
  }

  /*MEDDLY::FILE_output out(stdout);
  state1.show(out, 3);
  std::cout << "-----------------" << std::endl;
  state2.show(out, 3);
  std::cout << "-----------------" << std::endl;
  state_union.show(out, 3);*/

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(unionMin, random) {
  // no intialization needed again
  for (int i = 0; i < 1000; i++) {
    make_rnd_test(0, 6, 2, 8, UNIONMIN, &minInt);
  }
}

TEST(intersectionMax, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  int results_result[] = { 0, max, max, max, 2, max, max, max, 3, max, max, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge state_union(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);
  MEDDLY::apply(INTERSECTIONMAX, state1, state2, state_union);
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(state_union, ass_v);
    ASSERT_EQ(results_result[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(intersectionMax, random) {
  // no intialization needed again
  for (int i = 0; i < 1000; i++) {
    make_rnd_test(0, 6, 2, 8, INTERSECTIONMAX, &maxInt);
  }
}

/*TEST(restrictOp, thesis) {
  int max = std::numeric_limits<int>::max();
  int nVars = 6;
  int limits[] = { 2, 2, 2, 2, 2, 2};
  int** assigns = assignments(nVars, limits);
  int results[2*2*2*2*2*2];
  for (int i = 0; i < 2*2*2*2*2*2; i++) {
    if ((assigns[i][1] == 1 && assigns[i][4] == 1) ||
        (assigns[i][2] == 1 && assigns[i][5] == 1) ||
        (assigns[i][3] == 1 && assigns[i][6] == 1)) {
      results[i] = 0;
    } else {
      results[i] = max;
    }
  }
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state(evmdd);
  MEDDLY::dd_edge result(evmdd);
  evmdd->createEdge(assigns, results, 2*2*2*2*2*2, state);
  MEDDLY::FILE_output out(stdout);
  state.show(out, 3);
  std::cout << "-----------------" << std::endl;
}*/

/*TEST(restrictOp, thesis) {
  int max = std::numeric_limits<int>::max();
  int nVars = 4;
  int limits[] = { 2, 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results[] = { 4, 4, 2, max, 4, 4, 2, max,   5, 5, 3, max, 5, 5, 3, max,
                     9, 9, 7, max, 5, 5, 5, 5};
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state(evmdd);
  MEDDLY::dd_edge result(evmdd);
  evmdd->createEdge(assigns, results, 2*2*2*3, state);
  MEDDLY::FILE_output out(stdout);
  state.show(out, 3);
  std::cout << "-----------------" << std::endl;
  setRestrictVarVal(state, result, 2, 0);
  MEDDLY::apply(RESTRICT, state, result);
  result.show(out, 3);
  std::cout << "-----------------" << std::endl;
  setRestrictVarVal(state, result, 2, 1);
  MEDDLY::apply(RESTRICT, state, result);
  result.show(out, 3);
  std::cout << "-----------------" << std::endl;
  setMinVar(state, result, 2);
  MEDDLY::apply(MINVAR, state, result);
  result.show(out, 3);
}*/

TEST(restrictOp, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge state_results1(evmdd);
  MEDDLY::dd_edge state_results2(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);

  // create results stat 1
  int s1ev00[] = { 0, 0, 2, 2, 2, 2, max, max, 3, 3, max, max };
  int s1ev01[] = { max, max, max, max, max, max, 1, 1, max, max, 2, 2 };
  int s1ev10[] = { 0, max, 0, max, 2, max, 2, max, 3, max, 3, max };
  int s1ev11[] = { 2, max, 2, max, max, 1, max, 1, max, 2, max, 2 };
  int s1ev20[] = { 0, max, 2, max, 0, max, 2, max, 0, max, 2, max };
  int s1ev21[] = { 2, max, max, 1, 2, max, max, 1, 2, max, max, 1 };
  int s1ev22[] = { 3, max, max, 2, 3, max, max, 2, 3, max, max, 2 };
  int* s1res[] = { s1ev00, s1ev01, s1ev10, s1ev11, s1ev20, s1ev21, s1ev22 };

  // create results stat 2
  int s2ev00[] = { 0, 0, max, max, 2, 2, max, max, 1, 1, max, max };
  int s2ev01[] = { 2, 2, max, max, 4, 4, max, max, 3, 3, 3, 3 };
  int s2ev10[] = { 0, 2, 0, 2, 2, 4, 2, 4, 1, 3, 1, 3 };
  int s2ev11[] = { max, max, max, max, max, max, max, max, max, 3, max, 3 };
  int s2ev20[] = { 0, 2, max, max, 0, 2, max, max, 0, 2, max, max };
  int s2ev21[] = { 2, 4, max, max, 2, 4, max, max, 2, 4, max, max };
  int s2ev22[] = { 1, 3, max, 3, 1, 3, max, 3, 1, 3, max, 3 };
  int* s2res[] = { s2ev00, s2ev01, s2ev10, s2ev11, s2ev20, s2ev21, s2ev22 };

  int cur_res = 0;
  int result;
  for (int var = 1; var <= 3; var++) {
    for (int val = 0; val < limits[var - 1]; val++) {
      setRestrictVarVal(state1, state_results1, var, val);
      setRestrictVarVal(state2, state_results2, var, val);
      MEDDLY::apply(RESTRICT, state1, state_results1);
      MEDDLY::apply(RESTRICT, state2, state_results2);
      for (int i = 0; i < 12; i++) {
        // Creates the assignment vectors => we use our own evaluate function!
        int vec[nVars];
        for (int j = 1; j < nVars + 1; j++) {
          vec[j - 1] = assigns[i][j];
        }
        std::vector<int> ass_v(vec, vec + nVars);
        ass_v.insert(ass_v.begin(), 0);

        result = evaluatePath(state_results1, ass_v);
        ASSERT_EQ(s1res[cur_res][i], result);
        result = evaluatePath(state_results2, ass_v);
        ASSERT_EQ(s2res[cur_res][i], result);
      }
      cur_res++;
    }
  }
  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(minVar, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge state_results1(evmdd);
  MEDDLY::dd_edge state_results2(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);

  // create results state 1
  int s1minVar1[] = { 0, 0, 2, 2, 2, 2, 1, 1, 3, 3, 2, 2 };
  int s1minVar2[] = { 0, max, 0, max, 2, 1, 2, 1, 3, 2, 3, 2 };
  int s1minVar3[] = { 0, max, 2, 1, 0, max, 2, 1, 0, max, 2, 1 };
  int* s1res[] = { s1minVar1, s1minVar2, s1minVar3 };

  // create results state 2
  int s2minVar1[] = { 0, 0, max, max, 2, 2, max, max, 1, 1, 3, 3 };
  int s2minVar2[] = { 0, 2, 0, 2, 2, 4, 2, 4, 1, 3, 1, 3 };
  int s2minVar3[] = { 0, 2, max, 3, 0, 2, max, 3, 0, 2, max, 3 };
  int* s2res[] = { s2minVar1, s2minVar2, s2minVar3 };

  int result;
  for (int var = 1; var <= 3; var++) {
    setMinVar(state1, state_results1, var);
    setMinVar(state2, state_results2, var);
    MEDDLY::apply(MINVAR, state1, state_results1);
    MEDDLY::apply(MINVAR, state2, state_results2);
    for (int i = 0; i < 12; i++) {
      // Creates the assignment vectors => we use our own evaluate function!
      int vec[nVars];
      for (int j = 1; j < nVars + 1; j++) {
        vec[j - 1] = assigns[i][j];
      }
      std::vector<int> ass_v(vec, vec + nVars);
      ass_v.insert(ass_v.begin(), 0);

      result = evaluatePath(state_results1, ass_v);
      ASSERT_EQ(s1res[var - 1][i], result);
      result = evaluatePath(state_results2, ass_v);
      ASSERT_EQ(s2res[var - 1][i], result);
    }
  }
}

TEST(partialComplement, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge comp1(evmdd);
  MEDDLY::dd_edge comp2(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);

  // TEST the results
  int results1_after[] = { max, 0, max, 0, max, 0, 0, max, max, 0, 0, max };
  int results2_after[] = { max, max, 0, 0, max, max, 0, 0, max, max, 0, max };
  MEDDLY::apply(PARTIALCOMPLEMENT, state1, comp1);
  MEDDLY::apply(PARTIALCOMPLEMENT, state2, comp2);
  /*MEDDLY::FILE_output out(stdout);
   state2.show(out, 3);
   std::cout << "\n\n" << std::endl;
   comp2.show(out, 3);*/
  int result;
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
      //std::cout << vec[j - 1] << " " << std::flush;
    }
    //std::cout << std::endl;
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    result = evaluatePath(comp1, ass_v);
    ASSERT_EQ(results1_after[i], result);
    result = evaluatePath(comp2, ass_v);
    ASSERT_EQ(results2_after[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(partialComplement, test1) {
  int max = std::numeric_limits<int>::max();
  int nVars = 1;
  int limits[] = { 3 };
  int** assigns = assignments(nVars, limits);
  int results[] = { 3, 4, 4 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state(evmdd);
  MEDDLY::dd_edge comp(evmdd);
  evmdd->createEdge(assigns, results, 3, state);

  // TEST the results
  int results_after[] = { max, max, max };
  MEDDLY::apply(PARTIALCOMPLEMENT, state, comp);

  /*MEDDLY::FILE_output out(stdout);
   state.show(out, 3);
   std::cout << "\n\n" << std::endl;
   comp.show(out, 3);*/

  int result;
  for (int i = 0; i < 3; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
      //std::cout << vec[j - 1] << " " << std::flush;
    }
    //std::cout << std::endl;
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    result = evaluatePath(comp, ass_v);
    ASSERT_EQ(results_after[i], result);
  }

  // clear memory
  for (int i = 0; i < 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

int partialComp(int i, int dummy) {
  int max = std::numeric_limits<int>::max();
  if (i == max)
    return 0;
  return max;
}

TEST(partialComplement, random) {
  // no intialization needed again
  for (int i = 0; i < 1000; i++) {
    make_rnd_test(0, 6, 2, 8, PARTIALCOMPLEMENT, &partialComp);
  }
}

TEST(minStates, examplePaperMod) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 2, max, 2, max, 2, max, max, 2, 3, max, max, 2 };
  int results2[] = { 0, 0, max, max, 0, 4, max, max, 1, 3, max, 0 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge minStates1(evmdd);
  MEDDLY::dd_edge minStates2(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);

  // TEST the results
  int results1_after[] = { 2, max, 2, max, 2, max, max, 2, max, max, max, 2 };
  int results2_after[] = { 0, 0, max, max, 0, max, max, max, max, max, max, 0 };
  MEDDLY::apply(MINSTATES, state1, minStates1);
  MEDDLY::apply(MINSTATES, state2, minStates2);
  /*MEDDLY::FILE_output out(stdout);
   state1.show(out, 3);
   std::cout << "\n\n" << std::endl;
   minStates1.show(out, 3);*/
  int result;
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
      //std::cout << vec[j - 1] << " " << std::flush;
    }
    //std::cout << std::endl;
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    result = evaluatePath(minStates1, ass_v);
    ASSERT_EQ(results1_after[i], result);
    result = evaluatePath(minStates2, ass_v);
    ASSERT_EQ(results2_after[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

int minOrNot(int cur, int min) {
  if (cur == min) return cur;
  return std::numeric_limits<int>::max();
}

TEST(minStates, random) {
  // no intialization needed again
  for (int i = 0; i < 1000; i++) {
    make_rnd_test(0, 6, 2, 8, MINSTATES, &minOrNot);
  }
}

TEST(lessThan, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge union_result(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);
  MEDDLY::apply(UNIONMIN, state1, state2, union_result);

  // LESS THAN 3
  setLessThanVal(union_result, union_result, 3);
  MEDDLY::apply(LESSTHAN, union_result, union_result);
  int results_lessThan[] = { 0, 2, 2, max, 2, max, max, 1, 1, max, max, 2 };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_lessThan[i], result);
  }

  // LESS THAN 2
  setLessThanVal(union_result, union_result, 2);
  MEDDLY::apply(LESSTHAN, union_result, union_result);
  int results_lessThan2[] = { 0, max, max, max, max, max, max, 1, 1, max, max,
  max };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_lessThan2[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(lessThanApprox, examplePaper) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results2[] = { 0, 2, max, max, 2, 4, max, max, 1, 3, max, 3 };
  int results_union[] = { 0, 2, 2, max, 2, 4, max, 1, 1, 3, max, 2 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge union_result(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);
  MEDDLY::apply(UNIONMIN, state1, state2, union_result);

  // LESS THAN 3
  setLessThanApproxVal(union_result, union_result, 3);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union[i], result);
  }

  // LESS THAN 2
  setLessThanApproxVal(union_result, union_result, 2);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);
  int results_union2[] = { 0, max, max, max, 2, max, max, 1, 1, max, max, 2 };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union2[i], result);
  }

  // LESS THAN 1
  MEDDLY::apply(UNIONMIN, state1, state2, union_result);
  setLessThanApproxVal(union_result, union_result, 1);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);
  int results_union3[] = { 0, max, max, max, max, max, max, max, max, max, max,
  max };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union3[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}

TEST(lessThanApprox, examplePaper2) {
  int max = std::numeric_limits<int>::max();
  int nVars = 3;
  int limits[] = { 2, 2, 3 };
  int** assigns = assignments(nVars, limits);
  //int results1[] = { 0, max, 2, max, 2, max, max, 1, 3, max, max, 2 };
  int results1[] = { 2, max, max, 3, 1, max, max, 2, max, 2, max, 0 };
  int results2[] = { 3, max, 3, 1, max, max, 4, 2, max, max, 2, 0 };
  int results_union[] = { 2, max, 3, 1, 1, max, 4, 2, max, 2, 2, 0 };
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge union_result(evmdd);
  evmdd->createEdge(assigns, results1, 12, state1);
  evmdd->createEdge(assigns, results2, 12, state2);
  MEDDLY::apply(UNIONMIN, state1, state2, union_result);

  // LESS THAN 3
  setLessThanApproxVal(union_result, union_result, 3);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union[i], result);
  }

  // LESS THAN 2
  setLessThanApproxVal(union_result, union_result, 2);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);
  int results_union2[] = { 2, max, max, 1, 1, max, max, 2, max, max, max, 0 };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union2[i], result);
  }

  // LESS THAN 1
  MEDDLY::apply(UNIONMIN, state1, state2, union_result);
  setLessThanApproxVal(union_result, union_result, 1);
  MEDDLY::apply(LESSTHANAPPROX, union_result, union_result);
  int results_union3[] = { max, max, max, max, max, max, max, max, max, max,
  max, 0 };

  // TEST THE RESULTS
  for (int i = 0; i < 12; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(union_result, ass_v);
    ASSERT_EQ(results_union3[i], result);
  }

  // clear memory
  for (int i = 0; i < 2 * 2 * 3; i++) {
    delete assigns[i];
    assigns[i] = nullptr;
  }
  delete assigns;
  assigns = nullptr;
}
