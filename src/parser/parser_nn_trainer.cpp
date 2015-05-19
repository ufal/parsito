// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <unordered_set>

#include "configuration/node_extractor.h"
#include "configuration/value_extractor.h"
#include "embedding/embedding.h"
#include "parser_nn_trainer.h"
#include "transition/transition_system.h"

namespace ufal {
namespace parsito {

void parser_nn_trainer::train(bool /*direct_connections*/, unsigned /*hidden_layer_size*/, const string& /*hidden_layer_type*/,
                              const string& transition_system_name, const string& transition_oracle_name,
                              const string& /*embeddings_description*/, const string& nodes_description, unsigned /*threads*/,
                              const vector<tree>& train, const vector<tree>& /*heldout*/, binary_encoder& enc) {
  string error;

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
  node_extractor nodes;
  if (!nodes.create(nodes_description, error)) runtime_failure(error);

  // Load value_extractors and embeddings
  vector<string> values_names;
  vector<value_extractor> values;
  vector<embedding> embeddings;

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
  for (auto&& value_name : values_names)
    enc.add_str(value_name);
  for (auto&& embedding : embeddings)
    embedding.save(enc);
}

} // namespace parsito
} // namespace ufal
