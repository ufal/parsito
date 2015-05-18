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
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Assert that int is at least 4B
static_assert(sizeof(int) >= sizeof(int32_t), "Int must be at least 4B wide!");

namespace ufal {
namespace parsito {

#define runtime_failure(message) exit((cerr << message << endl, 1))

using namespace std;

} // namespace parsito
} // namespace ufal

#include "utils/string_piece.h"
