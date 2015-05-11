// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "tree_input_format.h"
#include "utils/input.h"

namespace ufal {
namespace parsito {

// CoNLL-U format
class tree_input_format_conllu : public tree_input_format {
 public:
  virtual bool read_block(FILE* f, string& block) override;
  virtual void set_block(string_piece block) override;
  virtual bool next_tree(tree& t) override;

 private:
  string_piece block;
  unsigned index;
};

bool tree_input_format_conllu::read_block(FILE* f, string& block) {
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

// Static factory methods
tree_input_format* tree_input_format::new_conllu_input_format() {
  return new tree_input_format_conllu();
}

tree_input_format* tree_input_format::new_input_format(const string& name) {
  if (name.compare("conllu") == 0) return new_conllu_input_format();
  return nullptr;
}


} // namespace parsito
} // namespace ufal
