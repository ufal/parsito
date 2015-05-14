// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>

#include "tree.h"

namespace ufal {
namespace parsito {

const string tree::root_form = "<root>";

tree::tree() {
 add_node(root_form);
}

bool tree::empty() {
  return nodes.size() == 1;
}

void tree::clear() {
  nodes.clear();
  add_node(root_form);
}

node& tree::add_node(const string& form) {
  nodes.emplace_back(nodes.size(), form);
  return nodes.back();
}

void tree::set_head(int id, int head) {
  if (id < 0 || id >= int(nodes.size())) return;
  if (head >= int(nodes.size())) return;

  // Remove existing head
  if (nodes[id].head >= 0) {
    auto& children = nodes[nodes[id].head].children;
    for (size_t i = children.size(); i && children[i-1] >= id; i--)
      if (children[i-1] == id) {
        children.erase(children.begin() + i - 1);
        break;
      }
  }

  // Set new head
  nodes[id].head = head;
  if (head >= 0) {
    auto& children = nodes[head].children;
    size_t i = children.size();
    while (i && children[i-1] > id) i--;
    if (!i || children[i-1] < id) children.insert(children.begin() + i, id);
  }
}


} // namespace parsito
} // namespace ufal
