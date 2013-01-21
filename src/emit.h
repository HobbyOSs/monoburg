/**
 ** emit.h: this file is part of MonoBURG.
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
 ** \brief Prototypes of all emit functions.
 */

#ifndef __MONO_EMIT_H__
# define __MONO_EMIT_H__

# include "rule.h"

extern FILE *outputfd;

void     output (char *fmt, ...);

void     emit_header_file ();
void     emit_code_file ();

void     emit_closure ();
void     emit_cost_func ();
void     emit_decoders ();
void     emit_emitter_func ();
void     emit_header ();
void     emit_includes ();
void     emit_kids ();
void     emit_label_func ();
void     emit_nonterm ();
void     emit_prototypes ();
void     emit_state ();
void     emit_term ();
void     emit_vardefs ();
void     emit_rule_string (Rule *rule, char *fill);
void     emit_costs (char *st, Tree *t);
void     emit_cond_assign (Rule *rule, char *cost, char *fill);
void     emit_tree_string (Tree *tree);

#endif /* __MONO_EMIT_H__ */
