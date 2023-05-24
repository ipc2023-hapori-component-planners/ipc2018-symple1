#ifndef OPERATOR_H
#define OPERATOR_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "globals.h"
#include "state.h"
#include "sym_evmdd/meddly_extensions/userDefiniedOperators.h"

// include meddly for sdac representation (added by David)
#include <meddly.h>
#include <meddly_expert.h>

struct Prevail {
  int var;
  int prev;
  Prevail(std::istream &in);
  Prevail(int v, int p) :
  var(v), prev(p) {
  }

  bool is_applicable(const State &state) const {
    assert(var >= 0 && var < g_variable_name.size());
    assert(prev >= 0 && prev < g_variable_domain[var]);
    return state[var] == prev;
  }

  bool operator==(const Prevail &other) const {
    return var == other.var && prev == other.prev;
  }

  bool operator!=(const Prevail &other) const {
    return !(*this == other);
  }

  void dump() const;
};

struct PrePost {
  int var;
  int pre, post;
  std::vector<Prevail> cond;
  PrePost() {
  } // Needed for axiom file-reading constructor, unfortunately.
  PrePost(std::istream &in);
  PrePost(int v, int pr, int po, const std::vector<Prevail> &co) :
  var(v), pre(pr), post(po), cond(co) {
  }

  bool is_applicable(const State &state) const {
    assert(var >= 0 && var < g_variable_name.size());
    assert(pre == -1 || (pre >= 0 && pre < g_variable_domain[var]));
    return pre == -1 || state[var] == pre;
  }

  bool does_fire(const State &state) const {
    for (int i = 0; i < cond.size(); i++)
      if (!cond[i].is_applicable(state))
        return false;
    return true;
  }

  void dump() const;
};

class Operator {
  bool is_an_axiom;
  std::vector<Prevail> prevail;      // var, val
  std::vector<PrePost> pre_post;     // var, old-val, new-val, effect conditions
  std::string name;
  int cost;

  // Added by: David
  std::string costFunction; // string of action costs
  std::shared_ptr<MEDDLY::dd_edge> costEVMDD; // EVMDD to represent sdac
  std::shared_ptr<MEDDLY::dd_edge> opEVMDD; // EVMDD to represent sdac

  mutable bool marked; // Used for short-term marking of preferred operators
  public:
  Operator(std::istream &in, bool is_axiom);
  // dummy for sdac (added by speckd)
  Operator(std::string name, int cost) {
    this->name = name;
    this->cost = cost;
  }
  void dump() const;
  std::string get_name() const {
    return name;
  }

  bool is_axiom() const {
    return is_an_axiom;
  }

  const std::vector<Prevail> &get_prevail() const {
    return prevail;
  }
  const std::vector<PrePost> &get_pre_post() const {
    return pre_post;
  }

  bool is_applicable(const State &state) const {
    for (int i = 0; i < prevail.size(); i++)
      if (!prevail[i].is_applicable(state))
        return false;
    for (int i = 0; i < pre_post.size(); i++)
      if (!pre_post[i].is_applicable(state))
        return false;
    return true;
  }

  bool is_marked() const {
    return marked;
  }
  void mark() const {
    marked = true;
  }
  void unmark() const {
    marked = false;
  }

  mutable bool marker1, marker2; // HACK! HACK!

  int get_cost() const {
    return cost;
  }

  int get_cost(const State& s) const {
    //std::cout << "call" << std::endl;
    if (!domain_has_sdac()) return get_cost();
    std::vector<int> vals;
    vals.push_back(0);
    for (int i = 0; i < g_variable_domain.size(); i++) {
      vals.push_back(MEDDLY::DONT_CARE);
      vals.push_back(static_cast<int>(s.get_buffer()[i]));
      //std::cout << g_variable_name[i] << ": " << static_cast<int>(s.get_buffer()[i]) << std::endl;
    }
    /*MEDDLY::dd_edge test(*get_SDAC_cost());
    MEDDLY::FILE_output out(stdout);
    test.show(out, 3);
    std::cout << "=> " << evaluatePath(*get_SDAC_cost(), vals) << std::endl;*/
    return evaluatePath(*get_SDAC_cost(), vals);
  }

  std::string get_SDAC_cost_function() const {
    return this->costFunction;
  }

  // Returns SDAC EVMDD if definied.
  shared_ptr<MEDDLY::dd_edge> get_SDAC_cost() const {
    if (costEVMDD == nullptr) {
      fprintf(stderr, "CostEVMDD of %s not definied.",
      this->get_name().c_str());
      throw "costEVMDD not definied!";
    }
    return costEVMDD;
  }

  // Define op representation if definied
  void set_SDAC_cost(shared_ptr<MEDDLY::dd_edge> costEVMDD) {
    this->costEVMDD = costEVMDD;
  }

  // Returns op as EVMDD if definied.
  shared_ptr<MEDDLY::dd_edge> get_op_evmdd() const {
    if (opEVMDD == nullptr) {
      fprintf(stderr, "Op as EVMDD of %s not definied.",
      this->get_name().c_str());
      throw "opEVMDD not definied!";
    }
    return opEVMDD;
  }

  // Define SDAC EVMDD
  void set_op_evmdd(shared_ptr<MEDDLY::dd_edge> opEVMDD) {
    this->opEVMDD = opEVMDD;
  }

  // Set cost (to achive the correct cost after sdac eval)
  void set_cost(int cost) {
    this->cost = cost;
  }

  // Set cost (to achive the correct cost after sdac eval)
  void set_cost_function(std::string cost_function) {
    this->costFunction = cost_function;
  }

  bool has_conditional_effects() {
    for (int i = 0; i < get_pre_post().size(); i++) {
        if (get_pre_post()[i].cond.size()) {
          return true;
        }
    }
    return false;
  }

};

#endif
