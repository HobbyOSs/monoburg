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
#include "named_subtree.h"
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
        rule->vartree = NULL;
        rule->code = NULL;

        return rule;
}

void rule_add (Rule *rule, char **p_code, char *cost, char *cfunc)
{
        rule_list = g_list_append (rule_list, rule);

        rule->cfunc = cfunc;

        if (rule->vartree) {
                char *varstr;

                varstr = compute_vartree_decs (rule->vartree);
                if (*varstr) {
                        char *varstr_void_use = compute_vartree_void_use (rule->vartree);

                        rule->code = g_strconcat (varstr, varstr_void_use, *p_code, NULL);
                        g_free (*p_code);
                        g_free (varstr);
                        g_free (varstr_void_use);
                        *p_code = rule->code;
                } else {
                        g_free (varstr);
                        rule->code = *p_code;
                }
        } else
                rule->code = *p_code;

        if (cfunc) {
                if (cost)
                        yyerror ("duplicated costs (constant costs and cost function)");
                if (dag_mode)
                        rule->cost = g_strdup_printf ("mono_burg_cost_%d (p, data)",
                                                      g_list_length (rule_list));
                else
                        rule->cost = g_strdup_printf ("mono_burg_cost_%d (tree, data)",
                                                      g_list_length (rule_list));
        } else if (cost) {
                rule->cost = cost;
        } else {
                rule->cost = g_strdup_printf ("%d", default_cost);
        }
        rule->lhs->rules = g_list_append (rule->lhs->rules, rule);

        if (rule->tree->op)
                rule->tree->op->rules = g_list_append (rule->tree->op->rules, rule);
        else
                rule->tree->nonterm->chain = g_list_append (rule->tree->nonterm->chain, rule);
}

GList *rule_list_prepend (GList *list, Rule *rule)
{
        if (list)
        {
                Rule *first_rule = (Rule *) list->data;

                fill_vartree (&(first_rule->vartree), rule->tree);
                rule->vartree = first_rule->vartree;
                first_rule->vartree = NULL;
        }
        else
                fill_vartree (&rule->vartree, rule->tree);

        return g_list_prepend (list, rule);
}

Tree *create_tree (char *id, char *varname, Tree *left, Tree *right)
{
        int arity = (left != NULL) + (right != NULL);
        Term *term = NULL;
        Tree *tree = g_new0 (Tree, 1);

        if (term_hash)
                if ((term = g_hash_table_lookup (term_hash, id)))
                        g_free (id);

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
                tree->varname = varname;
        } else {
                tree->varname = NULL;
        }

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

        prefix_list = g_list_prepend (prefix_list, id);
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

        term->name = id;
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
        {
                g_free (id);
                return nterm;
        }

        nterm = g_new0 (NonTerm, 1);

        nterm->name = id;
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
