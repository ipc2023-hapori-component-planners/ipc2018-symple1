shared_ptr<LogicalExpression> Multiplication::multiplyOut() {
  // Convert all parts of the multiplication
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (shared_ptr<LogicalExpression> expr : exprs) {
    newExprs.push_back(expr->multiplyOut());
  }
  // The following part is to ensure that multiplications are always the most
  // inner part of a function, so that we can add a constant to each
  // multiplication without worrying about multiplying multiple constants
  //
  // Check if the first expression is an operator
  auto connective = dynamic_pointer_cast<Connective>(newExprs[0]);
  if (connective) {
    // If the first expression is another multiplication just append
    // all the inner factors at the end of the outer product and delete
    // the inner multiplication (the first expression)
    auto multiplication = dynamic_pointer_cast<Multiplication>(connective);
    if (multiplication) {
      for (auto expr : multiplication->exprs) {
        newExprs.push_back(expr);
      }
      std::iter_swap(std::begin(newExprs), std::end(newExprs) - 1);
      newExprs.pop_back();
      auto mult = shared_ptr<Multiplication>(new Multiplication(newExprs));
      return mult->multiplyOut();
    }
    // (a°b) * ... = (a*...) °  (b*...)
    vector<shared_ptr<LogicalExpression>> cParts;
    vector<shared_ptr<LogicalExpression>> multParts{std::begin(newExprs) + 1,
                                                    std::end(newExprs)};
    // Create multiplications for each connective expression
    for (shared_ptr<LogicalExpression> expr : connective->exprs) {
      multParts.push_back(expr);
      auto mult = shared_ptr<Multiplication>(new Multiplication(multParts));
      multParts.pop_back();
      cParts.push_back(mult);
    }
    // Create a new connective consisting of multiplications
    // TODO: Is there a better way than checking for each case, maybe
    // the CRTP pattern?
    auto addition = dynamic_pointer_cast<Addition>(connective);
    auto subtraction = dynamic_pointer_cast<Subtraction>(connective);
    if (addition) {
      auto result = shared_ptr<Addition>(new Addition(cParts));
      return result->multiplyOut();
    } else if (subtraction) {
      auto result = shared_ptr<Subtraction>(new Subtraction(cParts));
      return result->multiplyOut();
    } else {
      this->print(cout);
      assert(false);
    }
  } else {
    for (auto i = std::begin(newExprs) + 1; i < std::end(newExprs); ++i) {
      auto connective = dynamic_pointer_cast<Connective>(*i);
      if (connective) {
        // Put the connective as first expression and transform again
        std::iter_swap(std::begin(newExprs), i);
        auto mult = shared_ptr<Multiplication>(new Multiplication(newExprs));
        return mult->multiplyOut();
      }
    }
    // If no part is an operator we are done
    auto result = shared_ptr<Multiplication>(new Multiplication(newExprs));
    return result;
  }
  assert(false);
  return nullptr;
}

shared_ptr<LogicalExpression> Multiplication::simplify() {
  vector<shared_ptr<LogicalExpression>> exprWithoutMult;
  for (auto const& expr : exprs) {
    auto mult = dynamic_pointer_cast<Multiplication>(expr);
    if (mult) {
      for (auto const& part : mult->exprs) {
        exprWithoutMult.push_back(part->simplify());
      }
    } else {
      exprWithoutMult.push_back(expr->simplify());
    }
  }

  vector<shared_ptr<LogicalExpression>> newExprs;
  double constMult = 1.0;

  for (auto const& newExpr : exprWithoutMult) {
    auto nc = dynamic_pointer_cast<NumericConstant>(newExpr);
    if (nc) {
      if (MathUtils::doubleIsEqual(nc->value, 0.0)) {
        return shared_ptr<NumericConstant>(new NumericConstant(0.0));
      }
      constMult *= nc->value;
    } else {
      newExprs.push_back(newExpr);
    }
  }

  newExprs.push_back(
      shared_ptr<NumericConstant>(new NumericConstant(constMult)));

  if (newExprs.size() == 1) {
    return newExprs[0];
  }

  return shared_ptr<Multiplication>(new Multiplication(newExprs));
}

shared_ptr<LogicalExpression> Multiplication::normalizeConstants() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeConstants());
  }
  auto result = shared_ptr<LogicalExpression>(new Multiplication(newExprs));
  return result;
}

shared_ptr<LogicalExpression> Multiplication::normalizeStateFluents() {
  vector<shared_ptr<LogicalExpression>> newExprs;
  for (auto const& expr : exprs) {
    newExprs.push_back(expr->normalizeStateFluents());
  }
  auto result = shared_ptr<LogicalExpression>(new Multiplication(newExprs));
  return result;
}

void Multiplication::createEdge(vector<MEDDLY::dd_edge>& edges,
                                MEDDLY::forest* mdd,
                                map<int, int> const& fluentMDDIndex) const {
  vector<MEDDLY::dd_edge> edgesToMultiply;
  for (auto expr : this->exprs) {
    expr->createEdge(edgesToMultiply, mdd, fluentMDDIndex);
  }
  assert(edgesToMultiply.size() >= 2);
  MEDDLY::dd_edge firstResult(mdd);
  MEDDLY::apply(MEDDLY::MULTIPLY, edgesToMultiply[0], edgesToMultiply[1],
                firstResult);
  MEDDLY::dd_edge result(mdd);
  for (size_t i = 2; i < edgesToMultiply.size(); ++i) {
    MEDDLY::apply(MEDDLY::MULTIPLY, firstResult, edgesToMultiply[i], result);
    firstResult = result;
  }
  // std::cout << "Multiplication" << std::endl;
  // firstResult.show(stdout,2);
  edges.push_back(firstResult);
}

void Multiplication::print(ostream& out) const {
  out << "*(";
  Connective::print(out);
  out << ")";
}
