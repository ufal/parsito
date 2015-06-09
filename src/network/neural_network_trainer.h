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
    unsigned batch = 0;
    vector<double> outcomes;
    vector<double> hidden_layer;
    vector<double> error_outcomes;
    vector<double> error_hidden;

    // Delta accumulators
    vector<vector<float>> direct_batch;
    vector<vector<float>> hidden_batch[2];
    vector<vector<vector<float>>> error_embedding;
    vector<vector<unsigned>> error_embedding_nonempty;

    // Trainer data
    struct trainer_data {
      double delta = 0;
      double gradient = 0;
    };
    vector<vector<trainer_data>> direct_trainer;
    vector<vector<trainer_data>> hidden_trainer[2];
    vector<vector<vector<trainer_data>>> embedding_trainer;
  };
  void propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, workspace& w) const;
  void backpropagate(vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, unsigned required_outcome, workspace& w);
  void finalize_sentence();

  void save_network(binary_encoder& enc) const;

 private:
  struct trainer_sgd {
    static bool need_trainer_data;
    static inline double delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data);
  };
  struct trainer_sgd_momentum {
    static bool need_trainer_data;
    static inline double delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data);
  };
  struct trainer_adagrad {
    static bool need_trainer_data;
    static inline double delta(double gradient, const network_trainer& trainer, workspace::trainer_data& data);
  };

  template <class TRAINER> void backpropagate_template(vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences, unsigned required_outcome, workspace& w);
  void save_matrix(const vector<vector<float>>& m, binary_encoder& enc) const;

  neural_network& network;
  unsigned iteration, iterations;
  network_trainer trainer;
  double trainer_parameter;
  unsigned batch_size;
  double l1_regularization, l2_regularization;
};

} // namespace parsito
} // namespace ufal

