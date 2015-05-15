// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstring>

#include "split.h"

namespace ufal {
namespace parsito {

void split(const string& text, char sep, vector<string>& tokens) {
  tokens.clear();
  if (text.empty()) return;

  string::size_type index = 0;
  for (string::size_type next; (next = text.find(sep, index)) != string::npos; index = next + 1)
    tokens.emplace_back(text, index, next - index);

  tokens.emplace_back(text, index);
}

void split(string_piece text, char sep, vector<string_piece>& tokens) {
  tokens.clear();
  if (!text.len) return;

  const char* str = text.str;
  for (const char* next; (next = (const char*) memchr(str, sep, text.str + text.len - str)); str = next + 1)
    tokens.emplace_back(str, next - str);

  tokens.emplace_back(str, text.str + text.len - str);
}

} // namespace parsito
} // namespace ufal