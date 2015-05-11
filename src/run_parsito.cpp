// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "common.h"
#include "utils/input.h"
#include "utils/options.h"
#include "version/version.h"

using namespace ufal::parsito;

int main(int argc, char* argv[]) {
  options::map options;
  if (!options::parse({{"version", options::value::none},
                      {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 2 && !options.count("version")))
    runtime_errorf("Usage: %s [options]\n"
                    "Options: --version\n"
                    "         --help", argv[0]);
  if (options.count("version"))
    return puts(version::version_and_copyright().c_str()), 0;

  return 0;
}
