/**
 ** named_subtree.c: this file is part of MonoBURG.
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
 ** \brief Named subtree related functions.
 */

#include <string.h>
#include "monoburg.h"
#include "parser.h"
#include "named_subtree.h"

static char *compute_vartree_decs_ (char *prefix, int depth, VarTree *t);
static void fill_vartree_ (VarTree **p_vartree, Tree *tree,
                           VarTree *vartree_root);
static void check_varname (char *varname, VarTree *t);


/** Compute declarations of a variable tree. */
char *compute_vartree_decs (VarTree *t)
{
        return compute_vartree_decs_ ("", 0, t);
}

static char *compute_vartree_decs_ (char *prefix, int depth, VarTree *t)
{
        char *res = g_strdup ("");
        char *child;
        char *tmp_res;
        char *tn;
        int i;

        if (t->varnames) {
                GList *varnames = t->varnames;

                for (; varnames; varnames = varnames->next) {
                        if (cxx_ref_p)
                                tmp_res = "%s\tMBTREE_TYPE &%s = *(%s(&tree))";
                        else
                                tmp_res = "%s\tMBTREE_TYPE *%s = (%s(tree))";
                        tmp_res = g_strdup_printf (tmp_res, res,
                                                   (char *) varnames->data, prefix);
                        g_free (res);
                        res = tmp_res;
                        for (i = 0; i < depth; ++i) {
                                tmp_res = g_strconcat (res, ")", NULL);
                                g_free (res);
                                res = tmp_res;
                        }
                        tmp_res = g_strconcat (res, ";\n", NULL);
                        g_free (res);
                        res = tmp_res;
                }
        }

        if (t->left) {
                tn = g_strconcat ("MBTREE_LEFT(", prefix, NULL);
                child = compute_vartree_decs_ (tn, depth + 1, t->left);
                tmp_res = g_strconcat (res, child, NULL);
                g_free (child);
                g_free (res);
                res = tmp_res;
                g_free (tn);
        }
        if (t->right) {
                tn = g_strconcat ("MBTREE_RIGHT(", prefix, NULL);
                child = compute_vartree_decs_ (tn, depth + 1, t->right);
                tmp_res = g_strconcat (res, child, NULL);
                g_free (child);
                g_free (res);
                res = tmp_res;
                g_free (tn);
        }

        return res;
}

/** Compute void uses of variable in a tree. */
char *compute_vartree_void_use (VarTree *t)
{
        char *res = g_strdup ("");
        char *tmp_res;
        char *child;

        if (t->varnames) {
                GList *varnames = t->varnames;

                for (; varnames; varnames = varnames->next) {
                        tmp_res = g_strdup_printf ("%s%s(void) %s;",
                                                   *res ? res : "\t",
                                                   *res ? " " : "",
                                                   (char *) varnames->data);
                        g_free (res);
                        res = tmp_res;
                }
                tmp_res = g_strconcat (res, "\n", NULL);
                g_free (res);
                res = tmp_res;
        }

        if (t->left) {
                child = compute_vartree_void_use (t->left);
                tmp_res = g_strconcat (res, child, NULL);
                g_free (child);
                g_free (res);
                res = tmp_res;
        }

        if (t->right) {
                child = compute_vartree_void_use (t->right);
                tmp_res = g_strconcat (res, child, NULL);
                g_free (child);
                g_free (res);
                res = tmp_res;
        }

        return res;
}

/** Fill the tree of all vars in the local rule. */
void fill_vartree (VarTree **p_vartree, Tree *tree)
{
        fill_vartree_ (p_vartree, tree, *p_vartree);
}

static void fill_vartree_ (VarTree **p_vartree, Tree *tree,
                           VarTree *vartree_root)
{
        VarTree *vartree = *p_vartree;

        if (!vartree && (tree->varname || tree->left || tree->right))
        {
                *p_vartree = (vartree = g_new0 (VarTree, 1));
                vartree->left = NULL;
                vartree->right = NULL;
                vartree->varnames = NULL;
        }

        if (tree->varname) {
                if (vartree->varnames &&
                    g_list_find_custom (vartree->varnames, tree->varname,
                                        (GCompareFunc) &strcmp))
                        g_free (tree->varname);
                else {
                        if (vartree_root)
                                check_varname (tree->varname, vartree_root);
                        else
                                vartree_root = vartree;
                        vartree->varnames = g_list_prepend (vartree->varnames,
                                                            tree->varname);
                }
                tree->varname = NULL;
        }

        if (tree->left)
                fill_vartree_ (&(vartree->left), tree->left,
                               vartree_root);
        if (tree->right)
                fill_vartree_ (&(vartree->right), tree->right,
                               vartree_root);
}

static void check_varname (char *varname, VarTree *t)
{
        GList *varnames = t->varnames;

        if (varnames)
                for (; varnames; varnames = varnames->next)
                        if (!strcmp (varname, (char *) varnames->data))
                                yyerror ("variable name `%s' redefined incorrectly", varname);
        if (t->left)
                check_varname (varname, t->left);
        if (t->right)
                check_varname (varname, t->right);
}
