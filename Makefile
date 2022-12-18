#
# SPDX-License-Identifier: ISC
#
# Copyright (c) 2022 Michael Drake
#

PROJECT = nonogif
# For other build variants: `make VARIANT=debug` and `make VARIANT=sanitize`.
VARIANT = release

PREFIX ?= /usr/local

CC ?= gcc
MKDIR ?= mkdir -p
INSTALL ?= install -c
PKG_CONFIG ?= pkg-config

VERSION_FLAGS = -DVERSION_MAJOR=0 \
                -DVERSION_MINOR=1 \
                -DVERSION_PATCH=0

CPPFLAGS += -MMD -MP $(VERSION_FLAGS)
CFLAGS += -Isrc -std=c2x
CFLAGS += -Wall -Wextra -pedantic -Wconversion -Wwrite-strings -Wcast-align \
		-Wpointer-arith -Winit-self -Wshadow -Wstrict-prototypes \
		-Wmissing-prototypes -Wredundant-decls -Wundef -Wvla \
		-Wdeclaration-after-statement

ifeq ($(VARIANT), debug)
	CFLAGS += -O0 -g
else ifeq ($(VARIANT), sanitize)
	CFLAGS += -O0 -g -fsanitize=address -fsanitize=undefined -fno-sanitize-recover
	LDFLAGS += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover
else
	CFLAGS += -O2 -DNDEBUG
endif

PKG_DEPS := libcyaml cgif
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(PKG_DEPS))
LDFLAGS += $(shell $(PKG_CONFIG) --libs $(PKG_DEPS))

SRC := \
	src/cli.c \
	src/main.c \
	src/options.c

BUILDDIR := build/$(VARIANT)

all: $(BUILDDIR)/$(PROJECT)

OBJ := $(patsubst %.c,%.o, $(addprefix $(BUILDDIR)/,$(SRC)))
DEP := $(patsubst %.c,%.d, $(addprefix $(BUILDDIR)/,$(SRC)))

$(OBJ): $(BUILDDIR)/%.o : %.c
	$(Q)$(MKDIR) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/$(PROJECT): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILDDIR)

install: $(BUILDDIR)/$(PROJECT)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) $(BUILDDIR)/$(PROJECT) $(DESTDIR)$(PREFIX)/bin/$(PROJECT)

-include $(DEP)

.PHONY: all clean install
