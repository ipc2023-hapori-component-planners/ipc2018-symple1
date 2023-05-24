// Copyright 16.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef TESTHELPER_H_
#define TESTHELPER_H_

#include <random>
#include <gtest/gtest.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <meddly.h>
#include "userDefiniedOperators.h"

// Create assignments for n Vars and the corresponding limits (including a leading 0)
int** assignments(int nVars, int* limits);

void make_rnd_test(int min_var, int max_var, int min_lim, int max_lim,
const MEDDLY::binary_opname* op, int (*eval)(int, int) );

void make_rnd_test(int min_var, int max_var, int min_lim, int max_lim,
const MEDDLY::unary_opname* op, int (*eval)(int, int) );

bool readEdge(std::string fileName, MEDDLY::forest* f, MEDDLY::dd_edge& res);

#endif  // TESTHELPER_H_
