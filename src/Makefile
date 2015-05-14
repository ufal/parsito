include Makefile.builtem

EXECUTABLES = $(call exe,run_parsito train_parsito)
LIBRARIES = $(call lib,libparsito)

.PHONY: all exe lib full
all: exe
exe: $(call exe,$(EXECUTABLES))
lib: $(call lib,$(LIBRARIES))
full: exe lib

# compilation
C_FLAGS += $(call include_dir,.)

LZMA_DEC_O = utils/compressor_load utils/lzma/LzmaDec
LZMA_ENC_O = utils/compressor_save utils/lzma/LzmaEnc utils/lzma/LzFind
PARSITO_O = $(LZMA_DEC_O) tree/tree tree/tree_format
PARSITO_O += version/version

# library
$(call lib,libparsito): $(call obj,$(PARSITO_O))
	$(call link_lib,$@,$^)

# executables
$(call exe,train_parsito): $(call obj,train_parsito utils/input utils/options $(PARSITO_O) $(LZMA_ENC_O))
$(call exe,run_parsito): $(call obj,run_parsito utils/input utils/options $(PARSITO_O))
$(call exe,$(EXECUTABLES)):
	$(call link_exe,$@,$^,$(call win_subsystem,console))

# cleaning
.PHONY: clean
clean:
	$(call rm,.build $(call all_exe,$(EXECUTABLES)) $(call all_lib,$(LIBRARIES)))
