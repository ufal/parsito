// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>

#include "common.h"

namespace ufal {
namespace parsito {

// Declarations
template <class T>
class threadsafe_stack {
 public:
  inline void push(T* t);
  inline T* pop();

  threadsafe_stack();
 private:
  vector<unique_ptr<T>> stack;

  // Unfortunately, Visual C++ 2013 does not support aggregate and list
  // initialization in non-static data members (see C2797), which is the only
  // way how atomic_flag can be initialized. Therefore, we initialize it
  // explicitly in the constructor.
  atomic_flag lock; // = ATOMIC_FLAG_INIT
};


// Definitions
template <class T>
threadsafe_stack<T>::threadsafe_stack() {
  // Needed because ATOMIC_FLAG_INIT cannot be used on Visual C++ 2013.
  lock.clear();

  // Allocate 16 slots in the stack so that we make reallocation during
  // lock less likely.
  stack.reserve(16);
}

template <class T>
void threadsafe_stack<T>::push(T* t) {
  while (lock.test_and_set(memory_order_acquire)) {}
  stack.emplace_back(t);
  lock.clear(memory_order_release);
}

template <class T>
T* threadsafe_stack<T>::pop() {
  T* res = nullptr;

  while (lock.test_and_set(memory_order_acquire)) {}
  if (!stack.empty()) {
    res = stack.back().release();
    stack.pop_back();
  }
  lock.clear(memory_order_release);

  return res;
}

} // namespace parsito
} // namespace ufal
