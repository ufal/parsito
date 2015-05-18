// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "common.h"
#include "configuration/configuration.h"
#include "tree/tree.h"

namespace ufal {
namespace parsito {

// Abstract transition class
class transition {
 public:
  virtual ~transition() {}

  virtual bool applicable(const configuration& c) const = 0;
  virtual void perform(configuration& c) const = 0;
};

// Specific transition classes
class transition_left_arc : public transition {
 public:
  transition_left_arc(const string& label) : label(label) {}

  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
 private:
  string label;
};

class transition_right_arc : public transition {
 public:
  transition_right_arc(const string& label) : label(label) {}

  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
 private:
  string label;
};

class transition_shift : public transition {
 public:
  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
};

class transition_swap : public transition {
 public:
  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
};

class transition_left_arc_2 : public transition {
 public:
  transition_left_arc_2(const string& label) : label(label) {}

  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
 private:
  string label;
};

class transition_right_arc_2 : public transition {
 public:
  transition_right_arc_2(const string& label) : label(label) {}

  virtual bool applicable(const configuration& c) const override;
  virtual void perform(configuration& c) const override;
 private:
  string label;
};

} // namespace parsito
} // namespace ufal
