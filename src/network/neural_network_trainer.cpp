// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <random>

#include "neural_network_trainer.h"

namespace ufal {
namespace parsito {

template <class URNG>
neural_network_trainer::neural_network_trainer(neural_network& network, unsigned input_size, unsigned output_size,
                                               const network_parameters& parameters, URNG& generator) : network(network) {
  normal_distribution<float> normal;

  // Initialize direct connections
  if (parameters.direct_connections) {
    network.direct.resize(input_size);
    for (auto&& row : network.direct) {
      row.resize(output_size);
      for (auto&& weight : row)
        weight = normal(generator);
    }
  }

  // Initialize hidden layer
  network.hidden_layer_activation = parameters.hidden_layer_type;
  if (parameters.hidden_layer) {
    network.hidden[0].resize(input_size);
    for (auto&& row : network.hidden[0]) {
      row.resize(parameters.hidden_layer);
      for (auto&& weight : row)
        weight = normal(generator);
    }

    network.hidden[1].resize(parameters.hidden_layer);
    for (auto&& row : network.hidden[1]) {
      row.resize(output_size);
      for (auto&& weight : row)
        weight = normal(generator);
    }
  }

  // Store the network_parameters
  iteration = 0;
  iterations = parameters.iterations;
  learning_rate_initial = parameters.learning_rate;
  learning_rate_final = parameters.learning_rate_final;
  gaussian_sigma = parameters.gaussian_sigma;
}

bool neural_network_trainer::next_iteration() {
  if (iteration++ >= iterations) return false;

  learning_rate = learning_rate_final && iterations > 1 ?
      exp(((iterations - iteration) * log(learning_rate_initial) + (iteration-1) * log(learning_rate_final)) / (iterations-1)) :
      learning_rate_initial;

  return true;
}

void neural_network_trainer::propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& words_sequences, workspace& w) const {
  network.propagate(embeddings, words_sequences, w.hidden_layer, w.outcomes);
}

void neural_network_trainer::backpropagate(const vector<embedding>& /*embeddings*/, const vector<const vector<int>*>& /*words_sequences*/, unsigned /*required_outcome*/, workspace& /*w*/) {
  // TODO
}

void neural_network_trainer::save_matrix(const vector<vector<float>>& m, binary_encoder& enc) const {
  enc.add_4B(m.size());
  enc.add_4B(m.empty() ? 0 : m.front().size());

  for (auto&& row : m) {
    assert(row.size() == m.front().size());
    enc.add<float>(row.data(), row.size());
  }
}

void neural_network_trainer::save_network(binary_encoder& enc) const {
  save_matrix(network.direct, enc);
  enc.add_1B(network.hidden_layer_activation);
  save_matrix(network.hidden[0], enc);
  save_matrix(network.hidden[1], enc);
}

} // namespace parsito
} // namespace ufal


