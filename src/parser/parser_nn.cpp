// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "parser_nn.h"

namespace ufal {
namespace parsito {

void parser_nn::parse(tree& /*t*/) const {
}

void parser_nn::load(binary_decoder& data) {
  labels.resize(data.next_2B());
  for (auto&& label : labels) {
    unsigned label_len = data.next_1B();
    label.assign(data.next<char>(label_len), label_len);
  }
}

} // namespace parsito
} // namespace ufal