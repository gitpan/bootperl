# Makefile for `tools'

SHELL = /bin/sh

srcdir = .
top_srcdir = ..

DEP_FILES = start.d main.d version.d
CLEAN = version.h compile.h
include $(top_srcdir)/Rules.make

.PHONY: all-targets
all-targets: start.o main.o version.o

start.o: start.S
main.o: main.c
version.o: version.c version.h compile.h

ifeq (,$(findstring depend,$(MAKECMDGOALS)))
include $(top_srcdir)/Version

version.h: $(top_srcdir)/Version
	echo \#define RELEASE \"$(KERNELRELEASE)\" > .ver
	echo \#define KERNEL_VERSION_CODE `expr $(VERSION) \\* 65536 + $(PATCHLEVEL) \\* 256 + $(SUBLEVEL)` >> .ver
	echo '#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))' >> .ver
	mv -f .ver $@

$(top_srcdir)/.version::
	if [ ! -f $@ ]; then \
		echo 1 > $@; \
	else \
		expr 0`cat $@` + 1 > $@; \
	fi

compile.h: $(top_srcdir)/Version $(top_srcdir)/.version
	echo \#define BUILD \"\#`cat $(top_srcdir)/.version`\" > .comp
	echo -n \#define VERSION \"\#`cat $(top_srcdir)/.version` >> .comp
	echo ' '`LANG=C date`'"' >> .comp
	echo \#define BUILD_TIME \"`LANG=C date +%T`\" >> .comp
	echo \#define BUILD_BY \"`whoami`\" >> .comp
	echo \#define BUILD_HOST \"`hostname`\" >> .comp
	echo \#define BUILD_DOMAIN \"`hostname -d`\" >> .comp
	echo \#define BUILD_CC \"`$(CC) $(CFLAGS) -v 2>&1 | tail -1`\" >> .comp
	mv -f .comp $@
else
version.h:
	touch version.h
compile.h:
	touch compile.h
endif
