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

const string value_extractor::literal_not_found = "</s>";

void value_extractor::extract(const node* n, string& value) const {
  if (!n)
    value.assign(literal_not_found);
  else
    switch (selector) {
      case FORM:
        value.assign(n->form);
        break;
      case LEMMA:
        value.assign(n->lemma);
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
            value.assign(n->misc, lid, lid_end - lid);
            break;
          }
        }
        value.assign(n->lemma);
        break;
      case TAG:
        value.assign(n->tag);
        break;
      case UNIVERSAL_TAG:
        value.assign(n->ctag);
        break;
      case DEPREL:
        value.assign(n->deprel);
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

  error.clear();

  if (literal_form.compare(0, literal_form.size(), description.str, description.len))
    selector = FORM;
  else if (literal_lemma.compare(0, literal_lemma.size(), description.str, description.len))
    selector = LEMMA;
  else if (literal_lemma_id.compare(0, literal_lemma_id.size(), description.str, description.len))
    selector = LEMMA_ID;
  else if (literal_tag.compare(0, literal_tag.size(), description.str, description.len))
    selector = TAG;
  else if (literal_universal_tag.compare(0, literal_universal_tag.size(), description.str, description.len))
    selector = UNIVERSAL_TAG;
  else if (literal_deprel.compare(0, literal_deprel.size(), description.str, description.len))
    selector = DEPREL;
  else
    return error.assign("Cannot parse value selector '").append(description.str, description.len).append("'!"), false;

  return true;
}

} // namespace parsito
} // namespace ufal
