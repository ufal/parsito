// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "tree_output_format.h"

namespace ufal {
namespace parsito {

// CoNLL-U format
class tree_output_format_conllu : public tree_output_format {
 public:
  virtual void append_tree(const tree& t, string& block) override;
};

void tree_output_format_conllu::append_tree(const tree& /*t*/, string& /*block*/) {
  // TODO
}

// Static factory methods
tree_output_format* tree_output_format::new_conllu_output_format() {
  return new tree_output_format_conllu();
}

tree_output_format* tree_output_format::new_output_format(const string& name) {
  if (name.compare("conllu") == 0) return new_conllu_output_format();
  return nullptr;
}


} // namespace parsito
} // namespace ufal
