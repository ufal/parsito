# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include Makefile.builtem
include Makefile.lib

EXECUTABLES = $(call exe,parsito_accuracy run_parsito train_parsito)
LIBRARIES = $(call lib,libparsito)

.PHONY: all exe lib full
all: exe
exe: $(EXECUTABLES)
lib: $(LIBRARIES)
full: exe lib

# library
$(call lib,libparsito): $(call obj,$(PARSITO_O))
	$(call link_lib,$@,$^)

C_FLAGS += $(call include_dir,.)
# executables
$(call exe,train_parsito): LD_FLAGS += $(call use_threads)
$(call exe,train_parsito): $(call obj,embedding/embedding_encode network/neural_network_trainer parser/parser_nn_trainer utils/compressor_save)
$(EXECUTABLES): $(call exe,%): $$(call obj,% utils/options $(PARSITO_O))
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
