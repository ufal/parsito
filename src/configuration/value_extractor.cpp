// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "value_extractor.h"
#include "utils/split.h"

namespace ufal {
namespace parsito {

const string value_extractor::literal_unknown = "<unk>";

void value_extractor::extract(const node* n, vector<string>& values) const {
  if (!n) {
    values.assign(selectors.size(), literal_unknown);
    return;
  }

  values.resize(selectors.size());
  for (size_t i = 0; i < selectors.size(); i++)
    switch (selectors[i]) {
      case FORM:
        values[i].assign(n->form);
        break;
      case LEMMA:
        values[i].assign(n->lemma);
        break;
      case LEMMA_ID:
        if (!n->misc.empty()) {
          // Try finding LId= in misc column
          auto lid = n->misc.find("LId=");
          if (lid != string::npos) {
            lid += 4;

            // Find optional | ending the lemma_id
            auto lid_end = n->misc.find('|', lid);
            if (lid_end == string::npos) lid_end = n->misc.size();

            // Store the lemma_id
            values[i].assign(n->misc, lid, lid_end - lid);
            break;
          }
        }
        values[i].assign(n->lemma);
        break;
      case TAG:
        values[i].assign(n->tag);
        break;
      case UNIVERSAL_TAG:
        values[i].assign(n->ctag);
        break;
      case DEPREL:
        values[i].assign(n->deprel);
        break;
    }
}

bool value_extractor::create(string_piece description, string& error) {
  string literal_form = "form";
  string literal_lemma = "lemma";
  string literal_lemma_id = "lemma_id";
  string literal_tag = "tag";
  string literal_universal_tag = "universal_tag";
  string literal_deprel = "deprel";

  selectors.clear();
  error.clear();

  vector<string_piece> lines;
  split(description, '\n', lines);
  for (auto&& line : lines) {
    if (!line.len || line.str[0] == '#') continue;

    if (literal_form.compare(0, literal_form.size(), line.str, line.len))
      selectors.emplace_back(FORM);
    else if (literal_lemma.compare(0, literal_lemma.size(), line.str, line.len))
      selectors.emplace_back(LEMMA);
    else if (literal_lemma_id.compare(0, literal_lemma_id.size(), line.str, line.len))
      selectors.emplace_back(LEMMA_ID);
    else if (literal_tag.compare(0, literal_tag.size(), line.str, line.len))
      selectors.emplace_back(TAG);
    else if (literal_universal_tag.compare(0, literal_universal_tag.size(), line.str, line.len))
      selectors.emplace_back(UNIVERSAL_TAG);
    else if (literal_deprel.compare(0, literal_deprel.size(), line.str, line.len))
      selectors.emplace_back(DEPREL);
    else
      return error.assign("Cannot parse value selector '").append(line.str, line.len).append("'!"), false;
  }
  return true;
}

} // namespace parsito
} // namespace ufal
