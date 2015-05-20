// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "transition_system.h"
#include "transition_system_projective.h"

namespace ufal {
namespace parsito {

unsigned transition_system::transition_count() const {
  return transitions.size();
}

bool transition_system::applicable(const configuration& conf, unsigned transition) const {
  assert(transition < transitions.size());

  return transitions[transition]->applicable(conf);
}

void transition_system::perform(configuration& conf, unsigned transition) const {
  assert(transition < transitions.size());

  transitions[transition]->perform(conf);
}

transition_system* transition_system::create(const string& name, const vector<string>& labels) {
  if (name == "projective") return new transition_system_projective(labels);
  return nullptr;
}

} // namespace parsito
} // namespace ufal

