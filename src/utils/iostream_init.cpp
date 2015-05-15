// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "iostream_init.h"

namespace ufal {
namespace parsito {

void iostream_init() {
  iostream::sync_with_stdio(false);
}

void iostream_init_binary_input() {
#ifdef _WIN32
  _setmode(_fileno(stdin), _O_BINARY);
#endif
}

void iostream_init_binary_output() {
#ifdef _WIN32
  _setmode(_fileno(stdout), _O_BINARY);
#endif
}

} // namespace parsito
} // namespace ufal
