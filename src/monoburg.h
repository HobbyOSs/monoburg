/**
 ** monoburg.h: this file is part of MonoBURG.
 **
 ** MonoBURG, an iburg like code generator generator.
 **
 ** Copyright (C) 2001, 2002, 2004, 2005 Ximian, Inc.
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU General Public License
 ** as published by the Free Software Foundation; either version 2
 ** of the License, or (at your option) any later version.
 **
 ** The complete GNU General Public Licence Notice can be found as the
 ** `NOTICE' file in the root directory.
 **
 ** \author Dietmar Maurer (dietmar@ximian.com)
 ** \author Cadilhac Michael (cadilhac@lrde.org)
 ** \brief Main's header file.
 */

#ifndef __MONO_MONOBURG_H__
# define __MONO_MONOBURG_H__

# include <stdio.h>
# include <config.h>
# include <glib.h>

extern gboolean lines_p;
extern gboolean glib_p;
extern gboolean exported_symbols_p;
extern gboolean cxx_ref_p;
extern int predefined_terms;
extern int dag_mode;

void     warn_cxx       (const gchar *use_of);

#endif /* __MONO_MONOBURG_H__ */
