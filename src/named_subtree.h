/**
 ** named_subtree.h: this file is part of MonoBURG.
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
 ** \brief Declaration of named subtree related functions.
 */

#ifndef __MONO_NAMED_SUBTREE_H__
# define __MONO_NAMED_SUBTREE_H__

# include "rule.h"

char    *compute_vartree_decs   (VarTree *t);
char    *compute_vartree_void_use (VarTree *t);
void     fill_vartree           (VarTree **p_vartree, Tree *tree);

#endif /* __MONO_NAMED_SUBTREE_H__ */
