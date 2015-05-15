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

transition_system_projective::transition_system_projective(const vector<string>& labels) : labels(labels) {
  transitions.emplace_back(new transition_shift());
  for (auto&& label : labels) {
    transitions.emplace_back(new transition_left_arc(label));
    transitions.emplace_back(new transition_right_arc(label));
  }
}

unsigned transition_system_projective::transition_count() const {
  return transitions.size();
}

void transition_system_projective::perform(configuration& c, tree& t, unsigned transition) const {
  if (transition < transitions.size())
    transitions[transition]->perform(c, t);
}

// Static oracle
class transition_system_projective_oracle_static : public transition_oracle {
 public:
  transition_system_projective_oracle_static(const vector<string>& labels) : labels(labels) {}

  virtual void outcomes(const configuration& c, const tree& t, const tree& golden, const vector<double>& predictions, vector<double>& outcomes) const override;
 private:
  const vector<string>& labels;
};

void transition_system_projective_oracle_static::outcomes(const configuration& c, const tree& /*t*/, const tree& golden, const vector<double>& /*predictions*/, vector<double>& outcomes) const {
  outcomes.assign(1 + 2 * labels.size(), 0);

  // Use left if appropriate
  if (c.stack.size() >= 2) {
    node* parent = c.stack[c.stack.size() - 1];
    node* child = c.stack[c.stack.size() - 2];
    if (golden.nodes[child->id].head == parent->id) {
      for (size_t i = 0; i < labels.size(); i++)
        if (golden.nodes[child->id].deprel == labels[i]) {
          outcomes[1 + 2*i] = 1;
          return;
        }
    }
  }

  // Use right if appropriate
  if (c.stack.size() >= 2) {
    node* child = c.stack[c.stack.size() - 1];
    node* parent = c.stack[c.stack.size() - 2];
    if (golden.nodes[child->id].head == parent->id) {
      for (size_t i = 0; i < labels.size(); i++)
        if (golden.nodes[child->id].deprel == labels[i]) {
          outcomes[1 + 2*i + 1] = 1;
          return;
        }
    }
  }

  // Otherwise, just shift
  outcomes[0] = 1;
}

// Oracle factory method
transition_oracle* transition_system_projective::oracle(const string& name) const {
  if (name.compare("static") == 0) return new transition_system_projective_oracle_static(labels);
  return nullptr;
}

} // namespace parsito
} // namespace ufal