// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "parser_nn.h"

namespace ufal {
namespace parsito {

void parser_nn::parse(tree& t) const {
  assert(system);

  // Retrieve or create workspace
  workspace* w = workspaces.pop();
  if (!w) w = new workspace();

  // Create configuration
  w->conf.init(&t);

  // Compute embeddings of all nodes
  if (w->embeddings.size() < t.nodes.size()) w->embeddings.resize(t.nodes.size());
  for (size_t i = 0; i < t.nodes.size(); i++) {
    if (w->embeddings[i].size() < embeddings.size()) w->embeddings[i].resize(embeddings.size());
    for (size_t j = 0; j < embeddings.size(); j++) {
      values[j].extract(t.nodes[i], w->word);
      w->embeddings[i][j] = embeddings[j].lookup_word(w->word, w->word_buffer);
    }
  }

  // Compute which transitions to perform and perform them
  while (!w->conf.final()) {
    // Extract nodes from the configuration
    nodes.extract(w->conf, w->extracted_nodes);
    w->extracted_embeddings.resize(w->extracted_nodes.size());
    for (size_t i = 0; i < w->extracted_nodes.size(); i++)
      w->extracted_embeddings[i] = w->extracted_nodes[i] >= 0 ? &w->embeddings[w->extracted_nodes[i]] : nullptr;

    // Classify using neural network
    network.propagate(embeddings, w->extracted_embeddings, w->network_buffer, w->outcomes);

    // Find most probable applicable transition
    int best = -1;
    for (unsigned i = 0; i < w->outcomes.size(); i++)
      if (system->applicable(w->conf, i) && (best < 0 || w->outcomes[i] > w->outcomes[best]))
        best = i;

    // Perform the best transition
    int child = system->perform(w->conf, best);

    // If a node was linked, recompute its embeddings as deprel has changed
    if (child >= 0)
      for (size_t i = 0; i < embeddings.size(); i++) {
        values[i].extract(t.nodes[child], w->word);
        w->embeddings[child][i] = embeddings[i].lookup_word(w->word, w->word_buffer);
      }
  }

  // Store workspace
  workspaces.push(w);
}

void parser_nn::load(binary_decoder& data) {
  string description, error;

  // Load labels
  labels.resize(data.next_2B());
  for (auto&& label : labels)
    data.next_str(label);

  // Load transition system
  string system_name;
  data.next_str(system_name);
  system.reset(transition_system::create(system_name, labels));
  if (!system) throw binary_decoder_error("Cannot load transition system");

  // Load node extractor
  data.next_str(description);
  if (!nodes.create(description, error))
    throw binary_decoder_error(error.c_str());

  // Load value extractors and embeddings
  values.resize(data.next_2B());
  for (auto&& value : values) {
    data.next_str(description);
    if (!value.create(description, error))
      throw binary_decoder_error(error.c_str());
  }

  embeddings.resize(values.size());
  for (auto&& embedding : embeddings)
    embedding.load(data);

  // Load the network
  network.load(data);
}

} // namespace parsito
} // namespace ufal
