/**
 ** emit_functions.c: this file is part of MonoBURG.
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
 ** \brief Emit rules' functions.
 */

#include "monoburg.h"
#include "emit.h"

/** Emit rule functions. */
void emit_emitter_func ()
{
  GList *l;
  int i, rulen;
  GHashTable *cache = g_hash_table_new (g_str_hash, g_str_equal);

  for (l =  rule_list, i = 0; l; l = l->next) {
    Rule *rule = (Rule *)l->data;

    if (rule->code) {
      if ((rulen = GPOINTER_TO_INT (g_hash_table_lookup (cache, rule->code)))) {
        emit_rule_string (rule, "");
        output ("#define mono_burg_emit_%d mono_burg_emit_%d\n\n", i, rulen - 1);
        i++;
        continue;
      }
      output ("static void ");

      emit_rule_string (rule, "");

      if (dag_mode)
        output ("mono_burg_emit_%d (MBState *state, MBTREE_TYPE %ctree, MBCGEN_TYPE *s)\n", i,
                (cxx_ref_p ? '&' : '*'));
      else
        output ("mono_burg_emit_%d (MBTREE_TYPE %ctree, MBCGEN_TYPE *s)\n", i,
                (cxx_ref_p ? '&' : '*'));

      output ("{\n");
      output ("\t(void) tree; (void) s;");
      if (dag_mode)
        output (" (void) state;");
      output ("\n\t{\n%s\n\t}\n", rule->code);
      output ("}\n\n");
      g_hash_table_insert (cache, rule->code, GINT_TO_POINTER (i + 1));
    }
    i++;
  }

  g_hash_table_destroy (cache);

  if (!exported_symbols_p)
    output ("static ");
  output ("MBEmitFunc const mono_burg_func [] = {\n");
  output ("\tNULL,\n");
  for (l =  rule_list, i = 0; l; l = l->next) {
    Rule *rule = (Rule *)l->data;
    if (rule->code)
      output ("\tmono_burg_emit_%d,\n", i);
    else
      output ("\tNULL,\n");
    i++;
  }
  output ("};\n\n");
}
