// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef UFAL_PARSITO_H
#define UFAL_PARSITO_H

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace ufal {
namespace parsito {

struct string_piece {
  const char* str;
  size_t len;

  string_piece() : str(nullptr), len(0) {}
  string_piece(const char* str) : str(str), len(strlen(str)) {}
  string_piece(const char* str, size_t len) : str(str), len(len) {}
  string_piece(const string& str) : str(str.c_str()), len(str.size()) {}
};

class node {
 public:
  int id;         // 0 is root, >0 is sentence node, <0 is undefined
  string form;    // form
  string lemma;   // lemma
  string upostag; // universal part-of-speech tag
  string xpostag; // language-specific part-of-speech tag
  string feats;   // list of morphological features
  int head;       // head, 0 is root, <0 is without parent
  string deprel;  // dependency relation to the head
  string deps;    // secondary dependencies
  string misc;    // miscellaneous information

  vector<int> children;

  node(int id = -1, const string& form = string()) : id(id), form(form), head(-1) {}
};

class tree {
 public:
  tree();

  vector<node> nodes;

  bool empty();
  void clear();
  node& add_node(const string& form);
  void set_head(int id, int head, const string& deprel);
  void unlink_all_nodes();

  static const string root_form;
};

// Input format
class tree_input_format {
 public:
  virtual ~tree_input_format() {}

  virtual bool read_block(istream& in, string& block) const = 0;
  virtual void set_block(string_piece block) = 0;
  virtual bool next_tree(tree& t, string& error) = 0;

  // Static factory methods
  static tree_input_format* new_input_format(const string& name);
  static tree_input_format* new_conllu_input_format();
};

// Output format
class tree_output_format {
 public:
  virtual ~tree_output_format() {}

  virtual void append_tree(const tree& t, string& block, const tree_input_format* additional_info = nullptr) const = 0;

  // Static factory methods
  static tree_output_format* new_output_format(const string& name);
  static tree_output_format* new_conllu_output_format();
};

// Parser
class parser {
 public:
  virtual ~parser() {};

  virtual void parse(tree& t) const = 0;

  enum { NO_CACHE = 0, FULL_CACHE = 2147483647};
  static parser* load(const char* file, unsigned cache = 1000);
  static parser* load(istream& in, unsigned cache = 1000);
};

} // namespace parsito
} // namespace ufal
