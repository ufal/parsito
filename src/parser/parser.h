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
#include "utils/binary_decoder.h"

namespace ufal {
namespace parsito {

class parser {
 public:
  virtual ~parser() {};

  virtual void parse(tree& t, configuration& c) const = 0;

  static parser* load(const char* file);
  static parser* load(istream& in);

 protected:
  virtual void load(binary_decoder& data) = 0;
  static parser* create(const string& name);
};

} // namespace parsito
} // namespace ufal
