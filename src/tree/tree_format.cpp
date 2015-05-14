// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstring>

#include "tree_format.h"
#include "utils/input.h"

namespace ufal {
namespace parsito {

// Input CoNLL-U format
class tree_input_format_conllu : public tree_input_format {
 public:
  virtual bool read_block(FILE* f, string& block) const override;
  virtual void set_block(string_piece block) override;
  virtual bool next_tree(tree& t, string& error) override;

 private:
  string_piece block;
};

bool tree_input_format_conllu::read_block(FILE* f, string& block) const {
  return getpara(f, block);
}

void tree_input_format_conllu::set_block(string_piece block) {
  this->block = block;
}

bool tree_input_format_conllu::next_tree(tree& t, string& error) {
  t.clear();
  error.clear();

  string_piece line;
  vector<string_piece> tokens;
  while (getline(block, line)) {
    // Empty lines denote end of tree, unless at the beginning
    if (!line.len) {
      if (t.nodes.empty()) continue;
      break;
    }

    // Ignore comments
    if (*line.str == '#') continue;

    // Parse another tree node
    split(line, '\t', tokens);
    if (tokens.size() != 10)
      return error.assign("The CoNLL-U line '").append(line.str, line.len).append("' does not contain 10 columns!") , false;

    // Skip multiword tokens
    if (memchr(tokens[0].str, '-', tokens[0].len)) continue;

    // Parse node ID and head
    char* end_of_int;
    int id = strtol(tokens[0].str, &end_of_int, 10);
    if (end_of_int != tokens[0].str + tokens[0].len)
      return error.assign("Cannot parse numeric id value '").append(tokens[0].str, tokens[0].len).append("'!"), false;
    if (id != int(t.nodes.size()))
      return error.assign("Wrong numeric id value '").append(tokens[0].str, tokens[0].len).append("'!"), false;

    int head = strtol(tokens[6].str, &end_of_int, 10);
    if (end_of_int != tokens[6].str + tokens[6].len)
      return error.assign("Cannot parse numeric head value '").append(tokens[6].str, tokens[6].len).append("'!"), false;
    if (head < 0)
      return error.assign("Numeric head value '").append(tokens[0].str, tokens[0].len).append("' cannot be negative!"), false;

    // Add new node
    auto& node = t.add_node(string(tokens[1].str, tokens[1].len));
    if (!(tokens[2].len == 1 && tokens[2].str[0] == '_')) node.lemma.assign(tokens[2].str, tokens[2].len);
    if (!(tokens[3].len == 1 && tokens[3].str[0] == '_')) node.ctag.assign(tokens[3].str, tokens[3].len);
    if (!(tokens[4].len == 1 && tokens[4].str[0] == '_')) node.tag.assign(tokens[4].str, tokens[4].len);
    if (!(tokens[5].len == 1 && tokens[5].str[0] == '_')) node.feats.assign(tokens[5].str, tokens[5].len);
    node.head = head;
    if (!(tokens[7].len == 1 && tokens[7].str[0] == '_')) node.deprel.assign(tokens[7].str, tokens[7].len);
    if (!(tokens[8].len == 1 && tokens[8].str[0] == '_')) node.deps.assign(tokens[8].str, tokens[8].len);
    if (!(tokens[9].len == 1 && tokens[9].str[0] == '_')) node.misc.assign(tokens[9].str, tokens[9].len);
  }

  // Set heads correctly
  for (auto&& node : t.nodes)
    if (node.id)
      t.set_head(node.id, node.head);

  return !t.empty();
}

// Output CoNLL-U format
class tree_output_format_conllu : public tree_output_format {
 public:
  virtual void append_tree(const tree& t, string& block) const override;

 private:
  static const string underscore;
  const string& underscore_on_empty(const string& str) const { return str.empty() ? underscore : str; }
};
const string tree_output_format_conllu::underscore = "_";

void tree_output_format_conllu::append_tree(const tree& t, string& block) const {
  // Skip the root node
  for (size_t i = 1; i < t.nodes.size(); i++) {
    block.append(to_string(i)).push_back('\t');
    block.append(t.nodes[i].form).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].lemma)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].ctag)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].tag)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].feats)).push_back('\t');
    block.append(to_string(t.nodes[i].head)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].deprel)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].deps)).push_back('\t');
    block.append(underscore_on_empty(t.nodes[i].misc)).push_back('\n');
  }
  block.push_back('\n');
}

// Input Static factory methods
tree_input_format* tree_input_format::new_conllu_input_format() {
  return new tree_input_format_conllu();
}

tree_input_format* tree_input_format::new_input_format(const string& name) {
  if (name.compare("conllu") == 0) return new_conllu_input_format();
  return nullptr;
}

// Output static factory methods
tree_output_format* tree_output_format::new_conllu_output_format() {
  return new tree_output_format_conllu();
}

tree_output_format* tree_output_format::new_output_format(const string& name) {
  if (name.compare("conllu") == 0) return new_conllu_output_format();
  return nullptr;
}

} // namespace parsito
} // namespace ufal
