shared_ptr<LogicalExpression> Addition::simplify() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  double constSum = 0.0;

  // Simplify and collect all constant expressions
  for (auto const& expr : exprs) {
    shared_ptr<LogicalExpression> newExpr = expr->simplify();
    auto nc = dynamic_pointer_cast<NumericConstant>(newExpr);
    if (nc) {
      constSum += nc->value;
    } else {
      newExprs.push_back(newExpr);
    }
  }

  if (newExprs.empty() && MathUtils::doubleIsEqual(constSum, 0.0)) {
    return shared_ptr<NumericConstant>(new NumericConstant(0.0));
  } else if (newExprs.empty() && !MathUtils::doubleIsEqual(constSum, 0.0)) {
    return shared_ptr<NumericConstant>(new NumericConstant(constSum));
  } else if (newExprs.size() == 1 && MathUtils::doubleIsEqual(constSum, 0.0)) {
    return newExprs[0];
  }

  // Add the constant part of the addition
  if (!MathUtils::doubleIsEqual(constSum, 0.0)) {
    newExprs.push_back(
        shared_ptr<NumericConstant>(new NumericConstant(constSum)));
  }

  return shared_ptr<Addition>(new Addition(newExprs));
}

shared_ptr<LogicalExpression> Addition::multiplyOut() {
  vector<shared_ptr<LogicalExpression>> multipliedExprs;
  for (auto expr : exprs) {
    multipliedExprs.push_back(expr->multiplyOut());
  }
  return shared_ptr<Addition>(new Addition(multipliedExprs));
}

shared_ptr<LogicalExpression> Addition::normalizeConstants() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeConstants());
  }
  auto result = shared_ptr<LogicalExpression>(new Addition(newExprs));
  return result;
}

shared_ptr<LogicalExpression> Addition::normalizeStateFluents() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeStateFluents());
  }
  auto result = shared_ptr<LogicalExpression>(new Addition(newExprs));
  return result;
}

void Addition::createEdge(vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                          map<int, int> const& fluentMDDIndex) const {
  vector<MEDDLY::dd_edge> edgesToAdd;
  for (auto expr : this->exprs) {
    expr->createEdge(edgesToAdd, mdd, fluentMDDIndex);
  }
  assert(edgesToAdd.size() >= 2);
  MEDDLY::dd_edge firstResult(mdd);
  MEDDLY::apply(MEDDLY::PLUS, edgesToAdd[0], edgesToAdd[1], firstResult);
  MEDDLY::dd_edge result(mdd);
  // firstResult.show(stdout,2);

  for (size_t i = 2; i < edgesToAdd.size(); ++i) {
    MEDDLY::apply(MEDDLY::PLUS, firstResult, edgesToAdd[i], result);
    firstResult = result;
  }
  // std::cout << "Addition" << std::endl;
  // firstResult.show(stdout,2);
  edges.push_back(firstResult);
}

void Addition::print(ostream& out) const {
  out << "+(";
  Connective::print(out);
  out << ")";
}
