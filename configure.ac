## configure.ac: this file is part of MonoBURG.
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

## -------------------------------------
## Project
AC_INIT([MonoBURG], [1.0.2a])
AM_CONFIG_HEADER([config.h])

## -------------------------------------
## Setup autotools

# Need autoconf 2.57 at least.
AC_PREREQ([2.57])

# Auxiliary files.
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([config/Makefile])

# Initiate automake.
AM_INIT_AUTOMAKE([1.7.5 dist-bzip2 check-news])

## -------------------------------------
## Development tools

# Look for a C compiler
AC_PROG_CC

# Look for a yacc-like program
AC_PROG_YACC

## -------------------------------------
## Environment

# Check for GLIB-2.0 presence
AM_PATH_GLIB_2_0

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
