## Makefile.am -- Process this file with automake to produce Makefile.in
##

SUBDIRS = config

## Include path
INCLUDES = $(GLIB_CFLAGS) -I$(srcdir)

## MonoBURG binary
bin_PROGRAMS = monoburg

monoburg_SOURCES =			\
	parser.y			\
	monoburg.c monoburg.h
monoburg_LDADD =			\
	$(GLIB_LIBS)

## MonoBURG manual
man_MANS = monoburg.1

EXTRA_DIST = $(man_MANS)

## Check/Sample
check_PROGRAMS = sample

TESTS = sample

sample.c: monoburg$(EXEEXT) $(srcdir)/sample.brg
	./monoburg$(EXEEXT) $(srcdir)/sample.brg > sample.c

sample_SOURCES = sample.brg
sample_LDADD = sample.o $(GLIB_LIBS)

CLEANFILES = sample.c
