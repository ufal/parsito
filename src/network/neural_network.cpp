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
  load_matrix(data, direct);
  hidden_layer_activation = activation_function::type(data.next_1B());
  load_matrix(data, hidden_layer[0]);
  load_matrix(data, hidden_layer[1]);
}

void neural_network::propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& words_sequences,
                               vector<double>& buffer, vector<double>& outcomes) const {
  if (!hidden_layer[0].empty()) {
    assert(!hidden_layer[1].empty());
    assert(hidden_layer[0].front().size() == hidden_layer[1].size());
    if (!direct.empty()) {
      assert(direct.size() == hidden_layer[0].size());
      assert(direct.front().size() == hidden_layer[1].front().size());
    }
  }
  assert(!direct.empty() || !hidden_layer[0].empty());
  for (auto&& words : words_sequences) assert(embeddings.size() == words->size());

  unsigned outcomes_size = !direct.empty() ? direct.front().size() : hidden_layer[1].front().size();

  outcomes.assign(outcomes_size, 0);

  // Direct connections if present
  if (!direct.empty()) {
    unsigned direct_index = 0;
    for (auto&& words : words_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if ((*words)[i] >= 0) {
          const float* embedding = embeddings[i].weight((*words)[i]);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, embedding++, direct_index++)
            for (unsigned k = 0; k < outcomes_size; k++)
              outcomes[k] += *embedding * direct[direct_index][k];
        } else {
          direct_index += embeddings[i].dimension;
        }
  }

  // Hidden layer if present
  if (!hidden_layer[0].empty()) {
    unsigned hidden_layer_size = hidden_layer[0].front().size();
    buffer.assign(hidden_layer_size, 0);

    unsigned hidden_layer_index = 0;
    for (auto&& words : words_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if ((*words)[i] >= 0) {
          const float* embedding = embeddings[i].weight((*words)[i]);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, embedding++, hidden_layer_index++)
            for (unsigned k = 0; k < hidden_layer_size; k++)
              buffer[k] += *embedding * hidden_layer[0][hidden_layer_index][k];
        } else {
          hidden_layer_index += embeddings[i].dimension;
        }

    // Activation function
    switch (hidden_layer_activation) {
      case activation_function::TANH:
        for (auto&& weight : buffer)
          weight = tanh(weight);
        break;
      case activation_function::CUBIC:
        for (auto&& weight : buffer)
          weight = weight * weight * weight;
        break;
    }

    for (unsigned i = 0; i < hidden_layer_size; i++)
      for (unsigned j = 0; j < outcomes_size; j++)
        outcomes[j] += buffer[i] * hidden_layer[1][i][j];
  }

  // Softmax
  double sum = 0;
  for (unsigned i = 0; i < outcomes_size; i++) sum += (outcomes[i] = exp(outcomes[i]));
  sum = 1 / sum;
  for (unsigned i = 0; i < outcomes_size; i++) outcomes[i] *= sum;
}

} // namespace parsito
} // namespace ufal
