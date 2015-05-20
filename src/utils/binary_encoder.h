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
#include "string_piece.h"

namespace ufal {
namespace parsito {

// Declaration
class binary_encoder {
 public:
  inline binary_encoder();

  inline void add_1B(unsigned val);
  inline void add_2B(unsigned val);
  inline void add_4B(unsigned val);
  inline void add_double(double val);
  inline void add_str(string_piece str);
  template <class T> inline void add(const T* data, unsigned elements);

  vector<unsigned char> data;
};


// Definition
binary_encoder::binary_encoder() {
  data.reserve(16);
}

void binary_encoder::add_1B(unsigned val) {
  if (uint8_t(val) != val) runtime_failure("Should encode value '" << val << "' in one byte!");
  data.push_back(val);
}

void binary_encoder::add_2B(unsigned val) {
  if (uint16_t(val) != val) runtime_failure("Should encode value '" << val << "' in two bytes!");
  data.insert(data.end(), (unsigned char*) &val, ((unsigned char*) &val) + sizeof(uint16_t));
}

void binary_encoder::add_4B(unsigned val) {
  if (uint32_t(val) != val) runtime_failure("Should encode value '" << val << "' in four bytes!");
  data.insert(data.end(), (unsigned char*) &val, ((unsigned char*) &val) + sizeof(uint32_t));
}

void binary_encoder::add_double(double val) {
  data.insert(data.end(), (unsigned char*) &val, ((unsigned char*) &val) + sizeof(double));
}


void binary_encoder::add_str(string_piece str) {
  if (str.len < 255) {
    data.push_back(str.len);
  } else {
    data.push_back(255);
    add_4B(str.len);
  }

  while (str.len--)
    data.push_back(*str.str++);
}

template <class T>
void binary_encoder::add(const T* data, unsigned elements) {
  this->data.insert(this->data.end(), (const unsigned char*) data, (const unsigned char*) (data + elements));
}

} // namespace parsito
} // namespace ufal
