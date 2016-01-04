// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <unordered_map>

#include "common.h"
#include "microrestd/microrestd.h"
#include "parser/parser.h"
#include "tree/tree_format.h"

namespace ufal {
namespace parsito {

namespace microrestd = ufal::microrestd;

class parsito_service : public microrestd::rest_service {
 public:
  typedef ufal::parsito::parser Parser;

  struct model_description {
    string rest_id, file, acknowledgements;
    unsigned beam_size;

    model_description(const string& rest_id, const string& file, const string& acknowledgements, unsigned beam_size)
        : rest_id(rest_id), file(file), acknowledgements(acknowledgements), beam_size(beam_size) {}
  };

  bool init(const vector<model_description>& model_descriptions);

  virtual bool handle(microrestd::rest_request& req) override;

 private:
  static unordered_map<string, bool (parsito_service::*)(microrestd::rest_request&)> handlers;

  // Models
  struct model_info {
    model_info(const string& rest_id, const string& acknowledgements, Parser* parser, unsigned beam_size)
        : rest_id(rest_id), acknowledgements(acknowledgements), parser(parser), beam_size(beam_size) {}

    string rest_id;
    string acknowledgements;
    unique_ptr<Parser> parser;
    unsigned beam_size;
  };
  vector<model_info> models;
  unordered_map<string, const model_info*> rest_models_map;

  const model_info* load_rest_model(const string& rest_id, string& error);

  // REST service
  class rest_response_generator : public microrestd::json_response_generator {
   public:
    rest_response_generator(const model_info* model);
   protected:
    const model_info* model;
  };

  bool handle_rest_models(microrestd::rest_request& req);
  bool handle_rest_parse(microrestd::rest_request& req);

  const string& get_rest_model_id(microrestd::rest_request& req);
  const char* get_data(microrestd::rest_request& req, string& error);
  tree_input_format* get_input_format(microrestd::rest_request& req, string& error);
  tree_output_format* get_output_format(microrestd::rest_request& req, string& error);

  microrestd::json_builder json_models;

  static const string default_input_format;
  static const string default_output_format;
};

} // namespace parsito
} // namespace ufal
