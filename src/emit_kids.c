/**
 ** emit_kids.c: this file is part of MonoBURG.
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
 ** \brief Functions for kids emitting.
 */

#include <string.h>
#include "monoburg.h"
#include "emit.h"

static char *compute_kids (char *ts, Tree *tree, int *n);

/** Emit `mono_burg_rule' and `mono_burg_kids'. */
void emit_kids ()
{
	GList *l, *nl;
	int i, j, c, n, *si;
	char **sa;

	if (!with_exported_symbols)
		output ("static ");
	output ("int\n");
	output ("mono_burg_rule (MBState *state, int goal)\n{\n");

	output ("\tg_return_val_if_fail (state != NULL, 0);\n");
	output ("\tg_return_val_if_fail (goal > 0, 0);\n\n");

	output ("\tswitch (goal) {\n");

	for (nl = nonterm_list; nl; nl = nl->next) {
		NonTerm *n = (NonTerm *)nl->data;
		output ("\tcase MB_NTERM_%s:\n", n->name);
		output ("\t\treturn mono_burg_decode_%s [state->rule_%s];\n",
			n->name, n->name);
	}

	output ("\tdefault: g_assert_not_reached ();\n");
	output ("\t}\n");
	output ("\treturn 0;\n");
	output ("}\n\n");


	if (dag_mode) {
		if (!with_exported_symbols)
			output ("static ");
		output ("MBState **\n");
		output ("mono_burg_kids (MBState *state, int rulenr, MBState *kids [])\n{\n");
		output ("\tg_return_val_if_fail (state != NULL, NULL);\n");
		output ("\tg_return_val_if_fail (kids != NULL, NULL);\n\n");

	} else {
		if (!with_exported_symbols)
			output ("static ");
		output ("MBTREE_TYPE **\n");
		output ("mono_burg_kids (MBTREE_TYPE *tree, int rulenr, MBTREE_TYPE *kids [])\n{\n");
		output ("\tg_return_val_if_fail (tree != NULL, NULL);\n");
		output ("\tg_return_val_if_fail (kids != NULL, NULL);\n\n");
	}

	output ("\tswitch (rulenr) {\n");

	n = g_list_length (rule_list);
	sa = g_new0 (char *, n);
	si = g_new0 (int, n);

	/* compress the case statement */
	for (l = rule_list, i = 0, c = 0; l; l = l->next) {
		Rule *rule = (Rule *)l->data;
		int kn = 0;
		char *k;

		if (dag_mode)
			k = compute_kids ("state", rule->tree, &kn);
		else
			k = compute_kids ("tree", rule->tree, &kn);

		for (j = 0; j < c; j++)
			if (!strcmp (sa [j], k))
				break;

		si [i++] = j;
		if (j == c)
			sa [c++] = k;
	}

	for (i = 0; i < c; i++) {
		for (l = rule_list, j = 0; l; l = l->next, j++)
			if (i == si [j])
				output ("\tcase %d:\n", j + 1);
		output ("%s", sa [i]);
		output ("\t\tbreak;\n");
	}

	output ("\tdefault:\n\t\tg_assert_not_reached ();\n");
	output ("\t}\n");
	output ("\treturn kids;\n");
	output ("}\n\n");

}


/** Emit kids affectation. */
static char *compute_kids (char *ts, Tree *tree, int *n)
{
	char *res;

	if (tree->nonterm) {
		return g_strdup_printf ("\t\tkids[%d] = %s;\n", (*n)++, ts);
	} else if (tree->op && tree->op->arity) {
		char *res2 = NULL;

		if (dag_mode) {
			res = compute_kids (g_strdup_printf ("%s->left", ts),
					    tree->left, n);
			if (tree->op->arity == 2)
				res2 = compute_kids (g_strdup_printf ("%s->right", ts),
						     tree->right, n);
		} else {
			res = compute_kids (g_strdup_printf ("MBTREE_LEFT(%s)", ts),
					    tree->left, n);
			if (tree->op->arity == 2)
				res2 = compute_kids (g_strdup_printf ("MBTREE_RIGHT(%s)", ts),
						     tree->right, n);
		}

		return g_strconcat (res, res2, NULL);
	}
	return "";
}

