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
#include "transition_system.h"

namespace ufal {
namespace parsito {

class transition_system_projective : public transition_system {
 public:
  transition_system_projective(const vector<string>& labels);

  virtual unsigned transition_count() const override;
  virtual void perform(configuration& c, tree& t, unsigned transition) const override;
  virtual transition_oracle* oracle(const string& name) const override;

 private:
  const vector<string>& labels;
  vector<unique_ptr<transition>> transitions;
};

} // namespace parsito
} // namespace ufal

