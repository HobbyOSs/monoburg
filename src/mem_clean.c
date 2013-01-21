/**
 ** mem_clean.c: this file is part of MonoBURG.
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
 ** \brief Cleaning functions that free everything.
 */

#include "monoburg.h"
#include "rule.h"
#include "mem_clean.h"

static void free_tree (Tree *tree);
static void free_vartree (VarTree *tree);
static void free_prefix (void);
static void free_rules_prevent_refree (Rule *rule, GList *rule_list);

void free_rules (void)
{
        GList *rule;
        Rule *p_rule;
        for (rule = rule_list; rule; rule = rule->next ) {
                p_rule = (Rule *) rule->data;
                free_rules_prevent_refree (p_rule, rule->next);
                g_free (p_rule->cost);
                g_free (p_rule->cfunc);
                g_free (p_rule->code);
                free_tree (p_rule->tree);
                free_vartree (p_rule->vartree);
                g_free (p_rule);
        }
        g_list_free (rule_list);

        free_prefix ();
}

void free_terms (void)
{
        GList *term;
        Term *p_term;
        for (term = term_list; term; term = term->next) {
                p_term = (Term *) term->data;
                g_free (p_term->name);
                g_list_free (p_term->rules);
                g_free (p_term);
        }
        g_list_free (term_list);
        g_hash_table_destroy (term_hash);
}

void free_nonterms (void)
{
        GList *nonterm = nonterm_list;
        NonTerm *p_nonterm;
        for (nonterm = nonterm_list; nonterm; nonterm = nonterm->next) {
                p_nonterm = (NonTerm *) nonterm->data;
                g_free (p_nonterm->name);
                g_list_free (p_nonterm->chain);
                g_list_free (p_nonterm->rules);
                g_free (p_nonterm);
        }
        g_list_free (nonterm_list);
        g_hash_table_destroy (nonterm_hash);
}


static void free_tree (Tree *tree)
{
        if (!tree)
                return;
        free_tree (tree->left);
        free_tree (tree->right);
        g_free (tree);
}

static void free_vartree (VarTree *tree)
{
        GList *varnames;

        if (!tree)
                return;
        for (varnames = tree->varnames; varnames; varnames = varnames->next)
                g_free (varnames->data);
        g_list_free (tree->varnames);
        free_vartree (tree->left);
        free_vartree (tree->right);
        g_free (tree);
}

static void free_prefix (void)
{
        GList *prefix;
        for (prefix = prefix_list; prefix; prefix = prefix->next) {
                g_free (prefix->data);
        }
        g_list_free (prefix);
}

static void free_rules_prevent_refree (Rule *rule, GList *rule_list)
{
        char *cost = rule->cost;
        char *cfunc = rule->cfunc;
        char *code = rule->code;
        Tree *tree = rule->tree;

        for (; rule_list; rule_list = rule_list->next) {
                rule = (Rule *) rule_list->data;
                if (cost == rule->cost)
                        rule->cost = NULL;
                if (cfunc == rule->cfunc)
                        rule->cfunc = NULL;
                if (code == rule->code)
                        rule->code = NULL;
                if (tree == rule->tree)
                        rule->tree = NULL;
        }
}
