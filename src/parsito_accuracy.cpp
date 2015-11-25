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
#include "tree/tree_format.h"
#include "version/version.h"

using namespace ufal::parsito;

int main(int argc, char* argv[]) {
  iostreams_init();

  options::map options;
  if (!options::parse({{"input", options::value{"conllu"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 2 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " [options] model_file\n"
                    "Options: --input=conllu\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  string input_format_name = options.count("input") ? options["count"] : "conllu";
  unique_ptr<tree_input_format> input_format(tree_input_format::new_input_format(input_format_name));
  if (!input_format)
    runtime_failure("Unknown input format '" << input_format_name << "'!");

  cerr << "Loading parser: ";
  unique_ptr<parser> p(parser::load(argv[1]));
  if (!p)
    runtime_failure("Cannot load parser from file '" << argv[1] << "'!");
  cerr << "done" << endl;

  clock_t now = clock();

  enum { WITH_PUNCTUATION = 0, WITHOUT_PUNCTUATION = 1, TOTAL_PUNCTUATION = 2 };
  enum { UAS = 0, LAS = 1, TOTAL_AS = 2 };
  int total[TOTAL_PUNCTUATION] = {0, 0};
  int correct[TOTAL_PUNCTUATION][TOTAL_AS] = {{0, 0}, {0, 0}};

  tree t;
  string input, output, error;
  while (input_format->read_block(cin, input)) {
    input_format->set_block(input);
    while (input_format->next_tree(t, error)) {
      tree gold = t;

      // Parse the tree
      t.unlink_all_nodes();
      p->parse(t);

      // Evaluate parsed tree
      for (int i = 1; i < int(t.nodes.size()); i++) {
        for (int punctuation = 0; punctuation < TOTAL_PUNCTUATION; punctuation++)
          if (punctuation == WITH_PUNCTUATION ||
              (punctuation == WITHOUT_PUNCTUATION && t.nodes[i].upostag != "PUNCT")) {
            total[punctuation]++;
            correct[punctuation][UAS] += t.nodes[i].head == gold.nodes[i].head;
            correct[punctuation][LAS] += t.nodes[i].head == gold.nodes[i].head && t.nodes[i].deprel == gold.nodes[i].deprel;
          }
      }
    }
    if (!error.empty())
      runtime_failure(error);
  }

  cerr << "Parsing done, in " << fixed << setprecision(3) << (clock() - now) / double(CLOCKS_PER_SEC) << " seconds." << endl;
  for (int punctuation = 0; punctuation < TOTAL_PUNCTUATION; punctuation++)
    cout << (punctuation == WITH_PUNCTUATION ? "With   " : "Without") << " punctuation, " << fixed << setprecision(2)
         << "UAS: " << setw(5) << correct[punctuation][UAS] * 100. / total[punctuation] << "%, "
         << "LAS: " << setw(5) << correct[punctuation][LAS] * 100. / total[punctuation] << "%" << endl;

  return 0;
}
