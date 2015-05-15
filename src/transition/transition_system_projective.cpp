// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "transition.h"
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

transition_oracle* transition_system_projective::oracle(const string& /*name*/) const {
  return nullptr;
}

} // namespace parsito
} // namespace ufal
