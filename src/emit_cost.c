/**
 ** emit_cods.c: this file is part of MonoBURG.
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
 ** \brief Emic cost functions.
 */

#include "monoburg.h"
#include "parser.h"
#include "emit.h"


/** Emit cost functions. */
void emit_cost_func ()
{
	GList *l;
	int i;

	if (!with_exported_symbols)
		output ("static int mono_burg_rule (MBState *state, int goal);\n");

	for (l =  rule_list, i = 0; l; l = l->next) {
		Rule *rule = (Rule *)l->data;

		if (rule->cfunc) {
			output ("inline static guint16\n");

			emit_rule_string (rule, "");

			if (dag_mode)
				output ("mono_burg_cost_%d (MBState *state, MBCOST_DATA *data)\n", i + 1);
			else
				output ("mono_burg_cost_%d (MBTREE_TYPE %ctree, MBCOST_DATA *data)\n", i + 1,
					(with_references ? '&' : '*'));
			output ("{\n");
			output ("\t(void) data; ");
			if (dag_mode)
				output ("(void) state;\n");
			else
				output ("(void) tree;\n");
			output ("%s\n", rule->cfunc);
			output ("}\n\n");
		}
		i++;
	}
}

/** Emit closure functions. */
void emit_closure ()
{
	GList *l, *rl;

	for (l = nonterm_list; l; l = l->next) {
		NonTerm *n = (NonTerm *)l->data;

		if (n->chain)
			output ("static void closure_%s (MBState *p, int c);\n", n->name);
	}

	output ("\n");

	for (l = nonterm_list; l; l = l->next) {
		NonTerm *n = (NonTerm *)l->data;

		if (n->chain) {
			output ("static void\n");
			output ("closure_%s (MBState *p, int c)\n{\n", n->name);
			for (rl = n->chain; rl; rl = rl->next) {
				Rule *rule = (Rule *)rl->data;

				emit_rule_string (rule, "\t");
				emit_cond_assign (rule, rule->cost, "\t");
			}
			output ("}\n\n");
		}
	}
}


/** Emit the cost sum. */
void emit_costs (char *st, Tree *t)
{
	char *tn;

	if (t->op) {

		if (t->left) {
			tn = g_strconcat (st, "left->", NULL);
			emit_costs (tn, t->left);
			g_free (tn);
		}

		if (t->right) {
			tn = g_strconcat (st, "right->", NULL);
			emit_costs (tn, t->right);
			g_free (tn);
		}
	} else
		output ("%scost[MB_NTERM_%s] + ", st, t->nonterm->name);
}

/** Emit test on cost. */
void emit_cond_assign (Rule *rule, char *cost, char *fill)
{
	char *rc;

	if (cost)
		rc = g_strconcat ("c + ", cost, NULL);
	else
		rc = g_strdup ("c");


	output ("%sif (%s < p->cost[MB_NTERM_%s]) {\n", fill, rc, rule->lhs->name);

	output ("%s\tp->cost[MB_NTERM_%s] = %s;\n", fill, rule->lhs->name, rc);

	output ("%s\tp->rule_%s = %d;\n", fill, rule->lhs->name,
		g_list_index (rule->lhs->rules, rule) + 1);

	if (rule->lhs->chain)
		output ("%s\tclosure_%s (p, %s);\n", fill, rule->lhs->name, rc);

	output ("%s}\n", fill);

	g_free (rc);

}

