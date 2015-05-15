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
#include "transition.h"
#include "transition_oracle.h"

namespace ufal {
namespace parsito {

class transition_system {
 public:
  virtual ~transition_system() {}

  virtual unsigned transition_count() const = 0;
  virtual void perform(configuration& c, tree& t, unsigned transition) const = 0;
  virtual transition_oracle* oracle(const string& name) const = 0;

  static transition_system* create(const string& name, const vector<string>& labels);
};

} // namespace parsito
} // namespace ufal