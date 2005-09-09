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

monoburg_LDADD = $(GLIB_LIBS)
