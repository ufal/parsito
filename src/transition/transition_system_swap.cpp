// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "transition_system_swap.h"

namespace ufal {
namespace parsito {

transition_system_swap::transition_system_swap(const vector<string>& labels) : transition_system(labels) {
  transitions.emplace_back(new transition_shift());
  transitions.emplace_back(new transition_swap());
  for (auto&& label : labels) {
    transitions.emplace_back(new transition_left_arc(label));
    transitions.emplace_back(new transition_right_arc(label));
  }
}

// Static oracle
class transition_system_swap_oracle_static_eager : public transition_oracle {
 public:
  transition_system_swap_oracle_static_eager(const vector<string>& labels) : labels(labels) {}

  class tree_oracle_static_eager : public transition_oracle::tree_oracle {
   public:
    tree_oracle_static_eager(const vector<string>& labels, const tree& gold, vector<int>&& projective_order) : labels(labels), gold(gold), projective_order(projective_order) {}
    virtual predicted_transition predict(const configuration& conf, unsigned network_outcome, unsigned iteration) const override;
   private:
    const vector<string>& labels;
    const tree& gold;
    const vector<int> projective_order;
  };

  virtual unique_ptr<tree_oracle> create_tree_oracle(const tree& gold) const override;
 private:
  void create_projective_order(const tree& gold, int node, vector<int>& projective_order, int& projective_index) const;
  const vector<string>& labels;
};

unique_ptr<transition_oracle::tree_oracle> transition_system_swap_oracle_static_eager::create_tree_oracle(const tree& gold) const {
  vector<int> projective_order(gold.nodes.size());
  int projective_index;
  create_projective_order(gold, 0, projective_order, projective_index);
  return unique_ptr<transition_oracle::tree_oracle>(new tree_oracle_static_eager(labels, gold, move(projective_order)));
}

void transition_system_swap_oracle_static_eager::create_projective_order(const tree& gold, int node, vector<int>& projective_order, int& projective_index) const {
  unsigned child_index = 0;
  while (child_index < gold.nodes[node].children.size() && gold.nodes[node].children[child_index] < node)
    create_projective_order(gold, gold.nodes[node].children[child_index++], projective_order, projective_index);
  projective_order[node] = projective_index++;
  while (child_index < gold.nodes[node].children.size())
    create_projective_order(gold, gold.nodes[node].children[child_index++], projective_order, projective_index);
}

transition_oracle::predicted_transition transition_system_swap_oracle_static_eager::tree_oracle_static_eager::predict(const configuration& conf, unsigned /*network_outcome*/, unsigned /*iteration*/) const {
  // Use left if appropriate
  if (conf.stack.size() >= 2) {
    int parent = conf.stack[conf.stack.size() - 1];
    int child = conf.stack[conf.stack.size() - 2];
    if (gold.nodes[child].head == parent && gold.nodes[child].children.size() == conf.t->nodes[child].children.size()) {
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          return predicted_transition(2 + 2*i, 2 + 2*i);

      assert(!"label was not found");
    }
  }

  // Use right if appropriate
  if (conf.stack.size() >= 2) {
    int child = conf.stack[conf.stack.size() - 1];
    int parent = conf.stack[conf.stack.size() - 2];
    if (gold.nodes[child].head == parent && gold.nodes[child].children.size() == conf.t->nodes[child].children.size()) {
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          return predicted_transition(2 + 2*i + 1, 2 + 2*i + 1);

      assert(!"label was not found");
    }
  }

  // Use swap if appropriate
  if (conf.stack.size() >= 2) {
    int last = conf.stack[conf.stack.size() - 1];
    int prev = conf.stack[conf.stack.size() - 2];
    if (projective_order[last] < projective_order[prev])
      return predicted_transition(1, 1);
  }

  // Otherwise, just shift
  return predicted_transition(0, 0);
}

// Oracle factory method
transition_oracle* transition_system_swap::oracle(const string& name) const {
  if (name == "static_eager") return new transition_system_swap_oracle_static_eager(labels);
  return nullptr;
}

} // namespace parsito
} // namespace ufal
