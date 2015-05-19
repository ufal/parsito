// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <fstream>

#include "common.h"
#include "parser/parser_nn_trainer.h"
#include "tree/tree_format.h"
#include "utils/compressor.h"
#include "utils/iostream_init.h"
#include "utils/options.h"
#include "utils/parse_int.h"
#include "version/version.h"

using namespace ufal::parsito;

int main(int argc, char* argv[]) {
  iostream_init();

  if (argc < 2) runtime_failure("Usage: " << argv[0] << " parser_model_identifier [options]");

  if (argv[1] != string("nn"))
    runtime_failure("Unknown parser_model_identifier '" << argv[1] << "'.!");

  options::map options;
  if (!options::parse({{"direct_connections", options::value{"0", "1"}},
                       {"embeddings", options::value::any},
                       {"heldout", options::value::any},
                       {"hidden_layer_size", options::value::any},
                       {"hidden_layer_type", options::value{"cubic","tanh"}},
                       {"input", options::value{"conllu"}},
                       {"nodes", options::value::any},
                       {"threads", options::value::any},
                       {"transition_oracle", options::value{"static"}},
                       {"transition_system", options::value{"projective"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 2 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " nn [options]\n"
                    "Options: --direct_connections=0|1 (should_nn_contain_direct_connections)\n"
                    "         --embeddings=embedding description file\n"
                    "         --heldout=heldout data file\n"
                    "         --hidden_layer_size=hidden layer size\n"
                    "         --hidden_layer_type=cubic|tanh (hidden layer activation function)\n"
                    "         --input=conllu (input format)\n"
                    "         --nodes=node selector file\n"
                    "         --transition_oracle=static\n"
                    "         --threads=number of training threads\n"
                    "         --transition_system=projective\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  // Use binary standard output
  iostream_init_binary_output();

  // Process options
  bool direct_connections = options.count("direct_connections") ? options["direct_connections"] == "1" : false;
  int hidden_layer_size = options.count("hidden_layer_size") ? parse_int(options["hidden_layer_size"], "hidden layer size") : 0;
  string hidden_layer_type = options.count("hidden_layer_type") ? options["hidden_layer_type"] : "cubic";
  if (hidden_layer_size < 0) runtime_failure("The hidden layer size cannot be negative!");
  if (!direct_connections && !hidden_layer_size) runtime_failure("The neural networks cannot have no direct connections nor hidden layer!");

  int threads = options.count("theads") ? parse_int(options["threads"], "number of threads") : 1;
  if (threads <= 0) runtime_failure("The number of threads must be positive!");

  if (!options.count("transition_system")) runtime_failure("The transition system must be specified!");
  if (!options.count("transition_oracle")) runtime_failure("The transition oracle must be specified!");

  // Load embeddings description
  if (!options.count("embeddings")) runtime_failure("The embedding description file is required!");
  string embeddings;
  {
    ifstream in(options["embeddings"]);
    if (!in.is_open()) runtime_failure("Cannot open embedding description file '" << options["embeddings"] << "'!");
    for (string line; getline(in, line);)
      embeddings.append(line).push_back('\n');
  }

  // Load node selector description
  if (!options.count("nodes")) runtime_failure("The node selector file is required!");
  string nodes;
  {
    ifstream in(options["nodes"]);
    if (!in.is_open()) runtime_failure("Cannot open node selector file '" << options["nodes"] << "'!");
    for (string line; getline(in, line);)
      nodes.append(line).push_back('\n');
  }

  // Load training data
  string input = options.count("input") ? options["count"] : "conllu";
  unique_ptr<tree_input_format> input_format(tree_input_format::new_input_format(input));
  if (!input_format)
    runtime_failure("Unknown input format '" << input << "'!");

  string block, error;
  tree t;
  vector<tree> train;
  while (input_format->read_block(cin, block)) {
    input_format->set_block(block);
    while (input_format->next_tree(t, error))
      train.push_back(t);
    if (!error.empty()) runtime_failure(error);
  }

  // Load heldout data if present
  vector<tree> heldout;
  if (options.count("heldout")) {
    ifstream in(options["heldout"]);
    if (!in.is_open()) runtime_failure("Cannot open heldout file '" << options["heldout"] << "'!");

    while (input_format->read_block(in, block)) {
      input_format->set_block(block);
      while (input_format->next_tree(t, error))
        heldout.push_back(t);
      if (!error.empty()) runtime_failure(error);
    }
  }

  // Prepare the binary encoder
  binary_encoder enc;
  enc.add_str("nn");

  // Train the parser_nn
  cerr << "Training the parser" << endl;
  parser_nn_trainer::train(direct_connections, hidden_layer_size, hidden_layer_type,
                           options["transition_system"], options["transition_oracle"], embeddings,
                           nodes, threads, train, heldout, enc);

  // Encode the parser
  cerr << "Encoding the parser: ";
  compressor::save(cout, enc);
  cerr << "done" << endl;

  return 0;
}
