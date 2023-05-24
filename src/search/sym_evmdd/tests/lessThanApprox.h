// Copyright 01.09.2017, University of Freiburg,
// Author: David Speck <speckd>.


#ifndef LESSTHANAPPROX_H_
#define LESSTHANAPPROX_H_

#include <meddly.h>
#include <meddly_expert.h>

extern const MEDDLY::unary_opname* LESSTHANAPPROX;

void initializeLessThanApprox();

void setLessThanApproxVal(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res,
int val);

#endif // LESSTHANAPPROX_H_
