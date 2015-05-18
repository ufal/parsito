// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <unordered_map>

#include "common.h"
#include "utils/binary_decoder.h"
#include "utils/string_piece.h"

namespace ufal {
namespace parsito {

class embedding {
 public:
  unsigned dimension;
  double update_weight;

  int lookup_word(const string& word) const; // <0 is not_found
  float* weight(int id); // nullptr for unknown id
  const float* weight(int id) const; // nullpt for unknown id

  void load(binary_decoder& data);
 private:
  unordered_map<string, int> dictionary;
  vector<float> weights;
};

} // namespace parsito
} // namespace ufal
