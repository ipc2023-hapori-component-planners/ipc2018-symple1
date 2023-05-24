// Copyright 19.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#include "testHelper.h"

// Create assignments for n Vars and the corresponding limits (including a leading 0)
int** assignments(int nVars, int* limits) {
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++) {
    nAssigns *= limits[i];
  }
  int** assigns = new int*[nAssigns];
  for (int i = 0; i < nAssigns; i++) {
    assigns[i] = new int[nVars + 1];
    assigns[i][0] = 0;
  }
  for (int v = 0; v < nVars; v++) {
    int rythm = 1;
    int cur_val = 0;
    for (int i = 0; i < v; i++) {
      rythm *= limits[i];
    }
    for (int i = 1; i <= nAssigns; i++) {
      assigns[i - 1][v + 1] = cur_val % limits[v];
      if (i % rythm == 0)
        cur_val++;
    }
  }
  return assigns;
}

void make_rnd_test(int min_var, int max_var, int min_lim, int max_lim,
const MEDDLY::binary_opname* op, int (*eval)(int, int)) {
  MEDDLY::FILE_output meddlyout(stdout);
  int max = std::numeric_limits<int>::max();
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(min_var, max_var);
  std::uniform_int_distribution<int> dist2(min_lim, max_lim);
  int nVars = dist(mt);
  int* limits = new int[nVars];
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++) {
    limits[i] = dist2(mt);
    nAssigns *= limits[i];
  }
  int** assigns = assignments(nVars, limits);
  int* results1 = new int[nAssigns];
  int* results2 = new int[nAssigns];
  for (int i = 0; i < nAssigns; i++) {
    results1[i] = dist2(mt);
    results2[i] = dist2(mt);
    if (results1[i] == max_lim)
      results1[i] = max;
    if (results2[i] == max_lim)
      results2[i] = max;
  }
// Create EVMDD
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state1(evmdd);
  MEDDLY::dd_edge state2(evmdd);
  MEDDLY::dd_edge state_result(evmdd);
  evmdd->createEdge(assigns, results1, nAssigns, state1);
  evmdd->createEdge(assigns, results2, nAssigns, state2);
  MEDDLY::apply(op, state1, state2, state_result);
  for (int i = 0; i < nAssigns; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(state_result, ass_v);

    // Check if correct!
    /*if (eval(results1[i], results2[i]) != result) {
     MEDDLY::FILE_output meddlyout(stdout);
     state1.show(meddlyout, 3);
     state1.getEdgeValue(result);
     std::cout << "State1 ROOT: " << result << std::endl;
     std::cout << "--------------------------" << std::endl;
     state2.show(meddlyout, 3);
     state2.getEdgeValue(result);
     std::cout << "State2 ROOT: " << result << std::endl;
     std::cout << "--------------------------" << std::endl;
     state_result.show(meddlyout, 3);
     state_result.getEdgeValue(result);
     std::cout << "State_Union_Min ROOT: " << result << std::endl;
     std::cout << "--------------------------" << std::endl;
     */
    ASSERT_EQ(eval(results1[i], results2[i]), result);
  }
}

void make_rnd_test(int min_var, int max_var, int min_lim, int max_lim,
const MEDDLY::unary_opname* op, int (*eval)(int, int)) {
  int max = std::numeric_limits<int>::max();
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(min_var, max_var);
  std::uniform_int_distribution<int> dist2(min_lim, max_lim);
  int nVars = dist(mt);
  int* limits = new int[nVars];
  int nAssigns = 1;
  for (int i = 0; i < nVars; i++) {
    limits[i] = dist2(mt);
    nAssigns *= limits[i];
  }
  int** assigns = assignments(nVars, limits);
  int* results = new int[nAssigns];
  for (int i = 0; i < nAssigns; i++) {
    results[i] = dist2(mt);
    if (results[i] == max_lim)
      results[i] = max;
  }
  // Create EVMDD
  MEDDLY::domain* dom = MEDDLY::createDomainBottomUp(limits, nVars);
  MEDDLY::forest* evmdd = dom->createForest(false, MEDDLY::forest::INTEGER,
  MEDDLY::forest::EVPLUS);
  MEDDLY::dd_edge state(evmdd);
  MEDDLY::dd_edge state_result(evmdd);
  evmdd->createEdge(assigns, results, nAssigns, state);
  MEDDLY::apply(op, state, state_result);
  int min = minVal(state_result);
  for (int i = 0; i < nAssigns; i++) {
    // Creates the assignment vectors => we use our own evaluate function!
    int vec[nVars];
    for (int j = 1; j < nVars + 1; j++) {
      vec[j - 1] = assigns[i][j];
    }
    std::vector<int> ass_v(vec, vec + nVars);
    ass_v.insert(ass_v.begin(), 0);
    int result = evaluatePath(state_result, ass_v);

    // Check if correct!
    if (eval(results[i], min) != result) {
      MEDDLY::FILE_output meddlyout(stdout);
      state.show(meddlyout, 3);
      std::cout << "--------------------------" << std::endl;
      state_result.show(meddlyout, 3);
      std::cout << "--------------------------" << std::endl;
      return;
    }
    ASSERT_EQ(eval(results[i], min), result);

  }
}

bool readEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& res) {
  FILE * pFile;
  pFile = fopen(fileName.c_str(), "r");
  MEDDLY::FILE_input fileout(pFile);
  f->readEdges(fileout, &res, 1);
  fclose(pFile);
  return true;
}
