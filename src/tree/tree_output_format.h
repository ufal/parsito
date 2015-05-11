// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "common.h"
#include "tree.h"

namespace ufal {
namespace parsito {

class tree_output_format {
 public:
  virtual ~tree_output_format() {}

  virtual void append_tree(const tree& t, string& block) = 0;

  // Static factory methods
  static tree_output_format* new_output_format(const string& name);

  static tree_output_format* new_conllu_output_format();
};

} // namespace parsito
} // namespace ufal
