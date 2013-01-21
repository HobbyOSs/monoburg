/**
 ** emit_pretty_print.c: this file is part of MonoBURG.
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
 ** \brief Emit pretty printed rules.
 */

#include "monoburg.h"
#include "emit.h"


/** Emit `fill' and then pretty print the rule in comment. */
void emit_rule_string (Rule *rule, char *fill)
{
        output ("%s/* ", fill);
        output ("%s: ", rule->lhs->name);
        emit_tree_string (rule->tree);
        output (" */\n");
}

/** Pretty print a rule tree. */
void emit_tree_string (Tree *tree)
{
        if (tree->op) {
                output ("%s", tree->op->name);
                if (tree->op->arity) {
                        output ("(");
                        emit_tree_string (tree->left);
                        if (tree->right) {
                                output (", ");
                                emit_tree_string (tree->right);
                        }
                        output (")");
                }
        } else
                output ("%s", tree->nonterm->name);
}
