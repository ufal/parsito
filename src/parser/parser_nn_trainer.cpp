// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <atomic>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <random>
#include <thread>
#include <unordered_set>

#include "network/neural_network_trainer.h"
#include "parser_nn.h"
#include "parser_nn_trainer.h"
#include "utils/parse_double.h"
#include "utils/parse_int.h"
#include "utils/split.h"

namespace ufal {
namespace parsito {

void parser_nn_trainer::train(const string& transition_system_name, const string& transition_oracle_name,
                              const string& embeddings_description, const string& nodes_description, const network_parameters& parameters,
                              unsigned number_of_threads, const vector<tree>& train, const vector<tree>& heldout, binary_encoder& enc) {
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
  parser_nn parser;
  unordered_set<string> labels_set;

  for (auto&& tree : train)
    for (auto&& node : tree.nodes)
      if (node.id && !labels_set.count(node.deprel)) {
        labels_set.insert(node.deprel);
        parser.labels.push_back(node.deprel);
      }

  // Create transition system and transition oracle
  parser.system.reset(transition_system::create(transition_system_name, parser.labels));
  if (!parser.system) runtime_failure("Cannot create transition system '" << transition_system_name << "'!");

  unique_ptr<transition_oracle> oracle(parser.system->oracle(transition_oracle_name));
  if (!oracle) runtime_failure("Cannot create transition oracle '" << transition_oracle_name << "' for transition system '" << transition_system_name << "'!");

  // Create node_extractor
  string error;
  if (!parser.nodes.create(nodes_description, error)) runtime_failure(error);

  // Load value_extractors and embeddings
  vector<string> value_names;
  vector<string_piece> lines, tokens;
  split(embeddings_description, '\n', lines);
  for (auto&& line : lines) {
    // Ignore empty lines and comments
    if (!line.len || line.str[0] == '#') continue;

    split(line, ' ', tokens);
    if (!(tokens.size() >= 4 && tokens.size() <= 5))
      runtime_failure("Expected 4 or 5 columns on embedding description line '" << line << "'!");

    value_names.emplace_back(string(tokens[0].str, tokens[0].len));
    parser.values.emplace_back();
    if (!parser.values.back().create(tokens[0], error)) runtime_failure(error);

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
      unordered_map<string, int> counts;
      for (auto&& tree : train)
        for (auto&& node : tree.nodes)
          if (node.id) {
            parser.values.back().extract(node, word);
            counts[word]++;
          }

      vector<pair<int, string>> sorted_counts;
      for (auto&& count : counts)
        sorted_counts.emplace_back(-count.second, count.first);

      sort(sorted_counts.begin(), sorted_counts.end());

      vector<float> word_weights(dimension);
      normal_distribution<float> normal(0, 1);
      for (auto&& count : sorted_counts) {
        if ((max_size > 0 && int(weights.size()) >= max_size)) break;
        for (auto&& word_weight : word_weights)
          word_weight = normal(generator);

        weights.emplace_back(count.second, word_weights);
      }
    }

    // Add the embedding
    parser.embeddings.emplace_back();
    parser.embeddings.back().create(dimension, update_weight, weights);

    // Count the cover of this embedding
    string word, buffer;
    unsigned words_total = 0, words_covered = 0;
    for (auto&& tree : train)
      for (auto&& node : tree.nodes)
        if (node.id) {
          parser.values.back().extract(node, word);
          words_total++;
          words_covered += parser.embeddings.back().lookup_word(word, buffer) >= 0;
        }

    cerr << "Initialized '" << tokens[0] << "' embedding with " << weights.size() << " words and " << 100. * words_covered / words_total << "% coverage." << endl;
  }

  // Train the network
  unsigned total_dimension = 0, total_nodes = 0;
  for (auto&& embedding : parser.embeddings) total_dimension += embedding.dimension;
  for (auto&& tree : train) total_nodes += tree.nodes.size() - 1;
  auto scaled_parameters = parameters;
  scaled_parameters.l1_regularization /= train.size();
  scaled_parameters.l2_regularization /= total_nodes;
  neural_network_trainer network_trainer(parser.network, total_dimension * parser.nodes.node_count(), parser.system->transition_count(), scaled_parameters, generator);

  vector<int> permutation;
  for (size_t i = 0; i < train.size(); i++)
    permutation.push_back(permutation.size());

  for (int iteration = 1; network_trainer.next_iteration(); iteration++) {
    // Train on training data
    shuffle(permutation.begin(), permutation.end(), generator);

    atomic<unsigned> atomic_index(0);
    atomic<double> atomic_logprob(0);
    auto training = [&]() {
      tree t;
      configuration conf;
      string word, word_buffer;
      vector<vector<int>> nodes_embeddings;
      vector<int> extracted_nodes;
      vector<const vector<int>*> extracted_embeddings;
      neural_network_trainer::workspace workspace;
      double logprob = 0;

      for (unsigned current_index; (current_index = atomic_index++) < permutation.size();) {
        const tree& gold = train[permutation[current_index]];
        t = gold;
        t.unlink_all_nodes();
        conf.init(&t);

        // Compute embeddings
        if (t.nodes.size() > nodes_embeddings.size()) nodes_embeddings.resize(t.nodes.size());
        for (size_t i = 0; i < t.nodes.size(); i++) {
          nodes_embeddings[i].resize(parser.embeddings.size());
          for (size_t j = 0; j < parser.embeddings.size(); j++) {
            parser.values[j].extract(t.nodes[i], word);
            nodes_embeddings[i][j] = parser.embeddings[j].lookup_word(word, word_buffer);
          }
        }

        // Train the network
        while (!conf.final()) {
          // Extract nodes
          parser.nodes.extract(conf, extracted_nodes);
          extracted_embeddings.resize(extracted_nodes.size());
          for (size_t i = 0; i < extracted_nodes.size(); i++)
            extracted_embeddings[i] = extracted_nodes[i] >= 0 ? &nodes_embeddings[extracted_nodes[i]] : nullptr;

          // Propagate
          network_trainer.propagate(parser.embeddings, extracted_embeddings, workspace);

          // Find most probable applicable transition
          int network_best = -1;
          for (unsigned i = 0; i < workspace.outcomes.size(); i++)
            if (parser.system->applicable(conf, i) && (network_best < 0 || workspace.outcomes[i] > workspace.outcomes[network_best]))
              network_best = i;

          // Apply the oracle
          auto prediction = oracle->predict(conf, gold, network_best);

          // Update logprob
          if (workspace.outcomes[prediction.best])
            logprob += log(workspace.outcomes[prediction.best]);

          // Backpropagate the chosen outcome
          network_trainer.backpropagate(parser.embeddings, extracted_embeddings, prediction.best, workspace);

          // Follow the chosen outcome
          int child = parser.system->perform(conf, prediction.to_follow);

          // If a node was linked, recompute its not-found embeddings as deprel has changed
          if (child >= 0)
            for (size_t i = 0; i < parser.embeddings.size(); i++)
              if (nodes_embeddings[child][i] < 0) {
                parser.values[i].extract(t.nodes[child], word);
                nodes_embeddings[child][i] = parser.embeddings[i].lookup_word(word, word_buffer);
              }
        }

        // FinishL1 regularize after processing the whole sentence
        network_trainer.finalize_sentence();
      }
      for (double old_atomic_logprob = atomic_logprob; atomic_logprob.compare_exchange_weak(old_atomic_logprob, old_atomic_logprob + logprob); ) {}
    };

    cerr << "Iteration " << iteration << ": ";
    if (number_of_threads > 1) {
      vector<thread> threads;
      for (unsigned i = 0; i < number_of_threads; i++) threads.emplace_back(training);
      for (; !threads.empty(); threads.pop_back()) threads.back().join();
    } else {
      training();
    }
    cerr << "training logprob " << scientific << setprecision(4) << atomic_logprob;

    // Evaluate heldout data if present
    if (!heldout.empty()) {
      tree t;

      unsigned total = 0, correct_unlabelled = 0, correct_labelled = 0;
      for (auto&& gold : heldout) {
        t = gold;
        t.unlink_all_nodes();
        parser.parse(t);
        for (size_t i = 1; i < t.nodes.size(); i++) {
          total++;
          correct_unlabelled += t.nodes[i].head == gold.nodes[i].head;
          correct_labelled += t.nodes[i].head == gold.nodes[i].head && t.nodes[i].deprel == gold.nodes[i].deprel;
        }
      }

      cerr << ", heldout UAS " << fixed << setprecision(2) << (100. * correct_unlabelled / total) << "%, LAS " << (100. * correct_labelled / total) << "%";
    }

    cerr << endl;
  }

  // Encode transition system
  enc.add_2B(parser.labels.size());
  for (auto&& label : parser.labels)
    enc.add_str(label);
  enc.add_str(transition_system_name);

  // Encode nodes selector
  enc.add_str(nodes_description);

  // Encode value extractors and embeddings
  enc.add_2B(value_names.size());
  for (auto&& value_name : value_names)
    enc.add_str(value_name);
  for (auto&& embedding : parser.embeddings)
    embedding.save(enc);

  // Encode the network
  network_trainer.save_network(enc);
}

} // namespace parsito
} // namespace ufal
