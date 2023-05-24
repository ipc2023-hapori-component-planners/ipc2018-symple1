// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef RESTRICT_H_
#define RESTRICT_H_

#include <meddly.h>
#include <meddly_expert.h>

extern const MEDDLY::unary_opname* RESTRICT;

void initializeRestrict();

void setRestrictVarVal(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int var, int val);

/***
 * Hack to get the global CT.
 * @param f
 * @return
 */
MEDDLY::compute_table* getComputeTable(MEDDLY::forest* f);

#endif  // RESTRICT_H_
