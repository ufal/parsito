# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

TARGETS = RunParsito.class

all: $(TARGETS)

%.class: %.java
	javac -cp .. $<

%: %.class
	LD_LIBRARY_PATH=.. java -cp .:.. $* $(ARGS)

.PHONY: clean
clean:
	rm -rf $(TARGETS)
