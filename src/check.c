/**
 ** check.c: this file is part of MonoBURG.
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
 ** \brief Check parse consistency.
 */

#include "monoburg.h"
#include "rule.h"

static void check_reach (NonTerm *n);
static void mark_reached (Tree *tree);

/** Check the consistency of the parse, by checking reachability of
    each non terminal symbols and arity of terminals. */
void check_result ()
{
	GList *l;

	for (l = term_list; l; l = l->next) {
		Term *term = (Term *)l->data;
		if (term->arity == -1)
			g_warning ("unused terminal \"%s\"",term->name);
	}

	check_reach (((NonTerm *)nonterm_list->data));

	for (l = nonterm_list; l; l = l->next) {
		NonTerm *n = (NonTerm *)l->data;
		if (!n->reached)
			g_warning ("unreachable nonterm \"%s\"", n->name);
	}
}

static void check_reach (NonTerm *n)
{
	GList *l;

	n->reached = 1;
	for (l = n->rules; l; l = l->next) {
		Rule *rule = (Rule *)l->data;
		mark_reached (rule->tree);
	}
}

static void mark_reached (Tree *tree)
{
	if (tree->nonterm && !tree->nonterm->reached)
		check_reach (tree->nonterm);
	if (tree->left)
		mark_reached (tree->left);
	if (tree->right)
		mark_reached (tree->right);
}
