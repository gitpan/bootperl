#PERLSRC=../../perl/perl-5.8.3
PERLSRC=../../perl/perl-5.9.1

CC=cc
STRIP=strip

.PHONY: all
all: microperl ccopts embed microperl.o kembed

# magical stuff for links
% :: $(PERLSRC)/%
	ln -sf $< $@
	@echo $@ >>.links

perly_files := $(wildcard $(PERLSRC)/perly.*)
perly.y:
	ln -sf $(perly_files) .

ifneq ($(wildcard $(PERLSRC)/reentr.inc),)
reentr.inc:
	ln -sf $(PERLSRC)/$@ $@
	@echo $@ >>.links
else
reentr.inc:
	touch reentr.inc
endif

# missing dependency
uconfig.h: perlio.h XSUB.h reentr.inc

include Makefile.micro

OPTIMIZE += -Os
DEFINES += -DMULTIPLICITY
O += wrap.o ctype.o
LIBS += -ldl
CFLAGS += -g -I../include

ccopts:
	echo $(DEFINES) >ccopts

microperl: wrap.o ctype.o

wrap.o: wrap.c
	$(CC) -c -o $@ $(CFLAGS) wrap.c

kwrap.o: kwrap.c ../kernel/perlwrap.c
	$(CC) -c -o $@ $(CFLAGS) kwrap.c

ctype.o: ctype.c
	$(CC) -c -o $@ $(CFLAGS) ctype.c

LIB_NO_OBJS = wrap.o uperlmain$(_O) ctype.o
LIB_OBJS = $(filter-out $(LIB_NO_OBJS),$(O))

libmicroperl.a: $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $?

embed: LDLIBS += -lm
embed: LDFLAGS += -static
embed: embed.o libmicroperl.a ctype.o

microperl.o: libmicroperl.a
	$(LINK.o) -r $^ -o $@

kembed: LDLIBS += -ldl
kembed: embed.o libmicroperl.a ctype.o kwrap.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

STRIP_FILES = microperl embed kembed
.PHONY: strip
strip:
	ls -l $(STRIP_FILES)
	$(STRIP) $(STRIP_FILES)
	ls -l $(STRIP_FILES)

.PHONY: clean distclean clean-extras maintainer-clean
clean: clean-extras
clean-extras:
	-xargs rm -f <.links
	-rm -f .links perly.* $(O) microperl ccopts *.o libmicroperl.a reentr.inc embed uconfig.h kembed *~

maintainer-clean: clean
