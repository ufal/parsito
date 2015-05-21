include Makefile.builtem

EXECUTABLES = run_parsito train_parsito
LIBRARIES = libparsito

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,.)

LZMA_DEC_O = utils/compressor_load utils/lzma/LzmaDec
LZMA_ENC_O = utils/compressor_save utils/lzma/LzmaEnc utils/lzma/LzFind
PARSITO_O = $(LZMA_DEC_O) configuration/configuration configuration/node_extractor
PARSITO_O += configuration/value_extractor embedding/embedding network/neural_network
PARSITO_O += parser/parser parser/parser_nn transition/transition
PARSITO_O += transition/transition_system transition/transition_system_projective
PARSITO_O += tree/tree tree/tree_format unilib/unicode unilib/utf8
PARSITO_O += utils/parse_int utils/split version/version

# library
$(call lib,libparsito): $(call obj,$(PARSITO_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,run_parsito): $(call obj,run_parsito utils/iostream_init utils/options $(PARSITO_O))
$(call exe,train_parsito): LD_FLAGS += $(call use_threads)
$(call exe,train_parsito): $(call obj,train_parsito embedding/embedding_encode network/neural_network_trainer parser/parser_nn_trainer utils/iostream_init utils/options utils/parse_double $(PARSITO_O) $(LZMA_ENC_O))
$(call exe,$(EXECUTABLES)):
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
