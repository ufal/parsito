// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <random>

#include "common.h"
#include "network_parameters.h"
#include "neural_network.h"
#include "utils/binary_encoder.h"

namespace ufal {
namespace parsito {

class neural_network_trainer {
 public:
  neural_network_trainer(neural_network& network, unsigned input_size, unsigned output_size,
                         const network_parameters& parameters, mt19937& generator);

  bool next_iteration();

  struct workspace {
    vector<double> outcomes;
    vector<double> hidden_layer;
    vector<double> error_outcomes;
    vector<double> error_hidden;
    vector<double> error_input;
  };
  void propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, workspace& w) const;
  void backpropagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, unsigned required_outcome, workspace& w);
  void finalize_sentence();

  void save_network(binary_encoder& enc) const;

 private:
  void save_matrix(const vector<vector<float>>& m, binary_encoder& enc) const;

  neural_network& network;
  unsigned iteration, iterations;
  double learning_rate, learning_rate_initial, learning_rate_final;
  double l1_regularization, l2_regularization;
};

} // namespace parsito
} // namespace ufal

