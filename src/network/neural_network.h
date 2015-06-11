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
#include "activation_function.h"
#include "embedding/embedding.h"
#include "utils/binary_decoder.h"

namespace ufal {
namespace parsito {

class neural_network {
 public:
  typedef vector<vector<vector<float>>> embeddings_cache;

  void propagate(const vector<embedding>& embeddings, const vector<const vector<int>*>& embedding_ids_sequences,
                 vector<double>& hidden_layer, vector<double>& outcomes, const embeddings_cache* cache = nullptr) const;

  void load(binary_decoder& data);
  void generate_embeddings_cache(const vector<embedding>& embeddings, embeddings_cache& cache) const;

 private:
  friend class neural_network_trainer;

  void load_matrix(binary_decoder& data, vector<vector<float>>& m);

  activation_function::type hidden_layer_activation;
  vector<vector<float>> weights[2];
};

} // namespace parsito
} // namespace ufal
