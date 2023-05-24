#include "logical_expression.h"
#include "math_utils.h"

#include <assert.h>
#include <iomanip>
#include <iostream>

using std::string;
using std::map;
using std::vector;
using std::set;
using std::ostream;
using std::shared_ptr;
using std::enable_shared_from_this;
using std::dynamic_pointer_cast;
using std::cout;

int LogicalExpression::constantMultiplier = 1;

shared_ptr<LogicalExpression> LogicalExpression::multiplyOut() {
  return shared_from_this();
}

shared_ptr<LogicalExpression> LogicalExpression::simplify() {
  return shared_from_this();
}

shared_ptr<LogicalExpression> LogicalExpression::normalizeStateFluents() {
  return shared_from_this();
}

shared_ptr<LogicalExpression> LogicalExpression::normalizeConstants() {
  return shared_from_this();
}

void LogicalExpression::fluents(
    set<shared_ptr<StateFluent const>>& /*fluents*/) const {}

void LogicalExpression::createEdge(
    vector<MEDDLY::dd_edge>& /*edges*/, MEDDLY::forest* /*mdd*/,
    map<int, int> const& /*fluentMDDIndex*/) const {
  print(cout);
  assert(false);
}

void LogicalExpression::print(ostream& out) const { out << "not supported"; }

/****************STATE FLUENT************
****************************************/
map<string, shared_ptr<StateFluent>> StateFluent::fluentIndices;

map<string, shared_ptr<StateFluent>> const& StateFluent::getFluentIndices() {
      return fluentIndices;
  }

shared_ptr<LogicalExpression> StateFluent::create(string const& name) {
  if (fluentIndices.find(name) != fluentIndices.end()) {
    return fluentIndices.at(name);
  } else {
    auto ptr = shared_ptr<StateFluent>(new StateFluent(name));
	ptr->index = fluentIndices.size();
    fluentIndices[name] = ptr;
    return ptr;
  }
}

shared_ptr<LogicalExpression> StateFluent::normalizeStateFluents() {
  vector<shared_ptr<LogicalExpression>> tuple;
  tuple.push_back(shared_ptr<NumericConstant>(new NumericConstant(1.0)));
  tuple.push_back(shared_from_this());
  auto result = shared_ptr<Multiplication>(new Multiplication(tuple));
  return result;
}

shared_ptr<LogicalExpression> StateFluent::normalizeConstants() {
  return shared_from_this();
}

shared_ptr<LogicalExpression> StateFluent::multiplyOut() {
  return shared_from_this();
}

void StateFluent::fluents(set<shared_ptr<StateFluent const>>& fluents) const {
  auto fluentPtr = dynamic_pointer_cast<StateFluent const>(shared_from_this());
  fluents.insert(fluentPtr);
}

void StateFluent::createEdge(vector<MEDDLY::dd_edge>& edges,
                             MEDDLY::forest* mdd,
                             map<int, int> const& fluentMDDIndex) const {
  MEDDLY::dd_edge edge(mdd);
  mdd->createEdgeForVar(fluentMDDIndex.at(index), false, edge);
  edges.push_back(edge);
}

void StateFluent::print(ostream& out) const { out << name; }

/****************CONSTANTS**************
 ****************************************/

shared_ptr<LogicalExpression> NumericConstant::normalizeConstants() {
  auto c = shared_ptr<NumericConstant>(
      new NumericConstant((int)(this->value * constantMultiplier)));
  return c;
}

shared_ptr<LogicalExpression> NumericConstant::normalizeStateFluents() {
  return shared_from_this();
}

shared_ptr<LogicalExpression> NumericConstant::multiplyOut() {
  return shared_from_this();
}

void NumericConstant::createEdge(
    vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
    map<int, int> const& /*fluentMDDIndex*/) const {
  MEDDLY::dd_edge constant(mdd);
  mdd->createEdge(static_cast<int>(this->value), constant);
  edges.push_back(constant);
}

void NumericConstant::print(ostream& out) const { out << value; }

/****************CONNECTIVE**************
 ****************************************/

void Connective::fluents(set<shared_ptr<StateFluent const>>& fluents) const {
  for (auto expr : exprs) {
    expr->fluents(fluents);
  }
}

void Connective::print(ostream& out) const {
  for (unsigned int i = 0; i < exprs.size(); ++i) {
    exprs[i]->print(out);
    if (i != exprs.size() - 1) {
      out << " ";
    }
  }
}

#include "addition.cc"
#include "multiplication.cc"
#include "subtraction.cc"
#include "absolutValue.cc"

