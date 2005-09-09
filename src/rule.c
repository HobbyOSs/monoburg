/**
 ** rule.c: this file is part of MonoBURG.
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
 ** \brief Functions to produce parse's result.
 */

#include <string.h>
#include "monoburg.h"
#include "parser.h"
#include "rule.h"

GList *term_list = NULL;
GList *nonterm_list = NULL;
GHashTable *term_hash = NULL;
GHashTable *nonterm_hash = NULL;
GList *rule_list = NULL;
GList *prefix_list = NULL;
int default_cost = 0;

static NonTerm *nonterm (char *id);


Rule *make_rule (char *id, Tree *tree)
{
	Rule *rule = g_new0 (Rule, 1);
	rule->lhs = nonterm (id);
	rule->tree = tree;

	return rule;
}

void rule_add (Rule *rule, char *code, char *cost, char *cfunc)
{
	if (!cfunc && !cost)
		cost = g_strdup_printf ("%d", default_cost);

	rule_list = g_list_append (rule_list, rule);
	rule->cost = g_strdup (cost);
	rule->cfunc = g_strdup (cfunc);
	rule->code = g_strdup (code);

	if (cfunc) {
		if (cost)
			yyerror ("duplicated costs (constant costs and cost function)");
		else {
			if (dag_mode)
				rule->cost = g_strdup_printf ("mono_burg_cost_%d (p, data)",
							      g_list_length (rule_list));
			else
				rule->cost = g_strdup_printf ("mono_burg_cost_%d (tree, data)",
							      g_list_length (rule_list));
		}
	}

	rule->lhs->rules = g_list_append (rule->lhs->rules, rule);

	if (rule->tree->op)
		rule->tree->op->rules = g_list_append (rule->tree->op->rules, rule);
	else
		rule->tree->nonterm->chain = g_list_append (rule->tree->nonterm->chain, rule);
}

static void check_varname (char *varname, Tree *t)
{
	if (t->varname && !strcmp (varname, t->varname))
		yyerror ("variable name `%s' redefined", varname);
	if (t->left)
		check_varname (varname, t->left);
	if (t->right)
		check_varname (varname, t->right);
}

static void check_varnames (Tree *t_source, Tree *t_on)
{
	if (t_source->varname)
		check_varname (t_source->varname, t_on);
	if (t_source->left)
		check_varnames (t_source->left, t_on);
	if (t_source->right)
		check_varnames (t_source->right, t_on);
}

static void check_has_varname (Tree *t)
{
	if (t->varname)
		yyerror ("can't use named rules with multiple rules");
	if (t->left)
		check_has_varname (t->left);
	if (t->right)
		check_has_varname (t->right);
}

GList *rule_list_prepend (GList *list, Rule *rule)
{
	if (list && !list->next)
		check_has_varname (((Rule *) list->data)->tree);
	if (list)
		check_has_varname (rule->tree);
	return g_list_prepend (list, rule);
}

Tree *create_tree (char *id, char *varname, Tree *left, Tree *right)
{
	int arity = (left != NULL) + (right != NULL);
	Term *term = NULL;
	Tree *tree = g_new0 (Tree, 1);

	if (term_hash)
		term = g_hash_table_lookup (term_hash, id);

	/* try if id has termprefix */
	if (!term) {
		GList *pl;
		for (pl = prefix_list; pl; pl = pl->next) {
			char *pfx = (char *)pl->data;
			if (!strncmp (pfx, id, strlen (pfx))) {
				term = create_term (id, -1);
				break;
			}
		}

	}

	if (term) {
		if (term->arity == -1)
			term->arity = arity;

		if (term->arity != arity)
			yyerror ("changed arity of terminal %s from %d to %d",
				 id, term->arity, arity);

		tree->op = term;
		tree->left = left;
		tree->right = right;
	} else {
		tree->nonterm = nonterm (id);
	}

	if (varname) {
		if (tree->left)
			check_varname (varname, tree->left);
		if (tree->right)
			check_varname (varname, tree->right);
		tree->varname = g_strdup (varname);
	} else {
		tree->varname = NULL;
	}

	if (tree->left && tree->right)
		check_varnames (tree->left, tree->right);

	return tree;
}

static void check_term_num (char *key, Term *value, int num)
{
	if (num != -1 && value->number == num)
		yyerror ("duplicate terminal id \"%s\"", key);
}

void create_term_prefix (char *id)
{
	if (!predefined_terms)
		yyerror ("%termprefix is only available with -p option");

	prefix_list = g_list_prepend (prefix_list, g_strdup (id));
}

Term *create_term (char *id, int num)
{
	Term *term;

	if (!predefined_terms && nonterm_list)
		yyerror ("terminal definition after nonterminal definition");

	if (num < -1)
		yyerror ("invalid terminal number %d", num);

	if (!term_hash)
		term_hash = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_foreach (term_hash, (GHFunc) check_term_num, (gpointer) num);

	term = g_new0 (Term, 1);

	term->name = g_strdup (id);
	term->number = num;
	term->arity = -1;

	term_list = g_list_append (term_list, term);

	g_hash_table_insert (term_hash, term->name, term);

	return term;
}

static NonTerm *nonterm (char *id)
{
	NonTerm *nterm;

	if (!nonterm_hash)
		nonterm_hash = g_hash_table_new (g_str_hash, g_str_equal);

	if ((nterm = g_hash_table_lookup (nonterm_hash, id)))
		return nterm;

	nterm = g_new0 (NonTerm, 1);

	nterm->name = g_strdup (id);
	nonterm_list = g_list_append (nonterm_list, nterm);
	nterm->number = g_list_length (nonterm_list);

	g_hash_table_insert (nonterm_hash, nterm->name, nterm);

	return nterm;
}

void start_nonterm (char *id)
{
	static gboolean start_def;

	if (start_def)
		yyerror ("start symbol redeclared");

	start_def = TRUE;
	nonterm (id);
}
