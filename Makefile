# Master makefile for 'os'

SHELL = /bin/sh

srcdir = .
top_srcdir = .

SUBDIRS = tools perl kernel init . boot

include $(top_srcdir)/Rules.make

.PHONY: all-targets
all-targets: vmkernel
CLEAN=vmkernel vmkernel-g bochs.out debugger.out parport.out disk.img

vmkernel-g: init/start.o init/main.o init/version.o kernel/kernel.o perl/libmicroperl.a
	$(LD) -T vmkernel.lds -o $@ $^

vmkernel: vmkernel-g
	cp vmkernel-g vmkernel
	strip vmkernel
	@echo "Compiled kernel build #`cat $(top_srcdir)/.version`"

.PHONY: init
init::
	@echo "Initialising tree"
	echo '#define RELEASE "unknown"' >init/version.h
	echo '#define KERNEL_VERSION_CODE 0' >>init/version.h
	echo '#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))' >>init/version.h
	echo '#define BUILD "#-1"' >init/compile.h
	echo '#define VERSION ""' >>init/compile.h
	echo '#define BUILD_TIME ""' >>init/compile.h
	echo '#define BUILD_BY ""' >>init/compile.h
	echo '#define BUILD_HOST ""' >>init/compile.h
	echo '#define BUILD_DOMAIN ""' >>init/compile.h
	echo '#define BUILD_CC ""' >>init/compile.h
