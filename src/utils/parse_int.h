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

// Try to parse an int from given string. If the int cannot be parsed or does
// not fit into int, false is returned and the error string is filled using the
// value_name argument.
bool parse_int(string_piece str, const char* value_name, int& value, string& error);

// Try to parse an int from given string. If the int cannot be parsed or does
// not fit into int, an error is displayed and program exits.
int parse_int(string_piece str, const char* value_name);

} // namespace parsito
} // namespace ufal
