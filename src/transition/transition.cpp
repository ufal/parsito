// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "transition.h"

namespace ufal {
namespace parsito {

// Left arc
bool transition_left_arc::applicable(const configuration& c) const {
  return c.stack.size() >= 2 && c.stack[c.stack.size() - 2];
}

void transition_left_arc::perform(configuration& c) const {
  assert(applicable(c));

  int parent = c.stack.back(); c.stack.pop_back();
  int child = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(parent);
  c.t->set_head(child, parent, label);
}

// Right arc
bool transition_right_arc::applicable(const configuration& c) const {
  return c.stack.size() >= 2;
}

void transition_right_arc::perform(configuration& c) const {
  assert(applicable(c));

  int child = c.stack.back(); c.stack.pop_back();
  int parent = c.stack.back();
  c.t->set_head(child, parent, label);
}

// Shift
bool transition_shift::applicable(const configuration& c) const {
  return !c.buffer.empty();
}

void transition_shift::perform(configuration& c) const {
  assert(applicable(c));

  c.stack.push_back(c.buffer.back());
  c.buffer.pop_back();
}

// Swap
bool transition_swap::applicable(const configuration& c) const {
  return c.stack.size() >= 2 && c.stack[c.stack.size() - 2] && c.stack[c.stack.size() - 2] < c.stack[c.stack.size() - 1];
}

void transition_swap::perform(configuration& c) const {
  assert(applicable(c));

  int top = c.stack.back(); c.stack.pop_back();
  int to_buffer = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(top);
  c.buffer.push_back(to_buffer);
}

// Left arc 2
bool transition_left_arc_2::applicable(const configuration& c) const {
  return c.stack.size() >= 3 && c.stack[c.stack.size() - 3];
}

void transition_left_arc_2::perform(configuration& c) const {
  assert(applicable(c));

  int parent = c.stack.back(); c.stack.pop_back();
  int ignore = c.stack.back(); c.stack.pop_back();
  int child = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(ignore);
  c.stack.push_back(parent);
  c.t->set_head(child, parent, label);
}

// Right arc 2
bool transition_right_arc_2::applicable(const configuration& c) const {
  return c.stack.size() >= 3;
}

void transition_right_arc_2::perform(configuration& c) const {
  assert(applicable(c));

  int child = c.stack.back(); c.stack.pop_back();
  int to_buffer = c.stack.back(); c.stack.pop_back();
  int parent = c.stack.back();
  c.buffer.push_back(to_buffer);
  c.t->set_head(child, parent, label);
}

} // namespace parsito
} // namespace ufal
