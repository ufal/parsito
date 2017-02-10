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
#include "network/network_parameters.h"
#include "parser/parser_nn_trainer.h"
#include "tree/tree_format.h"
#include "utils/compressor.h"
#include "utils/iostreams.h"
#include "utils/options.h"
#include "utils/parse_double.h"
#include "utils/parse_int.h"
#include "utils/split.h"
#include "version/version.h"

using namespace ufal::parsito;

void train_parser_nn(int argc, char* argv[]) {
  options::map options;
  if (!options::parse({{"adadelta", options::value::any},
                       {"adagrad", options::value::any},
                       {"adam", options::value::any},
                       {"batch_size", options::value::any},
                       {"dropout_hidden", options::value::any},
                       {"dropout_input", options::value::any},
                       {"early_stopping", options::value::any},
                       {"embeddings", options::value::any},
                       {"heldout", options::value::any},
                       {"hidden_layer", options::value::any},
                       {"hidden_layer_type", options::value{"cubic","tanh"}},
                       {"initialization_range", options::value::any},
                       {"input", options::value{"conllu"}},
                       {"iterations", options::value::any},
                       {"l1_regularization", options::value::any},
                       {"l2_regularization", options::value::any},
                       {"maxnorm_regularization", options::value::any},
                       {"nodes", options::value::any},
                       {"sgd", options::value::any},
                       {"sgd_momentum", options::value::any},
                       {"structured_interval", options::value::any},
                       {"threads", options::value::any},
                       {"transition_oracle", options::value{"static", "static_eager", "static_lazy", "dynamic"}},
                       {"transition_system", options::value{"projective","swap","link2"}},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help"))
    runtime_failure("Usage: " << argv[0] << " nn [options]\n"
                    "Options: --adadelta=momentum,epsilon\n"
                    "         --adagrad=learning rate,epsilon\n"
                    "         --adam=learning rate[,beta1,beta2,final learning rate]\n"
                    "         --batch_size=batch size\n"
                    "         --dropout_hidden=hidden layer dropout\n"
                    "         --dropout_input=input dropout\n"
                    "         --early_stopping=[0|1] use early stopping\n"
                    "         --embeddings=embedding description file\n"
                    "         --heldout=heldout data file\n"
                    "         --hidden_layer=hidden layer size\n"
                    "         --hidden_layer_type=cubic|tanh (hidden layer activation function)\n"
                    "         --initialization_range=initialization range\n"
                    "         --input=conllu (input format)\n"
                    "         --iterations=number of training iterations\n"
                    "         --l1_regularization=l1 regularization factor\n"
                    "         --l2_regularization=l2 regularization factor\n"
                    "         --maxnorm_regularization=max-norm regularization factor\n"
                    "         --nodes=node selector file\n"
                    "         --structured_interval=structured prediction interval\n"
                    "         --sgd=learning rate[,final learning rate]\n"
                    "         --sgd_momentum=momentum,learning rate[,final learning rate]\n"
                    "         --threads=number of training threads\n"
                    "         --transition_oracle=static|static_eager|static_lazy|dynamic\n"
                    "         --transition_system=projective|swap|link2\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version")) {
    cout << version::version_and_copyright() << endl;
    return;
  }

  // Use binary standard output
  iostreams_init_binary_output();

  // Process options
  network_parameters parameters;
  if (!options.count("iterations")) runtime_failure("Number of iterations must be specified!");
  parameters.iterations = parse_int(options["iterations"], "number of iterations");
  parameters.structured_interval = options.count("structured_interval") ? parse_int(options["structured_interval"], "structured prediction interval") : 0;
  parameters.hidden_layer = options.count("hidden_layer") ? parse_int(options["hidden_layer"], "hidden layer size") : 0;
  if (!parameters.hidden_layer) runtime_failure("The size of hidden layer must be specified!");
  if (!activation_function::create(options.count("hidden_layer_type") ? options["hidden_layer_type"] : "tanh", parameters.hidden_layer_type))
    runtime_failure("Unknown hidden layer type '" << options["hidden_layer_type"] << "'!");
  parameters.batch_size = options.count("batch_size") ? parse_int(options["batch_size"], "batch size") : 1;
  parameters.initialization_range = options.count("initialization_range") ? parse_double(options["initialization_range"], "initialiation range") : 0.1;
  parameters.l1_regularization = options.count("l1_regularization") ? parse_double(options["l1_regularization"], "l1 regularization") : 0;
  parameters.l2_regularization = options.count("l2_regularization") ? parse_double(options["l2_regularization"], "l2 regularization") : 0;
  parameters.maxnorm_regularization = options.count("maxnorm_regularization") ? parse_double(options["maxnorm_regularization"], "max-norm regularization") : 0;
  parameters.dropout_hidden = options.count("dropout_hidden") ? parse_double(options["dropout_hidden"], "hidden layer dropout") : 0;
  parameters.dropout_input = options.count("dropout_input") ? parse_double(options["dropout_input"], "input dropout") : 0;
  parameters.early_stopping = options.count("early_stopping") ? parse_int(options["early_stopping"], "early stopping") : options.count("heldout");

  int threads = options.count("threads") ? parse_int(options["threads"], "number of threads") : 1;
  if (threads <= 0) runtime_failure("The number of threads must be positive!");

  if (!options.count("transition_system")) runtime_failure("The transition system must be specified!");
  if (!options.count("transition_oracle")) runtime_failure("The transition oracle must be specified!");

  // Process network trainer options
  if (options.count("sgd") + options.count("sgd_momentum") + options.count("adagrad") + options.count("adedelta") > 1)
    runtime_failure("Cannot specify multiple trainer algorithms!");
  vector<string_piece> parts;
  if (options.count("sgd")) {
    parameters.trainer.algorithm = network_trainer::SGD;
    split(options["sgd"], ',', parts);
    if (parts.size() > 2) runtime_failure("More than two values given to the --sgd option!");
    parameters.trainer.learning_rate = parse_double(parts[0], "learning rate");
    parameters.trainer.learning_rate_final = parts.size() > 1 ? parse_double(parts[1], "final learning rate") : parameters.trainer.learning_rate;
  } else if (options.count("sgd_momentum")) {
    parameters.trainer.algorithm = network_trainer::SGD_MOMENTUM;
    split(options["sgd_momentum"], ',', parts);
    if (parts.size() < 2) runtime_failure("Expecting at least two values to the --sgd_momentum option!");
    if (parts.size() > 3) runtime_failure("More than three values given to the --sgd_momentum option!");
    parameters.trainer.momentum = parse_double(parts[0], "momentum");
    parameters.trainer.learning_rate = parse_double(parts[1], "learning rate");
    parameters.trainer.learning_rate_final = parts.size() > 2 ? parse_double(parts[2], "final learning rate") : parameters.trainer.learning_rate;
  } else if (options.count("adagrad")) {
    parameters.trainer.algorithm = network_trainer::ADAGRAD;
    split(options["adagrad"], ',', parts);
    if (parts.size() != 2) runtime_failure("Expecting two values to the --adagrad option!");
    parameters.trainer.learning_rate = parse_double(parts[0], "learning rate");
    parameters.trainer.learning_rate_final = parameters.trainer.learning_rate;
    parameters.trainer.epsilon = parse_double(parts[1], "adagrad epsilon");
  } else if (options.count("adadelta")) {
    parameters.trainer.algorithm = network_trainer::ADADELTA;
    split(options["adadelta"], ',', parts);
    if (parts.size() != 2) runtime_failure("Expecting two values to the --adadelta option!");
    parameters.trainer.momentum = parse_double(parts[0], "momentum");
    parameters.trainer.epsilon = parse_double(parts[1], "adadelta epsilon");
  } else if (options.count("adam")) {
    parameters.trainer.algorithm = network_trainer::ADAM;
    split(options["adam"], ',', parts);
    if (parts.size() < 1) runtime_failure("Expecting at least one value to the --adam option!");
    if (parts.size() > 4) runtime_failure("More than four values given to the --adam option!");
    parameters.trainer.learning_rate = parse_double(parts[0], "learning rate");
    parameters.trainer.momentum = parts.size() > 1 ? parse_double(parts[1], "beta1") : 0.9;
    parameters.trainer.momentum2 = parts.size() > 2 ? parse_double(parts[2], "beta2") : 0.999;
    parameters.trainer.learning_rate_final = parts.size() > 3 ? parse_double(parts[3], "final learning rate") : parameters.trainer.learning_rate;
    parameters.trainer.epsilon = 1e-8;
  } else
    runtime_failure("No trainer algorithm was specified!");

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

  string block;
  tree t;
  vector<tree> train;
  unsigned train_words = 0;
  cerr << "Loading training data: ";
  while (input_format->read_block(cin, block)) {
    input_format->set_text(block);
    while (input_format->next_tree(t)) {
      train.push_back(t);
      train_words += t.nodes.size() - 1;
    }
    if (!input_format->last_error().empty())
      runtime_failure(input_format->last_error());
  }
  cerr << train.size() << " sentences with " << train_words << " words." << endl;

  // Load heldout data if present
  vector<tree> heldout;
  if (options.count("heldout")) {
    ifstream in(options["heldout"]);
    if (!in.is_open()) runtime_failure("Cannot open heldout file '" << options["heldout"] << "'!");

    unsigned heldout_words = 0;
    cerr << "Loading heldout data: ";
    while (input_format->read_block(in, block)) {
      input_format->set_text(block);
      while (input_format->next_tree(t)) {
        heldout.push_back(t);
        heldout_words += t.nodes.size() - 1;
      }
      if (!input_format->last_error().empty())
        runtime_failure(input_format->last_error());
    }
    cerr << heldout.size() << " sentences with " << train_words << " words." << endl;
  }
  if (parameters.early_stopping && !heldout.size()) runtime_failure("Early stopping required, but no heldout data!");

  // Prepare the binary encoder
  binary_encoder enc;
  enc.add_str("nn_versioned");

  // Train the parser_nn
  cerr << "Training the parser" << endl;
  parser_nn_trainer::train(options["transition_system"], options["transition_oracle"],
                           embeddings, nodes, parameters, threads, train, heldout, enc);

  // Encode the parser
  cerr << "Encoding the parser: ";
  compressor::save(cout, enc);
  cerr << "done" << endl;

}

int main(int argc, char* argv[]) {
  iostreams_init();

  // Use poor-mans argument parsing at this point, not to touch the options
  // specific for every parser_model_identifier.
  if (argc < 2 || argv[1] == string("--help"))
      runtime_failure("Usage: " << argv[0] << " [generic_options] parser_model_identifier [parser_options]\n"
                      "Generic options: --version\n"
                      "                 --help\n"
                      "Parser model identifiers: nn\n"
                      "Parser options: use " << argv[0] << " nn --help");

  if (argc >= 2 && argv[1] == string("--version"))
    return cout << version::version_and_copyright() << endl, 0;

  if (argv[1] == string("nn"))
    train_parser_nn(argc, argv);
  else
    runtime_failure("Unknown parser_model_identifier '" << argv[1] << "'.!");

  return 0;
}
