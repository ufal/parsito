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

  virtual predicted_transition predict(const configuration& conf, const tree& golden, unsigned network_outcome) const override;
 private:
  const vector<string>& labels;
};

transition_oracle::predicted_transition transition_system_projective_oracle_static::predict(const configuration& conf, const tree& golden, unsigned /*network_outcome*/) const {
  // Use left if appropriate
  if (conf.stack.size() >= 2) {
    int parent = conf.stack[conf.stack.size() - 1];
    int child = conf.stack[conf.stack.size() - 2];
    if (golden.nodes[child].head == parent) {
      for (size_t i = 0; i < labels.size(); i++)
        if (golden.nodes[child].deprel == labels[i])
          return predicted_transition(1 + 2*i, 1 + 2*i);
    }
  }

  // Use right if appropriate
  if (conf.stack.size() >= 2) {
    int child = conf.stack[conf.stack.size() - 1];
    int parent = conf.stack[conf.stack.size() - 2];
    if (golden.nodes[child].head == parent) {
      for (size_t i = 0; i < labels.size(); i++)
        if (golden.nodes[child].deprel == labels[i])
          return predicted_transition(1 + 2*i + 1, 1 + 2*i + 1);
    }
  }

  // Otherwise, just shift
  return predicted_transition(0, 0);
}

// Oracle factory method
transition_oracle* transition_system_projective::oracle(const string& name) const {
  if (name == "static") return new transition_system_projective_oracle_static(labels);
  return nullptr;
}

} // namespace parsito
} // namespace ufal
