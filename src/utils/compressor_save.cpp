// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "binary_encoder.h"
#include "compressor.h"
#include "lzma/LzmaEnc.h"

namespace ufal {
namespace parsito {

static void *LzmaAlloc(void* /*p*/, size_t size) { return new char[size]; }
static void LzmaFree(void* /*p*/, void *address) { delete[] (char*) address; }
static lzma::ISzAlloc lzmaAllocator = { LzmaAlloc, LzmaFree };

bool compressor::save(ostream& out, const binary_encoder& enc) {
  size_t uncompressed_size = enc.data.size(), compressed_size = 2 * enc.data.size() + 100;
  vector<unsigned char> compressed(compressed_size);

  lzma::CLzmaEncProps props;
  lzma::LzmaEncProps_Init(&props);
  unsigned char props_encoded[LZMA_PROPS_SIZE];
  size_t props_encoded_size = LZMA_PROPS_SIZE;

  auto res = lzma::LzmaEncode(compressed.data(), &compressed_size, enc.data.data(), uncompressed_size, &props, props_encoded, &props_encoded_size, 0, nullptr, &lzmaAllocator, &lzmaAllocator);
  if (res != SZ_OK) return false;

  uint32_t poor_crc = uncompressed_size * 19991 + compressed_size * 199999991 + 1234567890;
  if (uint32_t(uncompressed_size) != uncompressed_size || uint32_t(compressed_size) != compressed_size) return false;
  if (!out.write((const char*) &uncompressed_size, sizeof(uint32_t))) return false;
  if (!out.write((const char*) &compressed_size, sizeof(uint32_t))) return false;
  if (!out.write((const char*) &poor_crc, sizeof(uint32_t))) return false;
  if (!out.write((const char*) &props_encoded, sizeof(props_encoded))) return false;
  if (!out.write((const char*) compressed.data(), compressed_size)) return false;

  return true;
}

} // namespace parsito
} // namespace ufal
