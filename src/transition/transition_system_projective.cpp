// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "transition_system_projective.h"

namespace ufal {
namespace parsito {

transition_system_projective::transition_system_projective(const vector<string>& labels) : transition_system(labels) {
  transitions.emplace_back(new transition_shift());
  for (auto&& label : labels) {
    transitions.emplace_back(new transition_left_arc(label));
    transitions.emplace_back(new transition_right_arc(label));
  }
}

// Static oracle
class transition_system_projective_oracle_static : public transition_oracle {
 public:
  transition_system_projective_oracle_static(const vector<string>& labels) : labels(labels) {}

  class tree_oracle_static : public transition_oracle::tree_oracle {
   public:
    tree_oracle_static(const vector<string>& labels, const tree& gold) : labels(labels), gold(gold) {}
    virtual predicted_transition predict(const configuration& conf, unsigned network_outcome, unsigned iteration) const override;
    virtual void interesting_transitions(const configuration& conf, vector<unsigned>& transitions) const override;
   private:
    const vector<string>& labels;
    const tree& gold;
  };

  virtual unique_ptr<tree_oracle> create_tree_oracle(const tree& gold) const override;
 private:
  const vector<string>& labels;
};

unique_ptr<transition_oracle::tree_oracle> transition_system_projective_oracle_static::create_tree_oracle(const tree& gold) const {
  return unique_ptr<transition_oracle::tree_oracle>(new tree_oracle_static(labels, gold));
}

void transition_system_projective_oracle_static::tree_oracle_static::interesting_transitions(const configuration& conf, vector<unsigned>& transitions) const {
  transitions.clear();
  if (!conf.buffer.empty()) transitions.push_back(0);
  if (conf.stack.size() >= 2)
    for (int direction = 0; direction < 2; direction++) {
      int child = conf.stack[conf.stack.size() - 2 + direction];
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          transitions.push_back(1 + 2*i + direction);
    }
}

transition_oracle::predicted_transition transition_system_projective_oracle_static::tree_oracle_static::predict(const configuration& conf, unsigned /*network_outcome*/, unsigned /*iteration*/) const {
  // Use left if appropriate
  if (conf.stack.size() >= 2) {
    int parent = conf.stack[conf.stack.size() - 1];
    int child = conf.stack[conf.stack.size() - 2];
    if (gold.nodes[child].head == parent) {
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          return predicted_transition(1 + 2*i, 1 + 2*i);

      assert(!"label was not found");
    }
  }

  // Use right if appropriate
  if (conf.stack.size() >= 2) {
    int child = conf.stack[conf.stack.size() - 1];
    int parent = conf.stack[conf.stack.size() - 2];
    if (gold.nodes[child].head == parent &&
        (conf.buffer.empty() || gold.nodes[child].children.empty() || gold.nodes[child].children.back() < conf.buffer.back())) {
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          return predicted_transition(1 + 2*i + 1, 1 + 2*i + 1);

      assert(!"label was not found");
    }
  }

  // Otherwise, just shift
  return predicted_transition(0, 0);
}

// Dynamic oracle
class transition_system_projective_oracle_dynamic : public transition_oracle {
 public:
  transition_system_projective_oracle_dynamic(const vector<string>& labels) : labels(labels) {}

  class tree_oracle_dynamic : public transition_oracle::tree_oracle {
   public:
    tree_oracle_dynamic(const vector<string>& labels, const tree& gold) : labels(labels), gold(gold), oracle_static(labels, gold) {}
    virtual predicted_transition predict(const configuration& conf, unsigned network_outcome, unsigned iteration) const override;
    virtual void interesting_transitions(const configuration& conf, vector<unsigned>& transitions) const override;
   private:
    const vector<string>& labels;
    const tree& gold;
    transition_system_projective_oracle_static::tree_oracle_static oracle_static;
  };

  virtual unique_ptr<tree_oracle> create_tree_oracle(const tree& gold) const override;
 private:
  const vector<string>& labels;
};

unique_ptr<transition_oracle::tree_oracle> transition_system_projective_oracle_dynamic::create_tree_oracle(const tree& gold) const {
  return unique_ptr<transition_oracle::tree_oracle>(new tree_oracle_dynamic(labels, gold));
}

void transition_system_projective_oracle_dynamic::tree_oracle_dynamic::interesting_transitions(const configuration& conf, vector<unsigned>& transitions) const {
  transitions.clear();
  if (!conf.buffer.empty()) transitions.push_back(0);
  if (conf.stack.size() >= 2)
    for (int direction = 0; direction < 2; direction++) {
      int child = conf.stack[conf.stack.size() - 2 + direction];
      for (size_t i = 0; i < labels.size(); i++)
        if (gold.nodes[child].deprel == labels[i])
          transitions.push_back(1 + 2*i + direction);
    }
}

transition_oracle::predicted_transition transition_system_projective_oracle_dynamic::tree_oracle_dynamic::predict(const configuration& conf, unsigned network_outcome, unsigned iteration) const {
  if (iteration <= 1) return oracle_static.predict(conf, network_outcome, iteration);

}

// Oracle factory method
transition_oracle* transition_system_projective::oracle(const string& name) const {
  if (name == "static") return new transition_system_projective_oracle_static(labels);
  if (name == "dynamic") return new transition_system_projective_oracle_dynamic(labels);
  return nullptr;
}

} // namespace parsito
} // namespace ufal
