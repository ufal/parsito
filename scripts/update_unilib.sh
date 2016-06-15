#!/bin/bash

# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2016 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

rm -rf ../src/unilib/*
git -C ../src/unilib clone --depth=1 --branch=stable https://github.com/ufal/unilib
mv ../src/unilib/unilib/unilib/* ../src/unilib/
mv ../src/unilib/unilib/{AUTHORS,CHANGES,LICENSE,README} ../src/unilib/
rm -rf ../src/unilib/unilib/
sed '
  s/^namespace unilib {/namespace parsito {\n&/
  s/^} \/\/ namespace unilib/&\n} \/\/ namespace parsito/
  ' -i ../src/unilib/*
