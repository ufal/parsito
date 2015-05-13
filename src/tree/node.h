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

namespace ufal {
namespace parsito {

class node {
 public:
  string form;
  string lemma;
  string tag;
  string ctag; // universal part-of-speech tag
  string feats; // list of Morphological features
  unsigned head; // head, 0 is root (head of root is root)
  string deprel; // dependency relation to the HEAD
  string deps; // secondary dependencies
  string misc; // miscellaneous information

  node() : head(0) {}
  node(const string& form) : form(form), head(0) {}
};

} // namespace parsito
} // namespace ufal
