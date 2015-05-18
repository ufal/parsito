// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "node_extractor.h"
#include "utils/parse_int.h"
#include "utils/split.h"

namespace ufal {
namespace parsito {

void node_extractor::extract(const configuration& c, vector<node*>& nodes) const {
  nodes.clear();
  for (auto&& selector : selectors) {
    // Start by locating starting node
    node* current = nullptr;
    switch (selector.start.first) {
      case STACK:
        if (selector.start.second < int(c.stack.size()))
          current = c.stack[c.stack.size() - 1 - selector.start.second];
        break;
      case BUFFER:
        if (selector.start.second < int(c.buffer.size()))
          current = c.buffer[c.buffer.size() - 1 - selector.start.second];
        break;
    }

    // Follow directions to the final node
    if (current)
      for (auto&& direction : selector.directions) {
        switch (direction.first) {
          case PARENT:
            current = current->head >= 0 ? &c.t->nodes[current->head] : nullptr;
            break;
          case CHILD:
            current = direction.second > 0 && direction.second < int(current->children.size()) ?
                        &c.t->nodes[current->children[direction.second]] :
                      direction.second < 0 && -direction.second <= int(current->children.size()) ?
                        &c.t->nodes[current->children[current->children.size() + direction.second]] :
                        nullptr;
            break;
        }
        if (!current) break;
      }

    // Add the selected node
    nodes.push_back(current);
  }
}

bool node_extractor::create(string_piece description, string& error) {
  selectors.clear();
  error.clear();

  vector<string_piece> lines, parts, words;
  split(description, '\n', lines);
  for (auto&& line : lines) {
    if (!line.len || line.str[0] == '#') continue;

    // Separate start and directions
    split(line, ',', parts);

    // Parse start
    split(parts[0], ' ', words);
    if (words.size() != 2)
      return error.assign("The node selector '").append(parts[0].str, parts[0].len).append("' on line '").append(line.str, line.len).append("' does not contain two space separated values!"), false;

    start_t start;
    if (words[0] == "stack")
      start = STACK;
    else if (words[0] == "buffer")
      start = BUFFER;
    else
      return error.assign("Cannot parse starting location '").append(words[0].str, words[0].len).append("' on line '").append(line.str, line.len).append(".!"), false;

    int start_index;
    if (!parse_int(words[1], "starting index", start_index, error)) return false;

    selectors.emplace_back(start, start_index);

    // Parse directions
    for (size_t i = 1; i < parts.size(); i++) {
      split(parts[i], ' ', words);
      if (words.empty())
        return error.assign("Empty node selector on line '").append(line.str, line.len).append(".!"), false;

      if (words[0] == "parent") {
        if (words.size() != 1)
          return error.assign("The node selector '").append(parts[i].str, parts[i].len).append("' on line '").append(line.str, line.len).append("' does not contain one space separated value!"), false;
        selectors.back().directions.emplace_back(PARENT, 0);
      } else if (words[0] == "child") {
        if (words.size() != 2)
          return error.assign("The node selector '").append(parts[i].str, parts[i].len).append("' on line '").append(line.str, line.len).append("' does not contain two space separated values!"), false;
        int child_index;
        if (!parse_int(words[1], "child index", child_index, error)) return false;
        selectors.back().directions.emplace_back(CHILD, child_index);
      } else {
        return error.assign("Cannot parse direction location '").append(words[0].str, words[0].len).append("' on line '").append(line.str, line.len).append(".!"), false;
      }
    }
  }

  return true;
}

} // namespace parsito
} // namespace ufal
