/**
 ** rule.h: this file is part of MonoBURG.
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
 ** \brief Rule and so on defining.
 */

#ifndef __MONO_RULE_H__
# define __MONO_RULE_H__

# include <glib.h>

typedef struct _Term Term;
struct _Term {
	char *name;
	int number;
	int arity;
	GList *rules; /* rules that start with this terminal */
};

typedef struct _NonTerm NonTerm;
struct _NonTerm {
	char *name;
	int number;
	GList *rules; /* rules with this nonterm on the left side */
	GList *chain;
	gboolean reached;
};

typedef struct _Tree Tree;
struct _Tree {
	Term *op;
	Tree *left;
	Tree *right;
	char *varname;
	NonTerm *nonterm; /* used by chain rules */
};

typedef struct _Rule Rule;
struct _Rule {
	NonTerm *lhs;
	Tree *tree;
	char *code;
	char *cost;
	char *cfunc;
};

extern GList *term_list;
extern GList *nonterm_list;
extern GHashTable *term_hash;
extern GHashTable *nonterm_hash;
extern GList *nonterm_list;
extern GList *rule_list;
extern GList *prefix_list;
extern int default_cost;

Tree    *create_tree    (char *id, char *varname, Tree *left, Tree *right);
Term    *create_term    (char *id, int num);
void     create_term_prefix (char *id);
GList	*rule_list_prepend (GList *list, Rule *rule);
NonTerm *nonterm        (char *id);
void     start_nonterm  (char *id);
Rule    *make_rule      (char *id, Tree *tree);
void     rule_add       (Rule *rule, char *code, char *cost, char *cfunc);
void     create_rule    (char *id, Tree *tree, char *code, char *cost,
			 char *cfunc);

#endif /* __MONO_RULE_H__ */
