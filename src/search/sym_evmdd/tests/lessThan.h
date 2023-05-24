// Copyright 23.07.2017, University of Freiburg,
// Author: David Speck <speckd>.

#ifndef LESSTHAN_H_
#define LESSTHAN_H_

#include <meddly.h>
#include <meddly_expert.h>

extern const MEDDLY::unary_opname* LESSTHAN;

void initializeLessThan();

void setLessThanVal(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int val);

#endif // LESSTHAN_H_
