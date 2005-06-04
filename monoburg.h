#ifndef __MONO_MONOBURG_H__
#define __MONO_MONOBURG_H__

#include <glib.h>

#define MAX_FDS 10
#define MAX_FILENAME_LEN 100
#define MAX_NAMESPACES 10

void yyerror (char *fmt, ...);
int  yylex   (void);

extern FILE *outputfd;
extern GHashTable *definedvars;
extern char *namespaces[MAX_NAMESPACES];
extern int n_namespace;

typedef struct _Rule Rule;

typedef struct _Term Term;
struct _Term{
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
	NonTerm *nonterm; /* used by chain rules */
};

struct _Rule {
	NonTerm *lhs;
	Tree *tree;
	char *code;
	char *cost;
	char *cfunc;
};

typedef struct _File File;

struct _File {
	int yylineno;
	int yylinepos;
	char *filename;
	FILE *fd;
};

extern File inputs[MAX_FDS];


Tree    *create_tree    (char *id, Tree *left, Tree *right);

Term    *create_term    (char *id, int num);

void     create_term_prefix (char *id);

NonTerm *nonterm        (char *id);

void     start_nonterm  (char *id);

Rule    *make_rule      (char *id, Tree *tree);

void     rule_add       (Rule *rule, char *code, char *cost, char *cfunc);

void     create_rule    (char *id, Tree *tree, char *code, char *cost,
			 char *cfunc);

void     warn_cpp       (const gchar *use_of);

void     yyparsetail    (void);

void     reset_parser (void);

#endif
