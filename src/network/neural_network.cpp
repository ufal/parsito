// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>

#include "neural_network.h"

namespace ufal {
namespace parsito {

void neural_network::load_matrix(binary_decoder& data, vector<vector<float>>& m) {
  unsigned rows = data.next_4B();
  unsigned columns = data.next_4B();

  m.resize(rows);
  for (auto&& row : m) {
    const float* row_ptr = data.next<float>(columns);
    row.assign(row_ptr, row_ptr + columns);
  }
}

void neural_network::load(binary_decoder& data) {
  hidden_layer_activation = activation_function::type(data.next_1B());
  load_matrix(data, weights[0]);
  load_matrix(data, weights[1]);
}

void neural_network::propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences,
                               vector<double>& hidden_layer, vector<double>& outcomes) const {
  assert(!weights[0].empty());
  assert(!weights[1].empty());
  for (auto&& embedding_ids : embedding_ids_sequences) if (embedding_ids) assert(embeddings.size() == embedding_ids->size());

  unsigned hidden_layer_size = weights[0].front().size();
  unsigned outcomes_size = weights[1].front().size();

  outcomes.assign(outcomes_size, 0);

  // Hidden layer
  hidden_layer.assign(hidden_layer_size, 0);

  unsigned index = 0;
  for (auto&& embedding_ids : embedding_ids_sequences)
    for (unsigned i = 0; i < embeddings.size(); i++)
      if (embedding_ids && (*embedding_ids)[i] >= 0) {
        const float* embedding = embeddings[i].weight((*embedding_ids)[i]);
        for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, embedding++, index++)
          for (unsigned j = 0; j < hidden_layer_size; j++)
            hidden_layer[j] += *embedding * weights[0][index][j];
      } else {
        index += embeddings[i].dimension;
      }
  for (unsigned i = 0; i < hidden_layer_size; i++) // Bias
    hidden_layer[i] += weights[0][index][i];

  // Activation function
  switch (hidden_layer_activation) {
    case activation_function::TANH:
      for (auto&& weight : hidden_layer)
        weight = tanh(weight);
      break;
    case activation_function::CUBIC:
      for (auto&& weight : hidden_layer)
        weight = weight * weight * weight;
      break;
  }

  for (unsigned i = 0; i < hidden_layer_size; i++)
    for (unsigned j = 0; j < outcomes_size; j++)
      outcomes[j] += hidden_layer[i] * weights[1][i][j];
  for (unsigned i = 0; i < outcomes_size; i++) // Bias
    outcomes[i] += weights[1][hidden_layer_size][i];

  // Softmax
  double max = outcomes[0];
  for (unsigned i = 1; i < outcomes_size; i++) if (outcomes[i] > max) max = outcomes[i];

  double sum = 0;
  for (unsigned i = 0; i < outcomes_size; i++) sum += (outcomes[i] = exp(outcomes[i] - max));
  sum = 1 / sum;

  for (unsigned i = 0; i < outcomes_size; i++) outcomes[i] *= sum;
}

} // namespace parsito
} // namespace ufal
