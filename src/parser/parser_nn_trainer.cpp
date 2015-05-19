// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <fstream>
#include <random>
#include <unordered_set>

#include "configuration/node_extractor.h"
#include "configuration/value_extractor.h"
#include "embedding/embedding.h"
#include "parser_nn_trainer.h"
#include "transition/transition_system.h"
#include "utils/parse_double.h"
#include "utils/parse_int.h"
#include "utils/split.h"

namespace ufal {
namespace parsito {

void parser_nn_trainer::train(bool /*direct_connections*/, unsigned /*hidden_layer_size*/, const string& /*hidden_layer_type*/,
                              const string& transition_system_name, const string& transition_oracle_name,
                              const string& embeddings_description, const string& nodes_description, unsigned /*threads*/,
                              const vector<tree>& train, const vector<tree>& /*heldout*/, binary_encoder& enc) {
  if (train.empty()) runtime_failure("No training data was given!");

  // Random generator with fixed seed for reproducibility
  mt19937 generator(42);

  // Check that all non-root nodes have heads and nonempty deprel
  for (auto&& tree : train)
    for (auto&& node : tree.nodes)
      if (node.id) {
        if (node.head < 0) runtime_failure("The node '" << node.form << "' with id " << node.id << " has no head set!");
        if (node.deprel.empty()) runtime_failure("The node '" << node.form << "' with id " << node.id << " has no deprel set!");
      }

  // Generate labels for transition system
  vector<string> labels;
  unordered_set<string> labels_set;

  for (auto&& tree : train)
    for (auto&& node : tree.nodes)
      if (node.id && !labels_set.count(node.deprel)) {
        labels.push_back(node.deprel);
        labels_set.insert(node.deprel);
      }

  // Create transition system and transition oracle
  unique_ptr<transition_system> system(transition_system::create(transition_system_name, labels));
  if (!system) runtime_failure("Cannot create transition system '" << transition_system_name << "'!");

  unique_ptr<transition_oracle> oracle(system->oracle(transition_oracle_name));
  if (!oracle) runtime_failure("Cannot create transition oracle '" << transition_oracle_name << "' for transition system '" << transition_system_name << "'!");

  // Create node_extractor
  string error;
  node_extractor nodes;
  if (!nodes.create(nodes_description, error)) runtime_failure(error);

  // Load value_extractors and embeddings
  vector<string> value_names;
  vector<value_extractor> values;
  vector<embedding> embeddings;

  vector<string_piece> lines, tokens;
  split(embeddings_description, '\n', lines);
  for (auto&& line : lines) {
    // Ignore empty lines and comments
    if (!line.len || line.str[0] == '#') continue;

    split(line, ' ', tokens);
    if (!(tokens.size() >= 4 && tokens.size() <= 5))
      runtime_failure("Expected 4 or 5 columns on embedding description line '" << line << "'!");

    value_names.emplace_back(string(tokens[0].str, tokens[0].len));
    values.emplace_back();
    if (!values.back().create(tokens[0], error)) runtime_failure(error);

    int max_size = parse_int(tokens[1], "maximum embedding size");
    int dimension = parse_int(tokens[2], "embedding dimension");
    double update_weight = parse_double(tokens[3], "embedding dimension");


    vector<pair<string, vector<float>>> weights;
    if (tokens.size() >= 5) {
      // Load embedding if it was given
      ifstream in(string(tokens[4].str, tokens[4].len));
      if (!in.is_open()) runtime_failure("Cannot load '" << tokens[0] << "' embedding from file '" << tokens[4] << "'!");

      // Load first line containing dictionary size and dimensions
      string line;
      vector<string_piece> parts;
      if (!getline(in, line)) runtime_failure("Cannot read first line from embedding file '" << tokens[4] << "'!");
      split(line, ' ', parts);
      if (parts.size() != 2) runtime_failure("Expected two numbers on the first line of embedding file '" << tokens[4] << "'!");
      int file_dimension = parse_int(parts[1], "embedding file dimension");

      if (file_dimension < dimension) runtime_failure("The embedding file '" << tokens[4] << "' has lower dimension than required!");

      // Generate random projection when smaller dimension is required
      vector<vector<float>> projection;
      if (file_dimension > dimension) {
        cerr << "Reducing dimension of '" << tokens[0] << "' embedding from " << file_dimension << " to " << dimension << "." << endl;

        uniform_real_distribution<double> uniform(0, 1);
        projection.resize(dimension);
        for (auto&& row : projection) {
          row.resize(file_dimension);
          for (auto&& weight : row) weight = uniform(generator);

          double sum = 0;
          for (auto&& weight : row) sum += weight;
          for (auto&& weight : row) weight /= sum;
        }
      }

      // Load input embedding
      vector<double> input_weights(file_dimension);
      vector<float> projected_weights(dimension);
      while (getline(in, line) && (max_size <= 0 || int(weights.size()) < max_size)) {
        split(line, ' ', parts);
        if (!parts.empty() && !parts.back().len) parts.pop_back(); // Ignore space at the end of line
        if (int(parts.size()) != file_dimension + 1) runtime_failure("Wrong number of values on line '" << line << "' of embedding file '" << tokens[4]);
        for (int i = 0; i < file_dimension; i++)
          input_weights[i] = parse_double(parts[1 + i], "embedding weight");

        for (int i = 0; i < dimension; i++)
          if (file_dimension == dimension) {
            projected_weights[i] = input_weights[i];
          } else {
            projected_weights[i] = 0;
            for (int j = 0; j < file_dimension; j++)
              projected_weights[i] += projection[i][j] * input_weights[j];
          }

        weights.emplace_back(string(parts[0].str, parts[0].len), projected_weights);
      }
    } else {
      // Generate embedding for max_size most frequent words
      string word;
      unordered_map<string, unsigned> counts;
      for (auto&& tree : train)
        for (auto&& node : tree.nodes)
          if (node.id) {
            values.back().extract(node, word);
            counts[word]++;
          }

      vector<pair<int, string>> sorted_counts;
      for (auto&& count : counts)
        sorted_counts.emplace_back(-count.second, count.first);

      sort(sorted_counts.begin(), sorted_counts.end());

      vector<float> word_weights(dimension);
      normal_distribution<float> normal;
      for (auto&& count : sorted_counts) {
        if ((max_size > 0 && int(weights.size()) >= max_size)) break;
        for (auto&& word_weight : word_weights)
          word_weight = normal(generator);

        weights.emplace_back(count.second, word_weights);
      }
    }

    // Add the embedding
    embeddings.emplace_back();
    embeddings.back().create(dimension, update_weight, weights);

    // Count the cover of this embedding
    string word;
    unsigned words_total = 0, words_covered = 0;
    for (auto&& tree : train)
      for (auto&& node : tree.nodes)
        if (node.id) {
          values.back().extract(node, word);
          words_total++;
          words_covered += embeddings.back().lookup_word(word) >= 0;
        }

    cerr << "Initialized '" << tokens[0] << "' embedding with " << weights.size() << " words and " << 100. * words_covered / words_total << "% coverage." << endl;
  }

  // Encode transition system and oracle
  enc.add_2B(labels.size());
  for (auto&& label : labels)
    enc.add_str(label);
  enc.add_str(transition_system_name);
  enc.add_str(transition_oracle_name);

  // Encode nodes selector
  enc.add_str(nodes_description);

  // Encode value extractors and embeddings
  enc.add_2B(values.size());
  for (auto&& value_name : value_names)
    enc.add_str(value_name);
  for (auto&& embedding : embeddings)
    embedding.save(enc);
}

} // namespace parsito
} // namespace ufal
