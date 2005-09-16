## Makefile.am: this file is part of MonoBURG.		-*-Makefile-*-
##
## MonoBURG, an iburg like code generator generator.
##
## Copyright (C) 2001, 2002, 2004, 2005 Ximian, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## The complete GNU General Public Licence Notice can be found as the
## `NOTICE' file in the root directory.

## The test policy is to check if the output of monoburg differ from a
## `reference'. To add a test, just add `file.test' to `TESTS' and put
## `file.brg' and `file.c.ref' in the `tests' directory.


TESTS = sample.test

$(top_builddir)/src/monoburg$(EXEEXT):
	cd $(top_builddir)/src && $(MAKE) $(AM_MAKEFLAGS) monoburg$(EXEEXT)

## Uses GNU Make features.
%.c: %.brg $(top_builddir)/src/monoburg$(EXEEXT)
	$(top_builddir)/src/monoburg$(EXEEXT) $< > $@

%.test: %.c
	diff_tool=diff;					\
	test x$(VERBOSE) = x && diff_tool=cmp;		\
	if ! $$diff_tool $< $(srcdir)/$<.ref; then	\
		cp -f $< $<.fail;			\
		echo 'rm $$0; exit 1' > $@;		\
	else						\
		echo 'rm $$0; exit 0' > $@;		\
	fi
	chmod a+x $@

EXTRA_DIST = $(TESTS:.test=.brg) $(TESTS:.test=.c.ref)
CLEANFILES = $(TESTS:.test=.c) $(TESTS:.test=.c.fail)
