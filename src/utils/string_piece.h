// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>

#include "common.h"

namespace ufal {
namespace parsito {

class string_piece {
 public:
  const char* str;
  size_t len;

  string_piece() : str(nullptr), len(0) {}
  string_piece(const char* str) : str(str), len(strlen(str)) {}
  string_piece(const char* str, size_t len) : str(str), len(len) {}
  string_piece(const string& str) : str(str.c_str()), len(str.size()) {}
};

inline bool operator==(const string_piece& a, const string_piece& b) {
  return a.len == b.len && memcmp(a.str, b.str, a.len) == 0;
}

inline bool operator!=(const string_piece& a, const string_piece& b) {
  return a.len != b.len || memcmp(a.str, b.str, a.len) != 0;
}

inline ostream& operator<<(ostream& out, const string_piece& str) {
  return out.write(str.str, str.len);
}

} // namespace parsito
} // namespace ufal
