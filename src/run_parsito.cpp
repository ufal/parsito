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
#include "utils/process_args.h"
#include "tree/tree_format.h"
#include "version/version.h"

using namespace ufal::parsito;

void parse(istream& in, ostream& out, tree_input_format& input_format, const tree_output_format& output_format) {
  string input, output, error;
  tree t;
  while (input_format.read_block(in, input)) {
    input_format.set_block(input);
    while (input_format.next_tree(t, error)) {
      output_format.append_tree(t, output);
      out << output;
      output.clear();
    }
    if (!error.empty())
      runtime_failure(error);
  }
}

int main(int argc, char* argv[]) {
  iostream_init();

  options::map options;
  if (!options::parse({{"input", options::value{"conllu"}},
                       {"output", options::value{"conllu"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 2 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " [options] model_file\n"
                    "Options: --input=conllu\n"
                    "         --output=conllu\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  string input = options.count("input") ? options["count"] : "conllu";
  unique_ptr<tree_input_format> input_format(tree_input_format::new_input_format(input));
  if (!input_format)
    runtime_failure("Unknown input format '" << input << "'!");

  string output = options.count("output") ? options["count"] : "conllu";
  unique_ptr<tree_output_format> output_format(tree_output_format::new_output_format(output));
  if (!output_format)
    runtime_failure("Unknown output format '" << output << "'!");

  process_args(2, argc, argv, parse, *input_format, *output_format);

  return 0;
}
