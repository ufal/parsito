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
#include "parser.h"

namespace ufal {
namespace parsito {

class parser_nn : public parser {
 public:
  virtual void parse(tree& t) const override;

 protected:
  virtual void load(binary_decoder& data) override;

 private:
  vector<string> labels;
};

} // namespace parsito
} // namespace ufal