// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "tree_format.h"
#include "utils/input.h"

namespace ufal {
namespace parsito {

// Input CoNLL-U format
class tree_input_format_conllu : public tree_input_format {
 public:
  virtual bool read_block(FILE* f, string& block) const override;
  virtual void set_block(string_piece block) override;
  virtual bool next_tree(tree& t) override;

 private:
  string_piece block;
  unsigned index;
};

bool tree_input_format_conllu::read_block(FILE* f, string& block) const {
  return getpara(f, block);
}

void tree_input_format_conllu::set_block(string_piece block) {
  this->block = block;
  index = 0;
}

bool tree_input_format_conllu::next_tree(tree& /*t*/) {
  // TODO
  return false;
}

// Output CoNLL-U format
class tree_output_format_conllu : public tree_output_format {
 public:
  virtual void append_tree(const tree& t, string& block) const override;
};

void tree_output_format_conllu::append_tree(const tree& t, string& block) const {
  for (size_t i = 0; i < t.nodes.size(); i++) {
    block.append(to_string(i + 1)).push_back('\t');
    block.append(t.nodes[i].form).push_back('\t');
    block.append(t.nodes[i].lemma).push_back('\t');
    block.append(t.nodes[i].ctag).push_back('\t');
    block.append(t.nodes[i].tag).push_back('\t');
    block.append(t.nodes[i].feats).push_back('\t');
    block.append(to_string(t.nodes[i].head)).push_back('\t');
    block.append(t.nodes[i].deprel).push_back('\t');
    block.append(t.nodes[i].deps).push_back('\t');
    block.append(t.nodes[i].misc).push_back('\n');
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
