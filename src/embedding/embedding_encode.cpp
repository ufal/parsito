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

void embedding::save(binary_encoder& enc) const {
  // Save dimension and update_weight
  enc.add_4B(dimension);
  enc.add_double(update_weight);

  // Save the dictionary
  vector<string_piece> words(dictionary.size());
  for (auto&& entry : dictionary) {
    assert(entry.second >= 0 && entry.second < int(dictionary.size()));
    words[entry.second] = entry.first;
  }
  enc.add_4B(dictionary.size());
  for (auto&& word : words)
    enc.add_str(word);

  // Save the weights
  enc.add_data((const char*) weights.data(), sizeof(float) * dimension * dictionary.size());
}

void embedding::create(unsigned dimension, double update_weight, const vector<pair<string, vector<float>>>& words) {
  assert(update_weight >= 0);

  this->dimension = dimension;
  this->update_weight = update_weight;

  dictionary.clear();
  weights.clear();
  for (auto&& word : words) {
    assert(word.second.size() == dimension);
    dictionary.emplace(word.first, dictionary.size());
    weights.insert(weights.end(), word.second.begin(), word.second.end());
  }
}

} // namespace parsito
} // namespace ufal

