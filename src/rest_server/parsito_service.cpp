// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "parsito_service.h"

namespace ufal {
namespace parsito {

// Init the Parsito service -- load the models
bool parsito_service::init(const vector<model_description>& model_descriptions) {
  if (model_descriptions.empty()) return false;

  // Load models
  models.clear();
  rest_models_map.clear();
  for (auto& model_description : model_descriptions) {
    Parser* parser = parser::load(model_description.file.c_str());
    if (!parser) return false;

    // Store the model
    models.emplace_back(model_description.rest_id, model_description.acknowledgements, parser);
  }

  // Fill rest_models_map with model name and aliases
  for (auto& model : models) {
    // Fail if this model id is aready in use.
    if (!rest_models_map.emplace(model.rest_id, &model).second) return false;

    // Create (but not overwrite) id without version.
    for (unsigned i = 0; i+1+6 < model.rest_id.size(); i++)
      if (model.rest_id[i] == '-') {
        bool is_version = true;
        for (unsigned j = i+1; j < i+1+6; j++)
          is_version = is_version && model.rest_id[j] >= '0' && model.rest_id[j] <= '9';
        if (is_version)
          rest_models_map.emplace(model.rest_id.substr(0, i) + model.rest_id.substr(i+1+6), &model);
      }

    // Create (but not overwrite) hyphen-separated prefixes.
    for (unsigned i = 0; i < model.rest_id.size(); i++)
      if (model.rest_id[i] == '-')
        rest_models_map.emplace(model.rest_id.substr(0, i), &model);
  }
  // Default model
  rest_models_map.emplace(string(), &models.front());

  // Init REST service
  json_models.clear().object().indent().key("models").indent().array();
  for (auto& model : models) {
    json_models.indent().value(model.rest_id);
  }
  json_models.indent().close().indent().key("default_model").indent().value(model_descriptions.front().rest_id).finish(true);

  return true;
}

// Handlers with their URLs
unordered_map<string, bool (parsito_service::*)(microrestd::rest_request&)> parsito_service::handlers = {
  // REST service
  {"/models", &parsito_service::handle_rest_models},
  {"/parse", &parsito_service::handle_rest_parse},
};

// Handle a request using the specified URL/handler map
bool parsito_service::handle(microrestd::rest_request& req) {
  auto handler_it = handlers.find(req.url);
  return handler_it == handlers.end() ? req.respond_not_found() : (this->*handler_it->second)(req);
}

// Load selected model
const parsito_service::model_info* parsito_service::load_rest_model(const string& rest_id, string& error) {
  auto model_it = rest_models_map.find(rest_id);
  if (model_it == rest_models_map.end())
    return error.assign("Requested model '").append(rest_id).append("' does not exist.\n"), nullptr;

  return model_it->second;
}

// REST service
inline microrestd::string_piece sp(string_piece str) { return microrestd::string_piece(str.str, str.len); }
inline microrestd::string_piece sp(const char* str, size_t len) { return microrestd::string_piece(str, len); }

parsito_service::rest_response_generator::rest_response_generator(const model_info* model) {
  json.object();
  json.indent().key("model").indent().value(model->rest_id);
  json.indent().key("acknowledgements").indent().array();
  json.indent().value("http://ufal.mff.cuni.cz/parsito#parsito_acknowledgements");
  if (!model->acknowledgements.empty()) json.indent().value(model->acknowledgements);
  json.indent().close().indent().key("result").indent().value("");
}

// REST service handlers

bool parsito_service::handle_rest_models(microrestd::rest_request& req) {
  return req.respond(rest_response_generator::mime, json_models);
}

bool parsito_service::handle_rest_parse(microrestd::rest_request& req) {
  string error;
  auto rest_id = get_rest_model_id(req);
  auto model = load_rest_model(rest_id, error);
  if (!model) return req.respond_error(error);

  auto data = get_data(req, error); if (!data) return req.respond_error(error);

  unique_ptr<tree_input_format> input_format(get_input_format(req, error)); if (!input_format) return req.respond_error(error);
  unique_ptr<tree_output_format> output_format(get_output_format(req, error)); if (!output_format) return req.respond_error(error);

  // Try loading all input trees
  input_format->set_block(data);
  tree t;
  while (input_format->next_tree(t, error)) {}
  if (!error.empty()) return req.respond_error(error.insert(0, "Cannot parse input data: ").append("\n"));

  class generator : public rest_response_generator {
   public:
    generator(const model_info* model, const char* data, tree_input_format* input_format, tree_output_format* output_format, const Parser* parser)
        : rest_response_generator(model), input_format(input_format), output_format(output_format), parser(parser) {
      input_format->set_block(data);
    }

    bool generate() {
      if (!input_format->next_tree(t, error)) {
        json.finish(true);
        return false;
      }

      parser->parse(t);

      output_format->write_tree(t, output, input_format.get());
      json.value(output, true);

      return true;
    }

   private:
    tree t;
    string error, output;

    unique_ptr<tree_input_format> input_format;
    unique_ptr<tree_output_format> output_format;
    const Parser* parser;
  };
  return req.respond(generator::mime, new generator(model, data, input_format.release(), output_format.release(), model->parser.get()));
}

// REST service helpers

const string& parsito_service::get_rest_model_id(microrestd::rest_request& req) {
  static string empty;

  auto model_it = req.params.find("model");
  return model_it == req.params.end() ? empty : model_it->second;
}

const char* parsito_service::get_data(microrestd::rest_request& req, string& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.assign("Required argument 'data' is missing.\n"), nullptr;

  return data_it->second.c_str();
}

const string parsito_service::default_input_format = "conllu";

tree_input_format* parsito_service::get_input_format(microrestd::rest_request& req, string& error) {
  auto input_it = req.params.find("input");
  const string& input_format_name = input_it == req.params.end() ? default_input_format : input_it->second;
  auto input_format = tree_input_format::new_input_format(input_format_name);
  if (!input_format) return error.assign("Unknown input format '").append(input_format_name).append("'.\n"), nullptr;
  return input_format;
}

const string parsito_service::default_output_format = "conllu";

tree_output_format* parsito_service::get_output_format(microrestd::rest_request& req, string& error) {
  auto output_it = req.params.find("output");
  const string& output_format_name = output_it == req.params.end() ? default_output_format : output_it->second;
  auto output_format = tree_output_format::new_output_format(output_format_name);
  if (!output_format) return error.assign("Unknown output format '").append(output_format_name).append("'.\n"), nullptr;
  return output_format;
}

} // namespace parsito
} // namespace ufal
