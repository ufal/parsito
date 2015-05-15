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
#include "configuration.h"
#include "tree/tree.h"

namespace ufal {
namespace parsito {

class transition_oracle {
 public:
  virtual ~transition_oracle() {}

  virtual void outcomes(const configuration& c, const tree& t, const tree& golden, const vector<double>& predictions, vector<double>& outcomes) const = 0;
};

} // namespace parsito
} // namespace ufal