# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include ../common/Makefile.common

BINDING_MODULE=$(call dynlib,$(if $(filter win-%,$(PLATFORM)),parsito_csharp,libparsito_csharp))
ifeq ($(PLATFORM),win-gcc)
  BINDING_LD_FLAGS+=-Wl,--kill-at
endif

all: $(BINDING_MODULE)

$(call dynobj,parsito_csharp):C_FLAGS+=$(BINDING_C_FLAGS)
$(BINDING_MODULE): $(call dynobj,parsito_csharp) $(BINDING_PARSITO_O)
	$(call link_dynlib,$@,$^ $(BINDING_C_FLAGS), $(BINDING_LD_FLAGS))

parsito_csharp.cpp: parsito_csharp.i ../common/parsito.i
	@$(call mkdir,Ufal/Parsito)
	swig $(SWIG_FLAGS) -csharp -namespace Ufal.Parsito -outdir $(call platform_name,Ufal/Parsito) -o $@ $<

.PHONY: clean
clean:
	@$(call rm,.build Ufal $(call all_dynlib,$(BINDING_MODULE)) parsito_csharp.cpp $(wildcard examples/*.class))
