/**
 ** mem_clean.h: this file is part of MonoBURG.
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
 ** \brief Memory freeing related functions.
 */

#ifndef __MONO_MEM_CLEAN_H__
# define __MONO_MEM_CLEAN_H__

void	 free_rules	(void);
void	 free_terms	(void);
void	 free_nonterms	(void);

#endif /* __MONO_MEM_CLEAN_H__ */
