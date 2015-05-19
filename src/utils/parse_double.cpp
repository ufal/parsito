// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>

#include "parse_double.h"

namespace ufal {
namespace parsito {

bool parse_double(string_piece str, const char* value_name, double& value, string& error) {
  string_piece original = str;

  // Skip spaces
  while (str.len && (str.str[0] == ' ' || str.str[0] == '\f' || str.str[0] == '\n' || str.str[0] == '\r' || str.str[0] == '\t' || str.str[0] == '\v'))
    str.str++, str.len--;

  // Allow minus
  bool positive = true;
  if (str.len && str.str[0] == '-') {
    positive = false;
    str.str++, str.len--;
  }

  // Parse value, checking for overflow/underflow
  if (!str.len) return error.assign("No non-space character found when parsing ").append(value_name).append(" double value!"), false;
  if (!(str.str[0] >= '0' || str.str[0] <= '9')) return error.assign("Non-digit character found when parsing ").append(value_name).append(" double value '").append(original.str, original.len).append("'!"), false;

  value = 0;
  while (str.len && str.str[0] >= '0' && str.str[0] <= '9') {
    value = 10 * value + (str.str[0] - '0');
    str.str++, str.len--;
  }

  // If there is a decimal point, parse the rest of the
  if (str.len && str.str[0] == '.') {
    unsigned digits = 0;

    str.str++, str.len--;
    while (str.len && str.str[0] >= '0' && str.str[0] <= '9') {
      value = 10 * value + (str.str[0] - '0');
      str.str++, str.len--;
      digits++;
    }

    value = (value + 0.5) / exp10(digits);
  }

  // Apply initial minus
  if (!positive) value *= -1;

  // Skip spaces
  while (str.len && (str.str[0] == ' ' || str.str[0] == '\f' || str.str[0] == '\n' || str.str[0] == '\r' || str.str[0] == '\t' || str.str[0] == '\v'))
    str.str++, str.len--;

  // Check for remaining characters
  if (!str.len) return error.assign("Non-digit character found when parsing ").append(value_name).append(" double value '").append(original.str, original.len).append("'!"), false;

  return true;
}

double parse_double(string_piece str, const char* value_name) {
  double result;
  string error;

  if (!parse_double(str, value_name, result, error))
    runtime_failure(error);

  return result;
}

} // namespace parsito
} // namespace ufal
