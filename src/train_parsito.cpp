// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "common.h"
#include "utils/iostream_init.h"
#include "utils/options.h"
#include "version/version.h"

using namespace ufal::parsito;

int main(int argc, char* argv[]) {
  iostream_init();

  options::map options;
  if (!options::parse({{"input", options::value{"conllu"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 1 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " [options]\n"
                    "Options: --input=conllu\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  // Use binary standard output
  iostream_init_binary_output();

  return 0;
}
