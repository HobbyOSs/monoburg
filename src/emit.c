/**
 ** emit.c: this file is part of MonoBURG.
 **
 ** MonoBURG, an iburg like code generator generator.
 **
 ** Copyright (C) 2001, 2002, 2004, 2005, 2006 Ximian, Inc.
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
 ** \brief General emit functions that produce files' skeletons.
 */

#include <string.h>
#include "monoburg.h"
#include "emit.h"

FILE *outputfd;

static int term_compare_func (Term *t1, Term *t2);
static int next_term_num ();
static int term_compare_func (Term *t1, Term *t2);
static void emit_tree_match (char *st, Tree *t);
static void emit_rule_match (Rule *rule);
static char *compute_nonterms (Tree *tree);


/** Output some text /a la/ printf in the currently opened file. */
void output (char *fmt, ...)
{
  va_list ap;

  /* Replace all the tabulations by two spaces.  */
  char *cp, *cp2;
  va_start(ap, fmt);
  cp = g_strdup_vprintf(fmt, ap);
  va_end (ap);

  for (cp2 = cp; *cp2; ++cp2)
    if (*cp2 == '\t')
      {
        putc (' ', outputfd);
        putc (' ', outputfd);
      }
    else
      putc (*cp2, outputfd);
  g_free (cp);
}

/** Emit includes of external header files. */
void emit_includes ()
{
  if (glib_p) {
    output ("#include <glib.h>\n");
    output ("\n");
  }
  else {
    output ("#include <stdio.h>\n");
    output ("#include <stdlib.h>\n");
    output ("#include <stdarg.h>\n");
    output ("#include <assert.h>\n");
  }
}

/** Emit header defines, replace glib definitions if !glib_p. */
void emit_header ()
{
  if (!glib_p) {
    output ("#ifndef guint8\n"
            "# define guint8 unsigned char\n"
            "#endif /* !guint8 */\n"
            "#ifndef guint16\n"
            "# define guint16 unsigned short\n"
            "#endif /* !guint16 */\n"
            "#ifndef gpointer\n"
            "# define gpointer void*\n"
            "#endif /* !gpointer */\n"
            "\n");

    output ("#ifndef g_new\n"
            "static void *\n"
            "mono_burg_xmalloc_ (size_t size)\n"
            "{\n"
            "       void *p;\n"
            "\n"
            "       p = malloc (size);\n"
            "       if (!p) {\n"
            "               fprintf (stderr, \"Not enough memory\\n\");\n"
            "               exit (1);\n"
            "       }\n"
            "       return p;\n"
            "}\n"
            "# define g_new(struct_type, n_structs) ((struct_type *) mono_burg_xmalloc_ (sizeof(struct_type) * n_structs))\n"
            "#endif /* !g_new */\n"
            "\n");

    output ("#ifndef g_error\n"
            "static int\n"
            "mono_burg_error_ (const char *format, ...)\n"
            "{\n"
            "       int n = 0;\n"
            "       va_list ap;\n"
            "\n"
            "       n = fprintf (stderr, \"Error: \");\n"
            "       va_start (ap, format);\n"
            "       n += vfprintf (stderr, format, ap);\n"
            "       va_end (ap);\n"
            "\n"
            "       return n;\n"
            "}\n"
            "# define g_error mono_burg_error_\n"
            "#endif /* !g_error */\n");

    output ("#ifndef g_assert\n"
            "# define g_assert assert\n"
            "#endif /* !g_assert */\n"
            "\n");

    output ("#ifndef g_return_val_if_fail\n"
            "# ifdef NDEBUG\n"
            "#  define g_return_val_if_fail(expr, val)\n"
            "# else /* !NDEBUG */\n"
            "#  define g_return_val_if_fail(expr, val) do { if (! (expr)) return val; } while (0)\n"
            "# endif /* NDEBUG */\n"
            "#endif /* !g_return_val_if_fail */\n"
            "#ifndef g_assert_not_reached\n"
            "# define g_assert_not_reached(X) assert (!\"Should not be there\")\n"
            "#endif /* !g_assert_not_reached */\n"
            "\n");
  }

  output ("#ifndef MBTREE_TYPE\n#error MBTREE_TYPE undefined\n#endif\n"
          "#ifndef MBTREE_OP\n#define MBTREE_OP(t) ((t)->op)\n#endif\n"
          "#ifndef MBTREE_LEFT\n#define MBTREE_LEFT(t) ((t)->left)\n#endif\n"
          "#ifndef MBTREE_RIGHT\n#define MBTREE_RIGHT(t) ((t)->right)\n#endif\n"
          "#ifndef MBTREE_STATE\n#define MBTREE_STATE(t) ((t)->state)\n#endif\n"
          "#ifndef MBCGEN_TYPE\n#define MBCGEN_TYPE int\n#endif\n"
          "#ifndef MBALLOC_STATE\n#define MBALLOC_STATE g_new (MBState, 1)\n#endif\n"
          "#ifndef MBCOST_DATA\n#define MBCOST_DATA gpointer\n#endif\n"
          "\n"
          "#define MBMAXCOST 32768\n"
          "\n");

  output ("#define MBCOND(x) if (!(x)) return MBMAXCOST;\n"
          "\n");
}

/** Emit the enum for terminal members. */
void emit_term ()
{
  GList *l;

  for (l = term_list; l; l = l->next) {
    Term *t = (Term *)l->data;
    if (t->number == -1)
      t->number = next_term_num ();
  }
  term_list = g_list_sort (term_list, (GCompareFunc)term_compare_func);

  output ("typedef enum {\n");
  for (l = term_list; l; l = l->next) {
    Term *t = (Term *)l->data;
    if (t->number == -1)
      t->number = next_term_num ();

    if (predefined_terms)
      output ("\tMB_TERM_%s = %s%s\n", t->name, t->name,
              l->next ? "," : "");
    else
      output ("\tMB_TERM_%s = %d%s\n", t->name, t->number,
              l->next ? "," : "");
  }
  output ("} MBTerms;\n\n");
}

/** Get the next available term id. */
static int next_term_num ()
{
  GList *l = term_list;
  int i = 1;

  while (l) {
    Term *t = (Term *)l->data;
    if (t->number == i) {
      l = term_list;
      i++;
    } else
      l = l->next;
  }
  return i;
}

/** Function to compare terms. */
static int term_compare_func (Term *t1, Term *t2)
{
  return t1->number - t2->number;
}


/** Emit the enum for non-terminal members. */
void emit_nonterm ()
{
  GList *l;

  output ("typedef enum {\n");
  for (l = nonterm_list; l; l = l->next) {
    NonTerm *n = (NonTerm *)l->data;
    output ("\tMB_NTERM_%s = %d%s\n", n->name, n->number,
            (l->next) ? "," : "");
  }
  output ("} MBNTerms;\n\n");
  output ("#define MB_MAX_NTERMS\t%d\n", g_list_length (nonterm_list));
  output ("\n");
}

/** Emit the MBState struct. */
void emit_state ()
{
  GList *l;
  int i, j;

  output ("struct _MBState {\n");
  output ("\tint\t\top;\n");

  if (dag_mode) {
    output ("\tMBTREE_TYPE\t *tree;\n");
    output ("\tgint32 reg1, reg2;\n");
  }

  output ("\tstruct _MBState\t*left, *right;\n");
  output ("\tguint16\t\tcost[%d];\n", g_list_length (nonterm_list) + 1);

  for (l = nonterm_list; l; l = l->next) {
    NonTerm *n = (NonTerm *)l->data;
    g_assert (g_list_length (n->rules) < 256);
    i = g_list_length (n->rules);
    j = 1;
    while (i >>= 1)
      j++;
    output ("\tunsigned int\t rule_%s:%d;\n",  n->name, j);
  }
  output ("};\n\n");
  output ("typedef struct _MBState MBState;\n");
}

/** Emit rule decoders. */
void emit_decoders ()
{
  GList *l;
  GList *rl;

  for (l = nonterm_list; l; l = l->next) {
    NonTerm *n = (NonTerm *)l->data;
    output ("const int mono_burg_decode_%s[] = {\n", n->name);
    output ("\t0,\n");
    for (rl = n->rules; rl; rl = rl->next) {
      Rule *rule = (Rule *)rl->data;
      output ("\t%d,\n", g_list_index (rule_list, rule) + 1);
    }

    output ("};\n\n");
  }
}

/** Emit the condition to match a tree. */
static void emit_tree_match (char *st, Tree *t)
{
  char *tn;
  int not_first = strcmp (st, "p->");

  /* We can omit this check at the top level */
  if (not_first) {
    if (predefined_terms)
      output ("\t\t\t%sop == %s /* %s */", st, t->op->name, t->op->name);
    else
      output ("\t\t\t%sop == %d /* %s */", st, t->op->number, t->op->name);
  }

  if (t->left && t->left->op) {
    tn = g_strconcat (st, "left->", NULL);
    if (not_first)
      output (" &&\n");
    not_first = 1;
    emit_tree_match (tn, t->left);
    g_free (tn);
  }

  if (t->right && t->right->op) {
    tn = g_strconcat (st, "right->", NULL);
    if (not_first)
      output (" &&\n");
    emit_tree_match (tn, t->right);
    g_free (tn);
  }
}

/** Emit the whole rule match using emit_tree_match. */
static void emit_rule_match (Rule *rule)
{
        Tree *t = rule->tree;

        if ((t->left && t->left->op) ||
            (t->right && t->right->op)) {
                output ("\t\tif (\n");
                emit_tree_match ("p->", t);
                output ("\n\t\t)\n");
        }
}

/** Emit the tree matcher that put a tree together with its rule. */
void emit_label_func ()
{
  GList *l;
  int i;

  if (dag_mode) {
    output ("static void\n");
    output ("mono_burg_label_priv (MBTREE_TYPE *tree, MBCOST_DATA *data, MBState *p) {\n");
  } else {
    output ("static MBState *\n");
    output ("mono_burg_label_priv (MBTREE_TYPE *tree, MBCOST_DATA *data) {\n");
  }

  output ("\tint arity;\n");
  output ("\tint c;\n");
  if (!dag_mode)
    output ("\tMBState *p;\n");
  output ("\tMBState *left = NULL, *right = NULL;\n\n");

  output ("\tswitch (mono_burg_arity [MBTREE_OP(tree)]) {\n");
  output ("\tcase 0:\n");
  output ("\t\tbreak;\n");
  output ("\tcase 1:\n");
  if (dag_mode) {
    output ("\t\tleft = MBALLOC_STATE;\n");
    output ("\t\tmono_burg_label_priv (MBTREE_LEFT(tree), data, left);\n");
  } else {
    output ("\t\tleft = mono_burg_label_priv (MBTREE_LEFT(tree), data);\n");
    output ("\t\tright = NULL;\n");
  }
  output ("\t\tbreak;\n");
  output ("\tcase 2:\n");
  if (dag_mode) {
    output ("\t\tleft = MBALLOC_STATE;\n");
    output ("\t\tmono_burg_label_priv (MBTREE_LEFT(tree), data, left);\n");
    output ("\t\tright = MBALLOC_STATE;\n");
    output ("\t\tmono_burg_label_priv (MBTREE_RIGHT(tree), data, right);\n");
  } else {
    output ("\t\tleft = mono_burg_label_priv (MBTREE_LEFT(tree), data);\n");
    output ("\t\tright = mono_burg_label_priv (MBTREE_RIGHT(tree), data);\n");
  }
  output ("\t}\n\n");

  output ("\tarity = (left != NULL) + (right != NULL);\n");
  output ("\tg_assert (arity == mono_burg_arity [MBTREE_OP(tree)]);\n\n");

  if (!dag_mode)
    output ("\tp = MBALLOC_STATE;\n");

  output ("\tmemset (p, 0, sizeof (MBState));\n");
  output ("\tp->op = MBTREE_OP(tree);\n");
  output ("\tp->left = left;\n");
  output ("\tp->right = right;\n");

  if (dag_mode)
    output ("\tp->tree = tree;\n");

  for (l = nonterm_list, i = 0; l; l = l->next) {
    output ("\tp->cost [%d] = 32767;\n", ++i);
  }
  output ("\n");

  output ("\tswitch (MBTREE_OP(tree)) {\n");
  for (l = term_list; l; l = l->next) {
    Term *t = (Term *)l->data;
    GList *rl;

    if (predefined_terms)
      output ("\tcase %s: /* %s */\n", t->name, t->name);
    else
      output ("\tcase %d: /* %s */\n", t->number, t->name);

    for (rl = t->rules; rl; rl = rl->next) {
      Rule *rule = (Rule *)rl->data;
      Tree *t = rule->tree;

      emit_rule_string (rule, "\t\t");

      emit_rule_match (rule);

      output ("\t\t{\n");

      output ("\t\t\tc = ");

      emit_costs ("", t);

      output ("%s;\n", rule->cost);

      emit_cond_assign (rule, NULL, "\t\t\t");

      output ("\t\t}\n");
    }

    output ("\t\tbreak;\n");
  }

  output ("\tdefault:\n");
  output ("#ifdef MBGET_OP_NAME\n");
  output ("\t\tg_error (\"unknown operator: %%s\", MBGET_OP_NAME(MBTREE_OP(tree)));\n");
  output ("#else\n");
  output ("\t\tg_error (\"unknown operator: 0x%%04x\", MBTREE_OP(tree));\n");
  output ("#endif\n");
  output ("\t}\n\n");

  if (!dag_mode) {
    output ("\tMBTREE_STATE(tree) = p;\n");
    output ("\treturn p;\n");
  }

  output ("}\n\n");

  if (!exported_symbols_p)
    output ("static ");
  output ("MBState *\n");
  output ("mono_burg_label (MBTREE_TYPE *tree, MBCOST_DATA *data)\n{\n");
  if (dag_mode) {
    output ("\tMBState *p = MBALLOC_STATE;\n");
    output ("\tmono_burg_label_priv (tree, data, p);\n");
  } else {
    output ("\tMBState *p = mono_burg_label_priv (tree, data);\n");
  }
  output ("\treturn p->rule_%s ? p : NULL;\n", ((NonTerm *)nonterm_list->data)->name);
  output ("}\n\n");
}

/** Compute list of non terminals of a rule tree. */
static char *compute_nonterms (Tree *tree)
{
  if (!tree)
    return g_strdup ("");

  if (tree->nonterm) {
    return g_strdup_printf ("MB_NTERM_%s, ", tree->nonterm->name);
  } else {
    char *s_left = compute_nonterms (tree->left);
    char *s_right = compute_nonterms (tree->right);
    char *s_result = g_strconcat (s_left, s_right, NULL);
    g_free (s_left);
    g_free (s_right);
    return s_result;
  }
}

/** Emit arity, term_string, rule_string arrays. */
void emit_vardefs ()
{
  GList *l;
  int i, j, c, n, *si;
  char **sa;

  if (predefined_terms) {
    output ("guint8 mono_burg_arity [MBMAX_OPCODES];\n");
    if (!exported_symbols_p)
      output ("static ");
    output ("void\nmono_burg_init (void)\n{\n");

    for (l = term_list, i = 0; l; l = l->next) {
      Term *t = (Term *)l->data;

      output ("\tmono_burg_arity [%s] = %d; /* %s */\n", t->name, t->arity, t->name);

    }

    output ("}\n\n");

  } else {
    output ("const guint8 mono_burg_arity [] = {\n");
    for (l = term_list, i = 0; l; l = l->next) {
      Term *t = (Term *)l->data;

      while (i < t->number) {
        output ("\t0,\n");
        i++;
      }

      output ("\t%d, /* %s */\n", t->arity, t->name);

      i++;
    }
    output ("};\n\n");

    if (exported_symbols_p) {
      output ("const char *const mono_burg_term_string [] = {\n");
      output ("\tNULL,\n");
      for (l = term_list, i = 0; l; l = l->next) {
        Term *t = (Term *)l->data;
        output ("\t\"%s\",\n", t->name);
      }
      output ("};\n\n");
    }
  }

  if (!exported_symbols_p)
    output ("static ");
  output ("const char * const mono_burg_rule_string [] = {\n");
  output ("\tNULL,\n");
  for (l = rule_list, i = 0; l; l = l->next) {
    Rule *rule = (Rule *)l->data;
    output ("\t\"%s: ", rule->lhs->name);
    emit_tree_string (rule->tree);
    output ("\",\n");
  }
  output ("};\n\n");

  n = g_list_length (rule_list);
  sa = g_new0 (char *, n);
  si = g_new0 (int, n);

  /* compress the _nts array */
  for (l = rule_list, i = 0, c = 0; l; l = l->next) {
    Rule *rule = (Rule *)l->data;
    char *s = compute_nonterms (rule->tree);

    for (j = 0; j < c; j++)
      if (!strcmp (sa [j], s))
        break;

    si [i++] = j;
    if (j == c) {
      output ("static const guint16 mono_burg_nts_%d [] = { %s0 };\n", c, s);
      sa [c++] = s;
    } else {
      g_free (s);
    }
  }
  output ("\n");

  if (!exported_symbols_p)
    output ("static ");
  output ("const guint16 *const mono_burg_nts [] = {\n");
  output ("\t0,\n");
  for (l = rule_list, i = 0; l; l = l->next) {
    Rule *rule = (Rule *)l->data;
    output ("\tmono_burg_nts_%d, ", si [i++]);
    emit_rule_string (rule, "");
  }
  while (c--)
    g_free (sa [c]);
  g_free (sa);
  g_free (si);

  output ("};\n\n");
}

/** Emit prototypes of all functions. */
void emit_prototypes ()
{
  if (dag_mode)
    output ("typedef void (*MBEmitFunc) (MBState *state, MBTREE_TYPE %ctree, MBCGEN_TYPE *s);\n\n",
            (cxx_ref_p ? '&' : '*'));
  else
    output ("typedef void (*MBEmitFunc) (MBTREE_TYPE %ctree, MBCGEN_TYPE *s);\n\n",
            (cxx_ref_p ? '&' : '*'));

  if (exported_symbols_p) {
    output ("extern const char * const mono_burg_term_string [];\n");
    output ("extern const char * const mono_burg_rule_string [];\n");
    output ("extern const guint16 *const mono_burg_nts [];\n");
    output ("extern MBEmitFunc const mono_burg_func [];\n");

    output ("MBState *mono_burg_label (MBTREE_TYPE *tree, MBCOST_DATA *data);\n");
    output ("int mono_burg_rule (MBState *state, int goal);\n");

    if (dag_mode)
      output ("MBState **mono_burg_kids (MBState *state, int rulenr, MBState *kids []);\n");
    else
      output ("MBTREE_TYPE **mono_burg_kids (MBTREE_TYPE *tree, int rulenr, MBTREE_TYPE *kids []);\n");
    if (predefined_terms) {
      output ("extern void mono_burg_init (void);\n");
    }
  }
  output ("\n");
}
