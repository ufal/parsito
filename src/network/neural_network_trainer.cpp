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

neural_network_trainer::neural_network_trainer(neural_network& network, unsigned input_size, unsigned output_size,
                                               const network_parameters& parameters, mt19937& generator) : network(network) {
  uniform_real_distribution<float> uniform(-parameters.initialization_range, parameters.initialization_range);

  // Initialize direct connections
  if (parameters.direct_connections) {
    network.direct.resize(input_size + 1/*bias*/);
    for (auto&& row : network.direct) {
      row.resize(output_size);
      for (auto&& weight : row)
        weight = uniform(generator);
    }
  }

  // Initialize hidden layer
  network.hidden_layer_activation = parameters.hidden_layer_type;
  if (parameters.hidden_layer) {
    network.hidden[0].resize(input_size + 1/*bias*/);
    for (auto&& row : network.hidden[0]) {
      row.resize(parameters.hidden_layer);
      for (auto&& weight : row)
        weight = uniform(generator);
    }

    network.hidden[1].resize(parameters.hidden_layer + 1/*bias*/);
    for (auto&& row : network.hidden[1]) {
      row.resize(output_size);
      for (auto&& weight : row)
        weight = uniform(generator);
    }
  }

  // Store the network_parameters
  iteration = 0;
  iterations = parameters.iterations;
  trainer = parameters.trainer;
  batch_size = parameters.batch_size;
  l1_regularization = parameters.l1_regularization;
  l2_regularization = parameters.l2_regularization;
}

bool neural_network_trainer::next_iteration() {
  if (iteration++ >= iterations) return false;

  if (trainer.algorithm == network_trainer::SGD)
    if (trainer.learning_rate != trainer.learning_rate_final && iteration > 1)
      trainer.learning_rate =
          exp(((iterations - iteration) * log(trainer.learning_rate) + log(trainer.learning_rate_final)) / (iterations - iteration + 1));

  return true;
}

void neural_network_trainer::propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, workspace& w) const {
  network.propagate(embeddings, embedding_ids_sequences, w.hidden_layer, w.outcomes);
}

// SGD
bool neural_network_trainer::trainer_sgd::need_trainer_data = false;
double neural_network_trainer::trainer_sgd::delta(double gradient, const network_trainer& trainer, workspace::trainer_data& /*data*/) {
  return trainer.learning_rate * gradient;
}

// SGD with momentum
bool neural_network_trainer::trainer_sgd_momentum::need_trainer_data = true;
double neural_network_trainer::trainer_sgd_momentum::delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data) {
  data.delta = trainer.momentum * data.delta + trainer.learning_rate * gradient;
  return data.delta;
}

// AdaGrad
bool neural_network_trainer::trainer_adagrad::need_trainer_data = true;
double neural_network_trainer::trainer_adagrad::delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data) {
  data.gradient += gradient * gradient;
  return trainer.learning_rate / sqrt(data.gradient) * gradient;
}

// AdaDelta
bool neural_network_trainer::trainer_adadelta::need_trainer_data = true;
double neural_network_trainer::trainer_adadelta::delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data) {
  data.gradient = trainer.momentum * data.gradient + (1 - trainer.momentum) * gradient * gradient;
  double delta = sqrt(data.delta + trainer.epsilon) / sqrt(data.gradient + trainer.epsilon) * gradient;
  data.delta = trainer.momentum * data.delta + (1 - trainer.momentum) * delta * delta;
  return delta;
}

// Backpropagation
template <class TRAINER>
void neural_network_trainer::backpropagate_template(vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, unsigned required_outcome, workspace& w) {
  size_t outcomes_size = w.outcomes.size();

  // Allocate space for delta accumulators
  if (network.direct.size() > w.direct_batch.size()) w.direct_batch.resize(network.direct.size());
  if (network.hidden[0].size() > w.hidden_batch[0].size()) w.hidden_batch[0].resize(network.hidden[0].size());
  if (network.hidden[1].size() > w.hidden_batch[1].size()) w.hidden_batch[1].resize(network.hidden[1].size());
  if (embeddings.size() > w.error_embedding.size()) w.error_embedding.resize(embeddings.size());
  if (embeddings.size() > w.error_embedding_nonempty.size()) w.error_embedding_nonempty.resize(embeddings.size());

  // Allocate space for trainer_data if required)
  if (TRAINER::need_trainer_data) {
    while (network.direct.size() > w.direct_trainer.size()) w.direct_trainer.emplace_back(outcomes_size);
    while (network.hidden[0].size() > w.hidden_trainer[0].size()) w.hidden_trainer[0].emplace_back(network.hidden[0].front().size());
    while (network.hidden[1].size() > w.hidden_trainer[1].size()) w.hidden_trainer[1].emplace_back(outcomes_size);
  }

  // Compute error vector
  w.error_outcomes.resize(outcomes_size);
  for (unsigned i = 0; i < outcomes_size; i++)
    w.error_outcomes[i] = (i == required_outcome) - w.outcomes[i];

  // Update direct connections and backpropagate to error_embedding
  if (!network.direct.empty()) {
    unsigned direct_index = 0;
    for (auto&& embedding_ids : embedding_ids_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if (embedding_ids && (*embedding_ids)[i] >= 0) {
          int embedding_id = (*embedding_ids)[i];

          float* error_embedding = nullptr; // Accumulate embedding error if required
          if (embeddings[i].can_update_weights(embedding_id)) {
            if (w.error_embedding[i].size() <= unsigned(embedding_id)) w.error_embedding[i].resize(embedding_id + 1);
            if (w.error_embedding[i][embedding_id].empty()) {
              w.error_embedding[i][embedding_id].assign(embeddings[i].dimension, 0);
              w.error_embedding_nonempty[i].emplace_back(embedding_id);
            }
            error_embedding = w.error_embedding[i][embedding_id].data();
          }

          const float* embedding = embeddings[i].weight(embedding_id);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, direct_index++, embedding++, error_embedding += !!error_embedding) {
            if (error_embedding)
              for (unsigned j = 0; j < outcomes_size; j++)
                *error_embedding += network.direct[direct_index][j] * w.error_outcomes[j];
            if (w.direct_batch[direct_index].empty()) w.direct_batch[direct_index].resize(outcomes_size);
            for (unsigned j = 0; j < outcomes_size; j++)
              w.direct_batch[direct_index][j] += *embedding * w.error_outcomes[j];
          }
        } else {
          direct_index += embeddings[i].dimension;
        }
    // Bias
    if (w.direct_batch[direct_index].empty()) w.direct_batch[direct_index].resize(outcomes_size);
    for (unsigned i = 0; i < outcomes_size; i++)
      w.direct_batch[direct_index][i] += w.error_outcomes[i];
  }

  // Update hidden layer connections and backpropagate to error_embedding
  if (!network.hidden[0].empty()) {
    unsigned hidden_layer_size = network.hidden[0].front().size();

    // Backpropagate error_outcomes to error_hidden
    w.error_hidden.assign(hidden_layer_size, 0);
    for (unsigned i = 0; i < hidden_layer_size; i++)
      for (unsigned j = 0; j < outcomes_size; j++)
        w.error_hidden[i] += network.hidden[1][i][j] * w.error_outcomes[j];

    // Perform activation function derivation
    switch (network.hidden_layer_activation) {
      case activation_function::TANH:
        for (unsigned i = 0; i < hidden_layer_size; i++)
          w.error_hidden[i] *= 1 - w.hidden_layer[i] * w.hidden_layer[i];
        break;
      case activation_function::CUBIC:
        for (unsigned i = 0; i < hidden_layer_size; i++) {
          double hidden_layer = cbrt(w.hidden_layer[i]);
          w.error_hidden[i] *= 3 * hidden_layer * hidden_layer;
        }
        break;
    }

    // Update hidden[1]
    for (unsigned i = 0; i < hidden_layer_size; i++) {
      if (w.hidden_batch[1][i].empty()) w.hidden_batch[1][i].resize(outcomes_size);
      for (unsigned j = 0; j < outcomes_size; j++)
        w.hidden_batch[1][i][j] += w.hidden_layer[i] * w.error_outcomes[j];
    }
    // Bias
    if (w.hidden_batch[1][hidden_layer_size].empty()) w.hidden_batch[1][hidden_layer_size].resize(outcomes_size);
    for (unsigned i = 0; i < outcomes_size; i++)
      w.hidden_batch[1][hidden_layer_size][i] += w.error_outcomes[i];

    // Update hidden[0] and backpropagate to error_embedding
    unsigned hidden_index = 0;
    for (auto&& embedding_ids : embedding_ids_sequences)
      for (unsigned i = 0; i < embeddings.size(); i++)
        if (embedding_ids && (*embedding_ids)[i] >= 0) {
          int embedding_id = (*embedding_ids)[i];

          float* error_embedding = nullptr; // Accumulate embedding error if required
          if (embeddings[i].can_update_weights(embedding_id)) {
            if (w.error_embedding[i].size() <= unsigned(embedding_id)) w.error_embedding[i].resize(embedding_id + 1);
            if (w.error_embedding[i][embedding_id].empty()) {
              w.error_embedding[i][embedding_id].assign(embeddings[i].dimension, 0);
              w.error_embedding_nonempty[i].emplace_back(embedding_id);
            }
            error_embedding = w.error_embedding[i][embedding_id].data();
          }

          const float* embedding = embeddings[i].weight(embedding_id);
          for (unsigned dimension = embeddings[i].dimension; dimension; dimension--, hidden_index++, embedding++, error_embedding += !!error_embedding) {
            if (error_embedding)
              for (unsigned j = 0; j < hidden_layer_size; j++)
                *error_embedding += network.hidden[0][hidden_index][j] * w.error_hidden[j];
            if (w.hidden_batch[0][hidden_index].empty()) w.hidden_batch[0][hidden_index].resize(hidden_layer_size);
            for (unsigned j = 0; j < hidden_layer_size; j++)
              w.hidden_batch[0][hidden_index][j] += *embedding * w.error_hidden[j];
          }
        } else {
          hidden_index += embeddings[i].dimension;
        }
    // Bias
    if (w.hidden_batch[0][hidden_index].empty()) w.hidden_batch[0][hidden_index].resize(hidden_layer_size);
    for (unsigned i = 0; i < hidden_layer_size; i++)
      w.hidden_batch[0][hidden_index][i] += w.error_hidden[i];
  }

  if (++w.batch < batch_size) return;
  w.batch = 0;

  // Update direct weights
  if (!network.direct.empty()) {
    for (unsigned i = 0; i < w.direct_batch.size(); i++)
      if (!w.direct_batch[i].empty()) {
        for (unsigned j = 0; j < outcomes_size; j++)
          network.direct[i][j] += TRAINER::delta(w.direct_batch[i][j], trainer, w.direct_trainer[i][j]) - l2_regularization * network.direct[i][j];
        w.direct_batch[i].clear();
      }
  }

  // Update hidden weights
  if (!network.hidden[0].empty())
    for (int i = 0; i < 2; i++) {
      for (unsigned j = 0; j < w.hidden_batch[i].size(); j++)
        if (!w.hidden_batch[i][j].empty()) {
          for (unsigned k = 0, size = w.hidden_batch[i][j].size(); k < size; k++)
            network.hidden[i][j][k] += TRAINER::delta(w.hidden_batch[i][j][k], trainer, w.hidden_trainer[i][j][k]) - l2_regularization * network.hidden[i][j][k];
          w.hidden_batch[i][j].clear();
        }
    }

  // Update embedding weights using error_embedding
  for (unsigned i = 0; i < embeddings.size(); i++) {
    for (auto&& id : w.error_embedding_nonempty[i]) {
      if (TRAINER::need_trainer_data) {
        if (w.embedding_trainer.size() <= i) w.embedding_trainer.resize(i + 1);
        if (w.embedding_trainer[i].size() <= id) w.embedding_trainer[i].resize(id + 1);
        if (w.embedding_trainer[i][id].size() < embeddings[i].dimension) w.embedding_trainer[i][id].resize(embeddings[i].dimension);
      }
      float* embedding = embeddings[i].weight(id);
      for (unsigned j = 0; j < embeddings[i].dimension; j++)
        embedding[j] += TRAINER::delta(w.error_embedding[i][id][j], trainer, w.embedding_trainer[i][id][j]) - l2_regularization * embedding[j];
      w.error_embedding[i][id].clear();
    }
    w.error_embedding_nonempty[i].clear();
  }
}

void neural_network_trainer::backpropagate(vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, unsigned required_outcome, workspace& w) {
  switch (trainer.algorithm) {
    case network_trainer::SGD:
      backpropagate_template<trainer_sgd>(embeddings, embedding_ids_sequences, required_outcome, w);
      return;
    case network_trainer::SGD_MOMENTUM:
      backpropagate_template<trainer_sgd_momentum>(embeddings, embedding_ids_sequences, required_outcome, w);
      return;
    case network_trainer::ADAGRAD:
      backpropagate_template<trainer_adagrad>(embeddings, embedding_ids_sequences, required_outcome, w);
      return;
    case network_trainer::ADADELTA:
      backpropagate_template<trainer_adadelta>(embeddings, embedding_ids_sequences, required_outcome, w);
      return;
  }

  runtime_failure("Internal error, unsupported trainer!");
}

void neural_network_trainer::finalize_sentence() {
  // Perform L1 regularization
  if (l1_regularization) {
    // Direct connections
    if (!network.direct.empty())
      for (auto&& row : network.direct)
        for (auto&& weight : row)
          if (weight < l1_regularization) weight += l1_regularization;
          else if (weight > l1_regularization) weight -= l1_regularization;
          else weight = 0;

    // Hidden layer connections
    if (!network.hidden[0].empty())
      for (auto&& hidden : network.hidden)
        for (auto&& row : hidden)
          for (auto&& weight : row)
            if (weight < l1_regularization) weight += l1_regularization;
            else if (weight > l1_regularization) weight -= l1_regularization;
            else weight = 0;
  }
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
