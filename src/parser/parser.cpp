// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <fstream>

#include "parser.h"
#include "parser_nn.h"
#include "utils/compressor.h"

namespace ufal {
namespace parsito {

parser* parser::load(const char* file) {
  ifstream in(file, ifstream::in | ifstream::binary);
  if (!in.is_open()) return nullptr;
  return load(in);
}

parser* parser::load(istream& in) {
  unique_ptr<parser> result;

  binary_decoder data;
  if (!compressor::load(in, data)) return nullptr;

  try {
    unsigned name_len = data.next_1B();
    string name(data.next<char>(name_len), name_len);

    result.reset(create(name));
    if (!result) return nullptr;

    result->load(data);
  } catch (binary_decoder_error&) {
    return nullptr;
  }

  return result && data.is_end() ? result.release() : nullptr;
}

parser* parser::create(const string& name) {
  if (name.compare("nn") == 0) return new parser_nn();
  return nullptr;
}

} // namespace parsito
} // namespace ufal