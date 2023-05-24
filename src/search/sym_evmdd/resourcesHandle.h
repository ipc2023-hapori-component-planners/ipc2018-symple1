// Copyright 21.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef RESOURCESHANDLE_H_
#define RESOURCESHANDLE_H_

#include <meddly.h>
#include <meddly_expert.h>
#include "../utilities.h"

// prints current Memory usage of Meddly
void printMemoryState(MEDDLY::forest* f);

/// Helper for evaluation (inefficient)
// returns number of edges in Meddly
long getNumOfEdges(MEDDLY::forest* f);
// returns number of edges in Meddly with certain edgeValue
int getNumOfSuppressableEdges(MEDDLY::forest* f, int edgeValue);

#endif // RESOURCESHANDLE_H_
