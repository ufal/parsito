// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

%include "parsito_stl.i"

%{
#include "parsito.h"
using namespace ufal::parsito;
%}

%template(Children) std::vector<int>;
typedef std::vector<int> Children;

%rename(Node) node;
class node {
 public:
  int id;              // 0 is root, >0 is sentence node, <0 is undefined
  std::string form;    // form
  std::string lemma;   // lemma
  std::string upostag; // universal part-of-speech tag
  std::string xpostag; // language-specific part-of-speech tag
  std::string feats;   // list of morphological features
  int head;            // head, 0 is root, <0 is without parent
  std::string deprel;  // dependency relation to the head
  std::string deps;    // secondary dependencies
  std::string misc;    // miscellaneous information

  std::vector<int> children;

  node(int id = -1, const std::string& form = std::string()) : id(id), form(form), head(-1) {}
};
%template(Nodes) std::vector<node>;
typedef std::vector<node> Nodes;

%rename(Tree) tree;
class tree {
 public:
  tree();

  std::vector<node> nodes;

  bool empty();
  void clear();
  %rename(addNode) add_node;
  node& add_node(const std::string& form);
  %rename(setHead) set_head;
  void set_head(int id, int head, const std::string& deprel);
  %rename(unlinkAllNodes) unlink_all_nodes;
  void unlink_all_nodes();

  static const std::string root_form;
};

%rename(TreeInputFormat) tree_input_format;
%nodefaultctor tree_input_format;
class tree_input_format {
 public:
  virtual ~tree_input_format() {}

  %extend {
    %rename(setText) set_text;
    virtual void set_text(const char* text) {
      $self->set_text(text, true);
    }
  }
  %rename(nextTree) next_tree;
  virtual bool next_tree(tree& t) = 0;
  %rename(lastError) last_error;
  const std::string& last_error() const;

  // Static factory methods
  %rename(newInputFormat) new_input_format;
  %newobject new_input_format;
  static tree_input_format* new_input_format(const std::string& name);
  %rename(newConlluInputFormat) new_conllu_input_format;
  %newobject new_conllu_input_format;
  static tree_input_format* new_conllu_input_format();
};

%rename(TreeOutputFormat) tree_output_format;
%nodefaultctor tree_output_format;
class tree_output_format {
 public:
  virtual ~tree_output_format() {}

  %extend {
    %rename(writeTree) write_tree;
    virtual std::string write_tree(const tree& t, const tree_input_format* additional_info = nullptr) const {
      std::string output;
      $self->write_tree(t, output, additional_info);
      return output;
    }
  }

  // Static factory methods
  %rename(newOutputFormat) new_output_format;
  %newobject new_output_format;
  static tree_output_format* new_output_format(const std::string& name);
  %rename(newConlluOutputFormat) new_conllu_output_format;
  %newobject new_conllu_output_format;
  static tree_output_format* new_conllu_output_format();
};

%rename(Version) version;
struct version {
  unsigned major;
  unsigned minor;
  unsigned patch;
  std::string prerelease;

  // Returns current version.
  static version current();
};

%rename(Parser) parser;
%nodefaultctor parser;
class parser {
 public:
  virtual ~parser() {};

  virtual void parse(tree& t) const = 0;

  enum { NO_CACHE = 0, FULL_CACHE = 2147483647};
  %newobject load;
  static parser* load(const char* file, unsigned cache = 1000);
};
