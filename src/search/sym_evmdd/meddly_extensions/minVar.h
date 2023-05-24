// Copyright 29.05.2017, University of Freiburg,
// Author: David Speck <speckd>.


#ifndef MINVAR_H_
#define MINVAR_H_

#include <meddly.h>
#include <meddly_expert.h>

extern int timeMinVar;
extern const MEDDLY::unary_opname* MINVAR;

void initializeMinVar();

void setMinVar(const MEDDLY::dd_edge& arg, const MEDDLY::dd_edge& res, int var);

#endif  // MINVAR_H_
