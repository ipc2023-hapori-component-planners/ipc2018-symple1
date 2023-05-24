// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.


#ifndef UNIONMIN_H_
#define UNIONMIN_H_

#include <meddly.h>
#include <meddly_expert.h>

extern const MEDDLY::binary_opname* UNIONMIN;

void initializeUnionMin();

int minInt(int a, int b);

#endif  // UNIONMIN_H_
