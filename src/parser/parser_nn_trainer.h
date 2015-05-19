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
#include "utils/binary_encoder.h"
#include "tree/tree.h"

namespace ufal {
namespace parsito {

class parser_nn_trainer {
 public:
  static void train(bool direct_connections, unsigned hidden_layer_size, const string& hidden_layer_type,
                    const string& transition_system_name, const string& transition_oracle_name,
                    const string& embeddings_description, const string& nodes_description, unsigned threads,
                    const vector<tree>& train, const vector<tree>& heldout, binary_encoder& enc);
};

} // namespace parsito
} // namespace ufal
