# Makefile for `kernel'

SHELL = /bin/sh

srcdir = .
top_srcdir = ..

DEP_FILES = setup.d console.d printk.d vsprintf.d ctype.d panic.d entry.d \
  i8259.d irq.d clock.d delay.d memory.d keyboard.d task.d \
  telemetry.d bochs.d syscall.d exception.d processor.d \
  reboot.d perl.d perlwrap.d
include $(top_srcdir)/Rules.make

.PHONY: all-targets
all-targets: kernel.o

kernel.o: setup.o console.o printk.o vsprintf.o ctype.o panic.o entry.o \
  i8259.o irq.o clock.o delay.o memory.o keyboard.o task.o \
  telemetry.o bochs.o syscall.o exception.o processor.o \
  reboot.o perl.o perlwrap.o
	$(LINK.o) -r -o $@ $^

setup.o: setup.c
console.o: console.c
printk.o: printk.c
vsprintf.o: vsprintf.c
ctype.o: ctype.c
panic.o: panic.c
entry.o: entry.S
i8259.o: i8259.c
irq.o: irq.c
clock.o: clock.c
delay.o: delay.c
memory.o: memory.c
keyboard.o: keyboard.c
serial.o: serial.c
task.o: task.c
telemetry.o: telemetry.c
module.o: module.c
bochs.o: bochs.c
exception.o: exception.c
syscall.o: syscall.c
processor.o: processor.c
reboot.o: reboot.c
floppy.o: floppy.c

perl.o perl.d: CPPFLAGS += $(shell cat $(top_srcdir)/perl/ccopts)
perl.o: perl.c

perlwrap.o: perlwrap.c
