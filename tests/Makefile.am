INCLUDES = $(GLIB_CFLAGS) -I$(srcdir)

check_PROGRAMS = sample

TESTS = sample

sample.c: sample.brg
	cd $(top_builddir)/src && $(MAKE) $(AM_MAKEFLAGS) monoburg
	$(top_builddir)/src/monoburg$(EXEEXT) $(srcdir)/sample.brg > sample.c

sample_LDADD = $(GLIB_LIBS)

EXTRA_DIST = sample.brg
CLEANFILES = sample.c sample.o parser.c
