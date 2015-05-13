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
#include "utils/process_args.h"
#include "tree/tree_format.h"
#include "version/version.h"

using namespace ufal::parsito;

void parse(FILE* in, FILE* out, tree_input_format& input_format, const tree_output_format& output_format) {
  string input, output;
  tree t;
  while (input_format.read_block(in, input)) {
    input_format.set_block(input);
    while (input_format.next_tree(t)) {
      output_format.append_tree(t, output);
      fputs(output.c_str(), out);
      output.clear();
    }
  }
}

int main(int argc, char* argv[]) {
  options::map options;
  if (!options::parse({{"input", options::value{"conllu"}},
                       {"output", options::value{"conllu"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help"))
    runtime_errorf("Usage: %s [options]\n"
                    "Options: --input=conllu\n"
                    "         --output=conllu\n"
                    "         --version\n"
                    "         --help", argv[0]);
  if (options.count("version"))
    return puts(version::version_and_copyright().c_str()), 0;

  string input = options.count("input") ? options["count"] : "conllu";
  unique_ptr<tree_input_format> input_format(tree_input_format::new_input_format(input));
  if (!input_format)
    runtime_errorf("Unknown input format '%s'!", input.c_str());

  string output = options.count("output") ? options["count"] : "conllu";
  unique_ptr<tree_output_format> output_format(tree_output_format::new_output_format(output));
  if (!output_format)
    runtime_errorf("Unknown output format '%s'!", output.c_str());

  process_args(1, argc, argv, parse, *input_format, *output_format);

  return 0;
}
