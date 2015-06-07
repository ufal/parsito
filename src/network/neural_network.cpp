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
  load_matrix(data, hidden[0]);
  load_matrix(data, hidden[1]);
}

void neural_network::propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences,
                               vector<double>& hidden_layer, vector<double>& outcomes) const {
  if (!hidden[0].empty()) {
    assert(!hidden[1].empty());
    assert(hidden[0].front().size() == hidden[1].size() - 1);
    if (!direct.empty()) {
      assert(direct.size() == hidden[0].size());
      assert(direct.front().size() == hidden[1].front().size());
    }
  }
  assert(!direct.empty() || !hidden[0].empty());
  for (auto&& embedding_ids : embedding_ids_sequences) if (embedding_ids) assert(embeddings.size() == embedding_ids->size());

  unsigned outcomes_size = !direct.empty() ? direct.front().size() : hidden[1].front().size();

  outcomes.assign(outcomes_size, 0);

  // Direct connections if present
  if (!direct.empty()) {
    unsigned direct_index = 0;
    for (auto&& embedding_ids : embedding_ids_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if (embedding_ids && (*embedding_ids)[i] >= 0) {
          const float* embedding = embeddings[i].weight((*embedding_ids)[i]);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, embedding++, direct_index++)
            for (unsigned j = 0; j < outcomes_size; j++)
              outcomes[j] += *embedding * direct[direct_index][j];
        } else {
          direct_index += embeddings[i].dimension;
        }
    for (unsigned i = 0; i < outcomes_size; i++) // Bias
      outcomes[i] += direct[direct_index][i];
  }

  // Hidden layer if present
  if (!hidden[0].empty()) {
    unsigned hidden_layer_size = hidden[0].front().size();
    hidden_layer.assign(hidden_layer_size, 0);

    unsigned hidden_index = 0;
    for (auto&& embedding_ids : embedding_ids_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if (embedding_ids && (*embedding_ids)[i] >= 0) {
          const float* embedding = embeddings[i].weight((*embedding_ids)[i]);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, embedding++, hidden_index++)
            for (unsigned j = 0; j < hidden_layer_size; j++)
              hidden_layer[j] += *embedding * hidden[0][hidden_index][j];
        } else {
          hidden_index += embeddings[i].dimension;
        }
    for (unsigned i = 0; i < hidden_layer_size; i++) // Bias
      hidden_layer[i] += hidden[0][hidden_index][i];

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
        outcomes[j] += hidden_layer[i] * hidden[1][i][j];
    for (unsigned i = 0; i < outcomes_size; i++) // Bias
      outcomes[i] += hidden[1][hidden_layer_size][i];
  }

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
