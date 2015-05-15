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
  return c.stack.size() >= 2 && c.stack[c.stack.size() - 2]->id;
}

void transition_left_arc::perform(configuration& c, tree& t) const {
  if (!applicable(c)) return;
  node* parent = c.stack.back(); c.stack.pop_back();
  node* child = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(parent);
  t.set_head(child->id, parent->id, label);
}

// Right arc
bool transition_right_arc::applicable(const configuration& c) const {
  return c.stack.size() >= 2;
}

void transition_right_arc::perform(configuration& c, tree& t) const {
  if (!applicable(c)) return;
  node* child = c.stack.back(); c.stack.pop_back();
  node* parent = c.stack.back();
  t.set_head(child->id, parent->id, label);
}

// Shift
bool transition_shift::applicable(const configuration& c) const {
  return !c.buffer.empty();
}

void transition_shift::perform(configuration& c, tree& /*t*/) const {
  if (!applicable(c)) return;
  c.stack.push_back(c.buffer.back());
  c.buffer.pop_back();
}

// Swap
bool transition_swap::applicable(const configuration& c) const {
  return c.stack.size() >= 2 && c.stack[c.stack.size() - 2]->id && c.stack[c.stack.size() - 2]->id < c.stack[c.stack.size() - 1]->id;
}

void transition_swap::perform(configuration& c, tree& /*t*/) const {
  if (!applicable(c)) return;
  node* top = c.stack.back(); c.stack.pop_back();
  node* to_buffer = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(top);
  c.buffer.push_back(to_buffer);
}

// Left arc 2
bool transition_left_arc_2::applicable(const configuration& c) const {
  return c.stack.size() >= 3 && c.stack[c.stack.size() - 3]->id;
}

void transition_left_arc_2::perform(configuration& c, tree& t) const {
  if (!applicable(c)) return;
  node* parent = c.stack.back(); c.stack.pop_back();
  node* ignore = c.stack.back(); c.stack.pop_back();
  node* child = c.stack.back(); c.stack.pop_back();
  c.stack.push_back(ignore);
  c.stack.push_back(parent);
  t.set_head(child->id, parent->id, label);
}

// Right arc 2
bool transition_right_arc_2::applicable(const configuration& c) const {
  return c.stack.size() >= 3;
}

void transition_right_arc_2::perform(configuration& c, tree& t) const {
  if (!applicable(c)) return;
  node* child = c.stack.back(); c.stack.pop_back();
  node* to_buffer = c.stack.back(); c.stack.pop_back();
  node* parent = c.stack.back();
  c.buffer.push_back(to_buffer);
  t.set_head(child->id, parent->id, label);
}

} // namespace parsito
} // namespace ufal
