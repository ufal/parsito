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
  if (!system) return;

  // Unlink all nodes in the given tree
  t.unlink_all_nodes();

  // Create configuration
  c.init(t);

  // Compute which transitions to perform and perform them
  while (!c.final()) {
    unsigned transition = 0; // TODO: compute which transition to perform
    system->perform(c, t, transition);
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
}

} // namespace parsito
} // namespace ufal
