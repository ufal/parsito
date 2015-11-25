// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "common.h"
#include "parser/parser.h"
#include "utils/iostreams.h"
#include "utils/options.h"
#include "utils/process_args.h"
#include "tree/tree_format.h"
#include "version/version.h"

using namespace ufal::parsito;

void parse(istream& in, ostream& out, const parser& p, tree_input_format& input_format, const tree_output_format& output_format) {
  string input, output, error;
  tree t;

  // Read blocks containing input trees
  while (input_format.read_block(in, input)) {
    // Process all trees in the block
    input_format.set_block(input);
    while (input_format.next_tree(t, error)) {
      // Parse the tree
      p.parse(t);

      // Output the parsed tree
      output_format.append_tree(t, output, &input_format);
      out << output;
      output.clear();
    }
    if (!error.empty())
      runtime_failure(error);
  }
}

int main(int argc, char* argv[]) {
  iostreams_init();

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

  cerr << "Loading parser: ";
  unique_ptr<parser> p(parser::load(argv[1]));
  if (!p)
    runtime_failure("Cannot load parser from file '" << argv[1] << "'!");
  cerr << "done" << endl;

  clock_t now = clock();
  process_args(2, argc, argv, parse, *p, *input_format, *output_format);
  cerr << "Parsing done, in " << fixed << setprecision(3) << (clock() - now) / double(CLOCKS_PER_SEC) << " seconds." << endl;

  return 0;
}
