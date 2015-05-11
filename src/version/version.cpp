// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "version.h"

namespace ufal {
namespace parsito {

// Returns current version.
version version::current() {
  return {0, 9, 0, "devel"};
}

// Returns multi-line formated version and copyright string.
string version::version_and_copyright() {
  string copyright;

  auto parsito = version::current();

  return "Parsito version " + to_string(parsito.major) + '.' + to_string(parsito.minor) + '.' + to_string(parsito.patch) +
      (parsito.prerelease.empty() ? "" : "-") + parsito.prerelease + "\n"
      "Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty of\n"
      "Mathematics and Physics, Charles University in Prague, Czech Republic.";
}

} // namespace parsito
} // namespace ufal
