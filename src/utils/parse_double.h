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

// Try to parse an double from given string. If the double cannot be parsed or does
// not fit doubleo double, false is returned and the error string is filled using the
// value_name argument.
bool parse_double(string_piece str, const char* value_name, double& value, string& error);

// Try to parse an double from given string. If the double cannot be parsed or does
// not fit doubleo double, an error is displayed and program exits.
double parse_double(string_piece str, const char* value_name);

} // namespace parsito
} // namespace ufal
