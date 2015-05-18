// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "common.h"
#include "embedding.h"

namespace ufal {
namespace parsito {

int embedding::lookup_word(const string& word) const {
  auto it = dictionary.find(word);
  return it == dictionary.end() ? -1 : it->second;
}

float* embedding::weight(int id) {
  if (id < 0 || id * dimension >= weights.size()) return nullptr;
  return weights.data() + id * dimension;
}

const float* embedding::weight(int id) const {
  if (id < 0 || id * dimension >= weights.size()) return nullptr;
  return weights.data() + id * dimension;
}

void embedding::load(binary_decoder& data) {
  // Load dimemsion
  dimension = data.next_4B();

  // Load update weight
  update_weight = *data.next<double>(1);

  // Load dictionary
  dictionary.clear();
  string word;
  for (unsigned size = data.next_4B(); size; size--) {
    data.next_str(word);
    dictionary.emplace(word, dictionary.size());
  }

  // Load weights
  const float* weights_ptr = data.next<float>(dimension * dictionary.size());
  weights.assign(weights_ptr, weights_ptr + dimension * dictionary.size());
}

} // namespace parsito
} // namespace ufal

