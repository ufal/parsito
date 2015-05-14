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

unsigned transition_system_projective::transition_count() const {
  return transitions.size();
}

void transition_system_projective::init(configuration& c, tree& t, const tree* golden) {
  this->c = &c;
  this->t = &t;
  this->golden = golden;
}

void transition_system_projective::perform(unsigned transition) {
  if (c && t && transition < transitions.size())
    transitions[transition]->perform(*c, *t);
}

void transition_system_projective::losses(vector<int>& losses) {
  losses.clear();

  if (!c || !t || !golden || transitions.empty()) return;

  // TODO
}

void transition_system_projective::create(const vector<string>& labels) {
  transitions.clear();
  transitions.emplace_back(new transition_shift());
  for (auto&& label : labels) {
    transitions.emplace_back(new transition_left_arc(label));
    transitions.emplace_back(new transition_right_arc(label));
  }
}

} // namespace parsito
} // namespace ufal
