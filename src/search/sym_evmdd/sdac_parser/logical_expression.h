#ifndef LOGICAL_EXPRESSION_H
#define LOGICAL_EXPRESSION_H

#include <meddly.h>

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

class StateFluent;

class LogicalExpression
    : public std::enable_shared_from_this<LogicalExpression> {
public:
  virtual ~LogicalExpression() {}

  // Ensures that multiplications are always the most inner part of a
  // connective (besides constants and fluents). We need this for normalizing
  // floating point constants.
  virtual std::shared_ptr<LogicalExpression> multiplyOut();

  // Combines various constants in a connective to one constant
  virtual std::shared_ptr<LogicalExpression> simplify();

  // Multiplies constants with a multiplier. We can later divide by this
  // multiplier to get rounded integers for EVMDD library meddly.
  virtual std::shared_ptr<LogicalExpression> normalizeConstants();

  // StateFluents result in multiplications with 1.0, so that they can be
  // multiplied later on
  virtual std::shared_ptr<LogicalExpression> normalizeStateFluents();

  // Collects the state fluents of the formula 
  virtual void
  fluents(std::set<std::shared_ptr<StateFluent const>>& fluents) const;

  // Creates an edge for this logical expression in the mdd and stores the
  // created edge in a vector.
  virtual void createEdge(std::vector<MEDDLY::dd_edge>& edges,
                          MEDDLY::forest* mdd,
                          std::map<int, int> const& fluentMDDIndex) const;

  virtual void print(std::ostream& out) const;

  // Multiplier for normalization of constants
  static int constantMultiplier;
};

class StateFluent : public LogicalExpression {
public:
  // Creates a new fluent object or returns the pointer to the already
  // existing fluent
  static std::shared_ptr<LogicalExpression> create(std::string const& index);

  int index;
  std::string name;

  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void fluents(std::set<std::shared_ptr<StateFluent const>>& fluents) const;
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;
  void print(std::ostream& out) const;

  static std::map<std::string, std::shared_ptr<StateFluent>> const& getFluentIndices();

private:
  StateFluent() : index(0), name("") {}

  StateFluent(std::string const& _name) : name(_name) {}

  // Lookup to avoid creation of duplicate fluents
  static std::map<std::string, std::shared_ptr<StateFluent>>
      fluentIndices;
};

class NumericConstant : public LogicalExpression {

public:
  NumericConstant(double _value) : value(_value) {}

  double value;
  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;
  void print(std::ostream& out) const;
};

// Connectives are logical expressions connected by some operator, like +,*,-
class Connective : public LogicalExpression {

public:
  Connective(std::vector<std::shared_ptr<LogicalExpression>>& _exprs)
      : exprs(_exprs) {}

  std::vector<std::shared_ptr<LogicalExpression>> exprs;

  void fluents(std::set<std::shared_ptr<StateFluent const>>& fluents) const;
  void print(std::ostream& out) const;
};

class Addition : public Connective {

public:
  Addition(std::vector<std::shared_ptr<LogicalExpression>>& _exprs)
      : Connective(_exprs) {}

  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> simplify();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;

  void print(std::ostream& out) const;
};

class Subtraction : public Connective {

public:
  Subtraction(std::vector<std::shared_ptr<LogicalExpression>>& _exprs)
      : Connective(_exprs) {}

  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> simplify();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;
  void print(std::ostream& out) const;
};

class Multiplication : public Connective {

public:
  Multiplication(std::vector<std::shared_ptr<LogicalExpression>>& _exprs)
      : Connective(_exprs) {}

  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> simplify();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;
  void print(std::ostream& out) const;
};

class AbsolutValue : public Connective {

public:
  AbsolutValue(std::vector<std::shared_ptr<LogicalExpression>>& _exprs)
      : Connective(_exprs) {}

  std::shared_ptr<LogicalExpression> multiplyOut();
  std::shared_ptr<LogicalExpression> simplify();
  std::shared_ptr<LogicalExpression> normalizeConstants();
  std::shared_ptr<LogicalExpression> normalizeStateFluents();
  void createEdge(std::vector<MEDDLY::dd_edge>& edges, MEDDLY::forest* mdd,
                  std::map<int, int> const& fluentMDDIndex) const;
  void print(std::ostream& out) const;
};

#endif /* LOGICAL_EXPRESSION_H */
