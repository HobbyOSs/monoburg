## configure.ac: this file is part of MonoBURG.
##
## MonoBURG, an iburg like code generator generator.
##
## Copyright (C) 2001, 2002, 2004, 2005, 2012 Ximian, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## The complete GNU General Public Licence Notice can be found as the
## `NOTICE' file in the root directory.

## -------------------------------------
## Project
AC_INIT([MonoBURG], [1.0.6a])
AC_CONFIG_HEADERS([config.h])

## -------------------------------------
## Set up the Autotools

# Need autoconf 2.57 at least.
AC_PREREQ([2.57])

# Auxiliary files.
AC_CONFIG_AUX_DIR([build-aux])

# Initiate automake.
AM_INIT_AUTOMAKE([1.11 silent-rules dist-bzip2 check-news -Wall])
AM_SILENT_RULES([yes])

## -------------------------------------
## Development tools

# Look for a C compiler
AC_PROG_CC

# Look for a yacc-like program
AC_PROG_YACC

## -------------------------------------
## Environment

# Check for GLib 2.6 presence.
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.6], ,
                  [AC_MSG_ERROR([Glib 2.6 or newer is required])])

## -------------------------------------
## Epilogue

# Ask for Makefile creation
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
  man/Makefile
])

# Instantiate the output files
AC_OUTPUT
