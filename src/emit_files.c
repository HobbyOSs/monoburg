/**
 ** emit_files.c: this file is part of MonoBURG.
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
 ** \brief Wrapper for C/Header file output.
 */

#include "monoburg.h"
#include "parser.h"
#include "mem_clean.h"
#include "emit.h"

/** Output header file data. */
void emit_header_file ()
{
        int n_namespace = 0;
        GList *namespace = namespaces;

        emit_includes ();
        emit_header ();
        while (namespace)
        {
          output ("namespace %s {\n", (char *) namespace->data);
          namespace = namespace->next;
          ++n_namespace;
        }
        emit_term ();
        emit_nonterm ();
        emit_state ();
        emit_prototypes ();
        while (n_namespace--)
          output ("}\n");
}

/** Output C file data. */
void emit_code_file ()
{
        int n_namespace = 0;
        GList *namespace = namespaces;

        for (namespace = namespaces; namespace; namespace = namespace->next) {
          output ("namespace %s {\n", (char *) namespace->data);
          g_free (namespace->data);
          ++n_namespace;
        }
        g_list_free (namespaces);
        emit_vardefs ();
        emit_cost_func ();
        emit_emitter_func ();
        emit_decoders ();

        emit_closure ();
        emit_label_func ();

        emit_kids ();
        while (n_namespace--)
          output ("}\n");
        free_rules ();
        free_terms ();
        free_nonterms ();
}
