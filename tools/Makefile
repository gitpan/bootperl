# Makefile for `tools'

SHELL = /bin/sh

srcdir = .
top_srcdir = ..

DEP_FILES = build.d
CLEAN = build
HOSTLINK.o := $(LINK.o)
include $(top_srcdir)/Rules.make

CPPFLAGS=
LINK.o := $(HOSTLINK.o)

.PHONY: all-targets
all-targets: build

build: build.o
build.o: build.c
