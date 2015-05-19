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

void parser_nn::parse(tree& t, configuration& c) const {
  assert(system);

  // Create configuration
  c.init(&t);

  // Compute which transitions to perform and perform them
  while (!c.final()) {
    unsigned transition = 0; // TODO: compute which transition to perform
    system->perform(c, transition);
  }
}

void parser_nn::load(binary_decoder& data) {
  // Load labels
  labels.resize(data.next_2B());
  for (auto&& label : labels)
    data.next_str(label);

  // Load transition system
  string system_name;
  data.next_str(system_name);
  system.reset(transition_system::create(system_name, labels));
  if (!system) throw binary_decoder_error("Cannot load transition system");

  // Load transtiion oracle
  string oracle_name;
  data.next_str(oracle_name);
  oracle.reset(system->oracle(oracle_name));
  if (!oracle) throw binary_decoder_error("Cannot load transition oracle");

  // Load node extractor
  string node_extractor_description, node_extractor_error;
  data.next_str(node_extractor_description);
  if (!nodes.create(node_extractor_description, node_extractor_error))
    throw binary_decoder_error("Cannot load node extractor");
}

} // namespace parsito
} // namespace ufal
