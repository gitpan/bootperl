# Makefile for `boot-trad'

SHELL = /bin/sh

srcdir = .
top_srcdir = ..

DEP_FILES = bootsect.d second.d
CLEAN = bootsect second image
include $(top_srcdir)/Rules.make
LD=ld -m elf_i386
LDFLAGS=-s

.PHONY: all-targets
all-targets: image
	@echo "Built boot-trad for build #`cat $(top_srcdir)/.version`"

bootsect: bootsect.o
	$(LD) -Ttext 0x0 -s --oformat binary -o $@ $<
bootsect.o: bootsect.s
bootsect.s: bootsect.S

second: second.o
	$(LD) -Ttext 0x0 -s --oformat binary -o $@ $<
second.o: second.s
second.s: second.S

image: bootsect second $(top_srcdir)/vmkernel
	$(top_srcdir)/tools/build bootsect second $(top_srcdir)/vmkernel >image
