## ----------------------------------------------------------------------------
## Configure MonoBURG, an IBurg-like code generator generator
##

## ------------------------------------- 
## Project
AC_INIT([MonoBURG], [1.0.2])
AM_CONFIG_HEADER([config.h])

## -------------------------------------
## Setup autotools

# Need autoconf 2.57 at least.
AC_PREREQ([2.57])

# Auxiliary files.
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([config/Makefile])

# Initiate automake.
AM_INIT_AUTOMAKE([1.7.5 dist-bzip2])

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
])

# Instantiate the output files
AC_OUTPUT
