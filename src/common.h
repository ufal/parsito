// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// Assert that int is at least 4B
static_assert(sizeof(int) >= sizeof(int32_t), "Int must be at least 4B wide!");

namespace ufal {
namespace parsito {

using namespace std;

// Printf-like logging function.
inline int eprintf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = vfprintf(stderr, fmt, ap);
  va_end(ap);
  return res;
}

// Printf-like exit function.
inline void runtime_errorf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

} // namespace parsito
} // namespace ufal
