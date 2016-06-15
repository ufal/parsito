#!/bin/bash

# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2016 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

rm -rf ../src/utils/*
git -C ../src/utils clone --depth=1 --branch=stable https://github.com/ufal/cpp_utils
mv ../src/utils/cpp_utils/src/* ../src/utils/
mv ../src/utils/cpp_utils/{AUTHORS,CHANGES,LICENSE,README} ../src/utils/
rm -rf ../src/utils/cpp_utils/
sed '
  s/^namespace utils {/namespace parsito {\n&/
  s/^} \/\/ namespace utils/&\n} \/\/ namespace parsito/
  ' -i ../src/utils/*
