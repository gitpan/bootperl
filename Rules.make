# -*- Makefile -*-

CC=ccache distcc cc
CPP=cpp

#.SUFFIXES:

CPPFLAGS += -D__KERNEL__ -I$(top_srcdir)/include -I$(srcdir)/include -I$(top_srcdir)/perl
CFLAGS+= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -fno-strict-aliasing
LINK.o = $(LD) $(LDFLAGS) $(TARGET_ARCH)

.PHONY: all
all:
ifdef SUBDIRS
	$(MAKE) $(AM_MAKEFLAGS) all-recursive
else
	$(MAKE) $(AM_MAKEFLAGS) all-targets
endif

ifdef DEP_FILES
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring depend,$(MAKECMDGOALS)))
-include $(DEP_FILES)
endif
endif
endif

OUTPUT_OPTION=-o $(*F).o
%.d: %.c
	$(COMPILE.c) -M $< >$(*F).D
	-cp $(*F).D  $(*F).d; \
	tr ' ' '\012' < $(*F).D \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> $(*F).d; \
	rm $(*F).D

%.d: %.S
	$(COMPILE.c) -D__ASSEMBLY__ -M $< >$(*F).D
	-cp $(*F).D  $(*F).d; \
	tr ' ' '\012' < $(*F).D \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> $(*F).d; \
	rm $(*F).D

%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

%.o: %.S
	$(COMPILE.c) $(OUTPUT_OPTION) -D__ASSEMBLY__ $<

## Recursive rules
.PHONY: all-recursive install-data-recursive install-exec-recursive \
        installdirs-recursive install-recursive uninstall-recursive  \
        check-recursive installcheck-recursive info-recursive dvi-recursive \
	depend-recursive
all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive \
depend-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target="$(patsubst %-recursive,%,$@)"; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-targets"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-targets" || exit 1; \
	fi; test -z "$$fail"

.PHONY: mostlyclean-recursive clean-recursive distclean-recursive \
        maintainer-clean-recursive
mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" = "." && dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target="$(patsubst %-recursive,%,$@)"; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-targets"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

.PHONY: tags-recursive
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

.PHONY: clean clean-targets
clean:
ifdef SUBDIRS
	$(MAKE) $(AM_MAKEFLAGS) clean-recursive
else
	$(MAKE) $(AM_MAKEFLAGS) clean-targets
endif
# Warning: do not use assember ending in .s as it gets cleaned!!!
clean-targets:
	rm -rf $(CLEAN) *.o *.s *.D *~

.PHONY: maintainer-clean maintainer-clean-targets
maintainer-clean:
ifdef SUBDIRS
	$(MAKE) $(AM_MAKEFLAGS) maintainer-clean-recursive
else
	$(MAKE) $(AM_MAKEFLAGS) maintainer-clean-targets
endif
# Warning: do not use assember ending in .s as it gets cleaned!!!
maintainer-clean-targets: clean-targets
	rm -rf $(DEP_FILES)

.PHONY: depend
depend:
ifdef SUBDIRS
	$(MAKE) $(AM_MAKEFLAGS) depend-recursive
else
	$(MAKE) $(AM_MAKEFLAGS) depend-targets
endif
ifdef DEP_FILES
depend-targets: $(DEP_FILES)
else
depend-targets:
endif

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
