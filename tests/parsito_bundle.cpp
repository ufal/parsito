// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include "parsito.h"

using namespace ufal::parsito;
using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " model_file" << endl;
    return 1;
  }

  unique_ptr<tree_input_format> input_format(tree_input_format::new_conllu_input_format());
  unique_ptr<tree_output_format> output_format(tree_output_format::new_conllu_output_format());

  cerr << "Loading parser: ";
  unique_ptr<parser> p(parser::load(argv[1]));
  if (!p) {
    cerr << "Cannot load parser from file '" << argv[1] << "'!" << endl;
    return 1;
  }
  cerr << "done" << endl;

  clock_t now = clock();
  tree t;
  string input, output, error;
  while (input_format->read_block(cin, input)) {
    // Process all trees in the block
    input_format->set_block(input);
    while (input_format->next_tree(t, error)) {
      // Parse the tree
      p->parse(t);

      // Output the parsed tree
      output_format->write_tree(t, output, input_format.get());
      cout << output << flush;
    }
    if (!error.empty()) {
      cerr << "Cannot load input: " << error << endl;
      return 1;
    }
  }
  cerr << "Parsing done, in " << fixed << setprecision(3) << (clock() - now) / double(CLOCKS_PER_SEC) << " seconds." << endl;

  return 0;
}
