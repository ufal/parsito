# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include ../common/Makefile.common

BINDING_MODULE=$(call dynlib,auto/Ufal/Parsito/Parsito)
BINDING_C_FLAGS+=$(if $(PERL_INCLUDE),,$(error PERL_INCLUDE must be set to the directory containing perl.h))
BINDING_C_FLAGS+=$(call include_dir,$(PERL_INCLUDE))
BINDING_C_FLAGS+=$(if $(filter %-gcc %-clang,$(PLATFORM)),-w)
BINDING_C_FLAGS+=$(if $(filter osx-%,$(PLATFORM)),-Wno-reserved-user-defined-literal,)

all: $(BINDING_MODULE)

$(call dynobj,parsito_perl):C_FLAGS+=$(BINDING_C_FLAGS)
$(BINDING_MODULE): $(call dynobj,parsito_perl) $(BINDING_PARSITO_O)
	@$(call mkdir,$(dir $@))
	@$(call touch,$(dir $@)Parsito.bs)
	$(call link_dynlib,$@,$^ $(BINDING_C_FLAGS), $(BINDING_LD_FLAGS))

parsito_perl.cpp: parsito_perl.i ../common/parsito.i
	@$(call mkdir,Ufal)
	swig $(SWIG_FLAGS) -perl -outdir Ufal -o $@ $<

.PHONY: clean
clean:
	@$(call rm,.build Ufal auto parsito_perl.cpp)
