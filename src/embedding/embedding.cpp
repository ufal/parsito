// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <limits>

#include "common.h"
#include "embedding.h"
#include "unilib/unicode.h"
#include "unilib/utf8.h"
#include "utils/binary_decoder.h"

namespace ufal {
namespace parsito {

using namespace unilib;

int embedding::lookup_word(const string& word, string& buffer) const {
  auto it = dictionary.find(word);
  if (it != dictionary.end()) return it->second;

  // We now apply several heuristics to find a match

  // Try locating uppercase/titlecase characters which we could lowercase
  bool first = true;
  unicode::category_t first_category = 0, other_categories = 0;
  for (auto&& chr : utf8::decoder(word)) {
    (first ? first_category : other_categories) |= unicode::category(chr);
    first = false;
  }

  if ((first_category & unicode::Lut) && (other_categories & unicode::Lut)) {
    // Lowercase all characters but the first
    buffer.clear();
    first = true;
    for (auto&& chr : utf8::decoder(word)) {
      utf8::append(buffer, first ? chr : unicode::lowercase(chr));
      first = false;
    }

    it = dictionary.find(buffer);
    if (it != dictionary.end()) return it->second;
  }

  if ((first_category & unicode::Lut) || (other_categories & unicode::Lut)) {
    utf8::map(unicode::lowercase, word, buffer);

    it = dictionary.find(buffer);
    if (it != dictionary.end()) return it->second;
  }

  // If the word starts with digit and contain only digits and non-letter characters
  // i.e. large number, date, time, try replacing it with first digit only.
  if ((first_category & unicode::N) && !(other_categories & unicode::L)) {
    buffer.clear();
    utf8::append(buffer, utf8::first(word));

    it = dictionary.find(buffer);
    if (it != dictionary.end()) return it->second;
  }

  return unknown_index;
}

int embedding::unknown_word() const {
  return unknown_index;
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

  updatable_index = numeric_limits<decltype(updatable_index)>::max();

  // Load dictionary
  dictionary.clear();
  string word;
  for (unsigned size = data.next_4B(); size; size--) {
    data.next_str(word);
    dictionary.emplace(word, dictionary.size());
  }

  unknown_index = data.next_1B() ? dictionary.size() : -1;

  // Load weights
  const float* weights_ptr = data.next<float>(dimension * (dictionary.size() + (unknown_index >= 0)));
  weights.assign(weights_ptr, weights_ptr + dimension * (dictionary.size() + (unknown_index >= 0)));
}

} // namespace parsito
} // namespace ufal

