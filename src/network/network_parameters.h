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

namespace ufal {
namespace parsito {

struct network_parameters {
  unsigned iterations;
  bool direct_connections;
  unsigned hidden_layer;
  activation_function::type hidden_layer_type;
  double learning_rate;
  double learning_rate_final;
  double gaussian_sigma;
};

} // namespace parsito
} // namespace ufal