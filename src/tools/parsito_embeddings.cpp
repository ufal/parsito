// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2016 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <fstream>

#include "common.h"
#include "embedding/embedding.h"
#include "utils/binary_decoder.h"
#include "utils/binary_encoder.h"
#include "utils/compressor.h"
#include "utils/iostreams.h"
#include "utils/options.h"
#include "utils/parse_double.h"
#include "utils/parse_int.h"
#include "utils/split.h"
#include "version/version.h"

using namespace ufal::parsito;

int main(int argc, char* argv[]) {
  iostreams_init();

  options::map options;
  if (!options::parse({{"precision", options::value::any},
                       {"version", options::value::none},
                       {"help", options::value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 4 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " [options] [extract|replace] embedding_name model_file\n"
                    "Options: --precision=precision of extraced embeddings [6]\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  // Parse options
  if (argv[1] != string("extract") && argv[1] != string("replace"))
    runtime_failure("The first argument must be either 'extract' or 'replace'!");
  bool extract = argv[1] == string("extract");
  int precision = options.count("precision") ? parse_int(options["precision"], "precision option") : 6;
  string requested_embedding = argv[2];
  string model_file = argv[3];

  // Load model
  ifstream model_is(model_file, ifstream::in | ifstream::binary);
  if (!model_is.is_open()) runtime_failure("Cannot open file '" << model_file << "'!");

  binary_decoder model;
  if (!compressor::load(model_is, model)) runtime_failure("Cannot decompress model from the file '" << model_file << "'!");

  // Load the requested embeddings and its range
  embedding e;
  size_t e_start, e_end;

  try {
    // Model id
    string model_id;
    model.next_str(model_id);
    if (model_id != "nn") runtime_failure("Only models with identifier 'nn' are currently supported!");

    // Labels
    string useless;
    for (int labels = model.next_2B(); labels; labels--)
      model.next_str(useless);

    // Transition system name
    model.next_str(useless);

    // Node extractor
    model.next_str(useless);

    // Value extractors and embeddings
    string value_name;
    int value_index = -1;
    for (int i = 0, values = model.next_2B(); i < values; i++) {
      model.next_str(value_name);
      if (value_name == requested_embedding)
        value_index = i;
    }
    if (value_index == -1) runtime_failure("The requested embedding with name '" << requested_embedding << "' was not found!");

    // Load embeddings up to the requested one
    for (int i = 0; i < value_index; i++)
      e.load(model);

    // Load the requested embedding
    e_start = model.tell();
    e.load(model);
    e_end = model.tell();
  } catch (binary_decoder_error& e) {
    runtime_failure("Cannot load model from the file '" << model_file << "': " << e.what() << "!");
  }

  if (extract) {
    // Extract the embeddings
    vector<pair<string, vector<float>>> words;
    vector<float> unknown_word;
    e.export_embeddings(words, unknown_word);

    // Header
    cout << words.size() + (!unknown_word.empty()) << ' ' << e.dimension << "\n" << fixed << setprecision(precision);

    // Unknown word embedding, if present
    if (!unknown_word.empty()) {
      cout << "<unk>";
      for (auto&& weight : unknown_word) cout << ' ' << weight;
      cout << "\n";
    }

    // The embeddings
    for (auto&& word : words) {
      cout << word.first;
      for (auto&& weight : word.second) cout << ' ' << weight;
      cout << "\n";
    }
  } else {
    // Replace the embeddings
    vector<pair<string, vector<float>>> words;
    vector<float> unknown_word;

    // Start by loading embeddings from the input
    string line;
    vector<string_piece> parts;
    if (!getline(cin, line)) runtime_failure("Cannot read first line of embedding data from standard input!");
    split(line, ' ', parts);
    if (parts.size() != 2) runtime_failure("Expected two numbers on the first line of embedding data!");
    if (e.dimension != unsigned(parse_int(parts[1], "embedding data dimension")))
      runtime_failure("The embedding data must have the same dimension as the embedding being replaced!");

    while (getline(cin, line)) {
      split(line, ' ', parts);
      if (!parts.empty() && !parts.back().len) parts.pop_back(); // Ignore space at the end of line
      if (parts.size() != e.dimension + 1) runtime_failure("Wrong number of values on line '" << line << "' of embedding data!");

      string word = string(parts[0].str, parts[0].len);
      if (word == "<unk>" && !unknown_word.empty()) runtime_failure("There are multiple embeddings for '<unk>'!");
      vector<float>& weights = word == "<unk>" ? unknown_word : (words.emplace_back(move(word), vector<float>()), words.back().second);
      for (unsigned i = 0; i < e.dimension; i++)
        weights.push_back(parse_double(parts[1 + i], "embedding weight"));
    }

    // Construct the embedding
    e.create(e.dimension, words.size(), words, unknown_word);

    // Create a model with the original embedding replaced
    binary_encoder enc;
    model.seek(0);
    while (model.tell() < e_start) enc.add_1B(model.next_1B());
    e.save(enc);
    model.seek(e_end);
    while (!model.is_end()) enc.add_1B(model.next_1B());

    // Compress and write the model
    iostreams_init_binary_output();
    compressor::save(cout, enc);
  }

  return 0;
}
