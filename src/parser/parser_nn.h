// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "common.h"
#include "configuration/node_extractor.h"
#include "configuration/value_extractor.h"
#include "embedding/embedding.h"
#include "network/neural_network.h"
#include "parser.h"
#include "transition/transition_system.h"
#include "utils/threadsafe_stack.h"

namespace ufal {
namespace parsito {

class parser_nn : public parser {
 public:
  virtual void parse(tree& t) const override;

 protected:
  virtual void load(binary_decoder& data) override;

 private:
  friend class parser_nn_trainer;

  vector<string> labels;
  unique_ptr<transition_system> system;

  node_extractor nodes;

  vector<value_extractor> values;
  vector<embedding> embeddings;

  neural_network network;

  struct workspace {
    configuration conf;

    string word, word_buffer;
    vector<vector<int>> embeddings;

    vector<int> extracted_nodes;
    vector<const vector<int>*> extracted_embeddings;

    vector<double> outcomes, network_buffer;
  };
  mutable threadsafe_stack<workspace> workspaces;
};

} // namespace parsito
} // namespace ufal
