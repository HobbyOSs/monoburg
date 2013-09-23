## Makefile.am: this file is part of MonoBURG.		-*-Makefile-*-
##
## MonoBURG, an iburg like code generator generator.
##
## Copyright (C) 2001, 2002, 2004, 2005, 2012, 2013 Ximian, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## The complete GNU General Public Licence Notice can be found as the
## `NOTICE' file in the root directory.

SUBDIRS = src tests man
EXTRA_DIST = bootstrap NOTICE

## ------------------------ ##
## ChangeLog regeneration.  ##
## ------------------------ ##

.PHONY: update-ChangeLog
update-ChangeLog:
	$(AM_V_GEN)if test -d $(srcdir)/.git; then		\
	   $(srcdir)/build-aux/gitlog-to-changelog		\
	      --format='%s%n%n%b%n' --no-cluster		\
	      --strip-tab --strip-cherry-pick			\
	      -- $$(cat $(srcdir)/.last-cl-gen)..		\
	    >ChangeLog.tmp					\
	  && git rev-list -n 1 HEAD >.last-cl-gen.tmp		\
	  && (echo; cat $(srcdir)/ChangeLog) >>ChangeLog.tmp	\
	  && mv -f ChangeLog.tmp $(srcdir)/ChangeLog		\
	  && mv -f .last-cl-gen.tmp $(srcdir)/.last-cl-gen	\
	  && rm -f ChangeLog.tmp;				\
	fi

EXTRA_DIST += build-aux/gitlog-to-changelog .last-cl-gen
