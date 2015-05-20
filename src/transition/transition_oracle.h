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
#include "configuration/configuration.h"
#include "tree/tree.h"

namespace ufal {
namespace parsito {

class transition_oracle {
 public:
  virtual ~transition_oracle() {}

  struct predicted_transition {
    unsigned best;
    unsigned to_follow;

    predicted_transition(unsigned best, unsigned to_follow) : best(best), to_follow(to_follow) {}
  };
  virtual predicted_transition predict(const configuration& conf, const tree& golden, unsigned network_outcome) const = 0;
};

} // namespace parsito
} // namespace ufal
