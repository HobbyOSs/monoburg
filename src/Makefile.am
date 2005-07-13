INCLUDES = $(GLIB_CFLAGS) -I$(srcdir)

bin_PROGRAMS = monoburg

monoburg_SOURCES =			\
	parser.y			\
	check.c check.h			\
	emit.c emit.h			\
	emit_cost.c emit_files.c	\
	emit_functions.c emit_kids.c	\
	emit_pretty_print.c main.c	\
	monoburg.h parser.h		\
	rule.c rule.h

monoburg_LDADD =			\
	$(GLIB_LIBS)

man_MANS = monoburg.1

EXTRA_DIST = $(man_MANS)
