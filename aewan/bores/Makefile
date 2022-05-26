# Copyright (c) 2003 Bruno T. C. de Oliveira
# 
# LICENSE INFORMATION:
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
# Copyright (c) 2002 Bruno T. C. de Oliveira
# 
# INFORMAÇÕES DE LICENÇA:
# Este programa é um software de livre distribuição; você pode
# redistribuí-lo e/ou modificá-lo sob os termos da GNU General
# Public License, conforme publicado pela Free Software Foundation,
# pela versão 2 da licença ou qualquer versão posterior.
# 
# Este programa é distribuído na esperança de que ele será útil
# aos seus usuários, porém, SEM QUAISQUER GARANTIAS; sem sequer
# a garantia implícita de COMERCIABILIDADE ou DE ADEQUAÇÃO A
# QUALQUER FINALIDADE ESPECÍFICA. Consulte a GNU General Public
# License para obter mais detalhes (uma cópia acompanha este
# programa, armazenada no arquivo COPYING).

SOURCES=$(wildcard *.c)
HEADERS=$(filter-out bores.h,$(wildcard *.h))
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

CFLAGS+=-Wall

all: libbores.a bores.h

bores.h:
	@echo >bores.h
	@echo "/* ------ GENERATED FILE. DO NOT EDIT -------- */" >>bores.h
	@echo "/* THIS FILE IS OVERWRITTEN EVERY TIME YOU     */" >>bores.h
	@echo "/* BUILD THE BORES LIBRARY.                    */" >>bores.h
	@echo "/*                                             */" >>bores.h
	@echo "/* Copyright (c) 2003 Bruno T. C. de Oliveira. */" >>bores.h
	@echo "/* All rights reserved.                        */" >>bores.h
	for i in $(HEADERS); do echo "#include \"$$i\"" >>bores.h; done
	
libbores.a: $(OBJECTS)
	rm -f libbores.a
	ar -r libbores.a $(OBJECTS)

.depends: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $(SOURCES) >.depends
	
-include .depends

clean:
	rm -f *.o .depends libbores.a bores.h

.PHONY: clean all


