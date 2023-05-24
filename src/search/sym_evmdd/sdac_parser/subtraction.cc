shared_ptr<LogicalExpression> Subtraction::multiplyOut() {
  vector<shared_ptr<LogicalExpression>> multipliedExprs;
  for (auto expr : exprs) {
    multipliedExprs.push_back(expr->multiplyOut());
  }
  return shared_ptr<Subtraction>(new Subtraction(multipliedExprs));
}

shared_ptr<LogicalExpression> Subtraction::simplify() {
  assert(exprs.size() >= 1);

  vector<shared_ptr<LogicalExpression>> newExprs;
  double constPart = 0.0;

  auto minuend = exprs[0]->simplify();
  auto nc1 = dynamic_pointer_cast<NumericConstant>(minuend);
  bool minuendIsConst = false;

  if (nc1) {
    constPart = nc1->value;
    minuendIsConst = true;
  } else {
    newExprs.push_back(minuend);
  }

  // Simplify and collect all other constant expressions
  for (auto exprIt = std::begin(exprs) + 1; exprIt != std::end(exprs);
       ++exprIt) {
    shared_ptr<LogicalExpression> newExpr = (*exprIt)->simplify();
    auto nc = dynamic_pointer_cast<NumericConstant>(newExpr);
    if (nc) {
      constPart -= nc->value;
    } else {
      newExprs.push_back(newExpr);
    }
  }

  if (newExprs.empty()) {
    // All elements are constant!
    assert(minuendIsConst);
    return shared_ptr<NumericConstant>(new NumericConstant(constPart));
  }

  if (minuendIsConst) {
    // The minuend is a constant -> insert it at the beginning!
    assert(!newExprs.empty());
    newExprs.insert(newExprs.begin(), shared_ptr<NumericConstant>(
                                          new NumericConstant(constPart)));
  } else {
    // The minuend is not a constant, so the const part is a subtrahend
    assert(!newExprs.empty());
    if (!MathUtils::doubleIsEqual(constPart, 0.0)) {
      newExprs.push_back(
          shared_ptr<NumericConstant>(new NumericConstant(constPart * -1.0)));
    }
  }

  assert(newExprs.size() >= 1);

  return shared_ptr<Subtraction>(new Subtraction(newExprs));
}

shared_ptr<LogicalExpression> Subtraction::normalizeConstants() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeConstants());
  }
  auto result = shared_ptr<LogicalExpression>(new Subtraction(newExprs));
  return result;
}

shared_ptr<LogicalExpression> Subtraction::normalizeStateFluents() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeStateFluents());
  }
  auto result = shared_ptr<LogicalExpression>(new Subtraction(newExprs));
  return result;
}

void Subtraction::createEdge(vector<MEDDLY::dd_edge>& edges,
                             MEDDLY::forest* mdd,
                             map<int, int> const& fluentMDDIndex) const {
  vector<MEDDLY::dd_edge> edgesToSubtract;
  for (auto expr : this->exprs) {
    expr->createEdge(edgesToSubtract, mdd, fluentMDDIndex);
  }
  assert(edgesToSubtract.size() >= 1);
  MEDDLY::dd_edge firstResult(mdd);
  if (edgesToSubtract.size() == 1) {
    // Create an edge for constant zero and subtract the expression
    MEDDLY::dd_edge constant(mdd);
    mdd->createEdge(0, constant);
    edges.push_back(constant);
    MEDDLY::apply(MEDDLY::MINUS, constant, edgesToSubtract[0], firstResult);
  } else {
    MEDDLY::apply(MEDDLY::MINUS, edgesToSubtract[0], edgesToSubtract[1],
                  firstResult);
    MEDDLY::dd_edge result(mdd);
    for (size_t i = 2; i < edgesToSubtract.size(); ++i) {
      MEDDLY::apply(MEDDLY::MINUS, firstResult, edgesToSubtract[i], result);
      firstResult = result;
    }
  }
  // std::cout << "Subtraction" << std::endl;
  // firstResult.show(stdout,2);
  edges.push_back(firstResult);
}

void Subtraction::print(ostream& out) const {
  out << "-(";
  Connective::print(out);
  out << ")";
}

