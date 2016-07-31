# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include Makefile.builtem
include Makefile.include
include rest_server/microrestd/Makefile.include

EXECUTABLES = $(call exe,parsito_accuracy run_parsito train_parsito)
TOOLS = $(call exe,tools/parsito_embeddings)
SERVER = $(call exe,rest_server/parsito_server)
LIBRARIES = $(call lib,libparsito)

.PHONY: all exe tools server lib full
all: exe
exe: $(EXECUTABLES)
tools: $(TOOLS)
server: $(SERVER)
lib: $(LIBRARIES)
full: exe tools server lib

# library
$(call lib,libparsito): $(call obj,$(PARSITO_OBJECTS))
	$(call link_lib,$@,$^)

C_FLAGS += $(call include_dir,.)
# executables
$(call exe,train_parsito): LD_FLAGS += $(call use_threads)
$(call exe,train_parsito): $(call obj,embedding/embedding_encode network/neural_network_trainer parser/parser_nn_trainer utils/compressor_save)
$(call exe,rest_server/parsito_server): LD_FLAGS+=$(call use_library,$(if $(filter win-%,$(PLATFORM)),$(MICRORESTD_LIBRARIES_WIN),$(MICRORESTD_LIBRARIES_POSIX)))
$(call exe,rest_server/parsito_server): $(call obj,rest_server/parsito_service $(addprefix rest_server/microrestd/,$(MICRORESTD_OBJECTS) $(MICRORESTD_PUGIXML_OBJECTS)))
$(call exe,tools/parsito_embeddings): $(call obj,embedding/embedding_encode utils/compressor_save)
$(EXECUTABLES) $(TOOLS) $(SERVER): $(call exe,%): $$(call obj,% utils/options $(PARSITO_OBJECTS))
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES) $(TOOLS) $(SERVER)) $(call all_lib,$(LIBRARIES)))
