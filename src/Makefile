include Makefile.builtem

EXECUTABLES = parsito_accuracy run_parsito train_parsito
LIBRARIES = libparsito

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,.)

PARSITO_O = configuration/configuration configuration/node_extractor
PARSITO_O += configuration/value_extractor embedding/embedding network/neural_network
PARSITO_O += parser/parser parser/parser_nn transition/transition
PARSITO_O += transition/transition_system transition/transition_system_link2
PARSITO_O += transition/transition_system_projective transition/transition_system_swap
PARSITO_O += tree/tree tree/tree_format unilib/unicode unilib/utf8
PARSITO_O += utils/compressor_load version/version

# library
$(call lib,libparsito): $(call obj,$(PARSITO_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,parsito_accuracy): $(call obj,parsito_accuracy utils/options $(PARSITO_O))
$(call exe,run_parsito): $(call obj,run_parsito utils/options $(PARSITO_O))
$(call exe,train_parsito): LD_FLAGS += $(call use_threads)
$(call exe,train_parsito): $(call obj,train_parsito embedding/embedding_encode network/neural_network_trainer parser/parser_nn_trainer utils/compressor_save utils/options $(PARSITO_O))
$(call exe,$(EXECUTABLES)):
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
