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

namespace ufal {
namespace parsito {

class binary_decoder;
class binary_encoder;

class compressor {
 public:
  static bool load(FILE* f, binary_decoder& data);
  static bool save(FILE* f, const binary_encoder& enc);
};

} // namespace parsito
} // namespace ufal
