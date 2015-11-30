# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TARGETS = RunParsito.exe

all: $(TARGETS)

%.exe: %.cs
	mcs $< $(wildcard ../Ufal/Parsito/*)

.PHONY: clean
clean:
	rm -rf $(TARGETS)
