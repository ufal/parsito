# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include ../src/Makefile.builtem
include ../src/Makefile.include

MERGE=$(call exe,merge_sources/merge_sources)

parsito.cpp: $(MERGE) force
	@$(call echo,>$@,// This file is part of Parsito <http://github.com/ufal/parsito/>.)
	@$(call echo,>>$@,//)
	@$(call echo,>>$@,// This file is a bundle of all sources and headers of Parsito library.)
	@$(call echo,>>$@,// Comments and copyrights of all individual files are kept.)
	$(call platform_name,$(MERGE) ufal::parsito ../src/ $(patsubst %,%.cpp,$(PARSITO_OBJECTS))) >>$@

$(MERGE):$(call exe,%):$$(call obj,%)
	$(call link_exe,$@,$^,$(call win_subsystem,console))

.PHONY: force
force:

.PHONY: clean
clean:
	@$(call rm,.build parsito.cpp $(call all_exe,$(MERGE)))
