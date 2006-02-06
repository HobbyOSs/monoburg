%{
/**
 ** parser.y: this file is part of MonoBURG.
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
 ** \brief Burg files' grammar description.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "monoburg.h"
#include "rule.h"
#include "parser.h"
#include "emit.h"

GList *inputs = NULL;
GHashTable *definedvars= NULL;
GList *namespaces = NULL;
GList *include_dirs = NULL;

%}

%union {
  char *text;
  int   ivalue;
  Tree  *tree;
  Rule  *rule;
  GList *rule_list;
}

%token <text> IDENT
%token <text> CODE
%token <text> STRING
%token  START
%token  COST
%token  TERM
%token  TERMPREFIX
%token  NAMESPACE
%token  NO_LINES
%token  NO_GLIB
%token  NO_EXPORTED_SYMBOLS
%token  CXX_REF
%token <ivalue> INTEGER

%type   <tree>          tree
%type   <text>          optcost
%type   <text>          optcfunc
%type   <text>          optcode
%type   <text>          optvarname
%type   <rule>          rule
%type   <rule_list>     rule_list

%%

decls   : /* empty */
	| START IDENT { start_nonterm ($2); } decls
	| TERM  tlist decls
	| TERMPREFIX plist decls
	| NAMESPACE IDENT {
			warn_cxx ("`%namespace' directive");
			namespaces = g_list_append (namespaces, $2);
		} decls
	| NO_LINES { lines_p = FALSE; } decls
	| NO_GLIB { glib_p = FALSE; } decls
	| NO_EXPORTED_SYMBOLS { exported_symbols_p = FALSE; } decls
	| CXX_REF {
			warn_cxx ("`%cxx-ref' directive");
			cxx_ref_p = TRUE;
			g_hash_table_insert (definedvars,
					     g_strdup ("__CXX_REF"),
					     GUINT_TO_POINTER (1));
		} decls
	| rule_list optcost optcode optcfunc {
			GList *tmp;
			for (tmp = $1; tmp; tmp = tmp->next) {
				rule_add (tmp->data, &($3), $2, $4);
			}
			g_list_free ($1);
		} decls
	;

rule	: IDENT ':' tree { $$ = make_rule ($1, $3); }
	;

rule_list : rule { $$ = rule_list_prepend (NULL, $1); }
	| rule ',' rule_list { $$ = rule_list_prepend ($3, $1); }
	;

optcode : /* empty */ { $$ = NULL; }
	| ';' { $$ = NULL; }
	| CODE
	;

plist	: /* empty */
	| plist IDENT { create_term_prefix ($2);}
	;

tlist	: /* empty */
	| tlist IDENT { create_term ($2, -1);}
	| tlist IDENT '=' INTEGER { create_term ($2, $4); }
	;

tree	: IDENT optvarname {
			$$ = create_tree ($2 ? $2 : $1, $2 ? $1 : NULL, NULL, NULL);
		}
	| IDENT optvarname '(' tree ')' {
			$$ = create_tree ($2 ? $2 : $1, $2 ? $1 : NULL, $4, NULL);
		}
	| IDENT optvarname '(' tree ',' tree ')' {
			$$ = create_tree ($2 ? $2 : $1, $2 ? $1 : NULL, $4, $6);
		}
	;

optcost : /* empty */ {$$ = NULL; }
	| STRING { $$ = g_strdup ($1); }
	| INTEGER { $$ = g_strdup_printf ("%d", $1); }
	;

optcfunc : /*empty */ { $$ = NULL; }
	 | COST CODE { $$ = $2; }
	 ;

optvarname: /* empty */ { $$ = NULL; }
	  | ':' IDENT { $$ = $2; }

%%

#define STATIC_STRLEN(Literal_Str)		\
  sizeof (Literal_Str) - 1

/* A call to strcmp() bounded by the length of LITERAL_STR.  */
#define BOUND_STRCMP(Str, Literal_Str)			\
  strncmp (Str, Literal_Str, STATIC_STRLEN (Literal_Str))

#define IS_TOKEN(Token, Str)			\
  !BOUND_STRCMP(Str, Token) &&			\
  isspace (*((Str) + STATIC_STRLEN (Token)))

#define EAT(Token, Str)				\
  Str += STATIC_STRLEN (Token)

#define LASTINPUT ((File *) g_list_last (inputs)->data)

/* We assume that there's no line larger than 2048 chars. */
static char input[2048];
static char *next = input;
static int state = 0;


char *
fgets_inc(char *s, int size)
{
  int n_length;
  char filename[MAX_FILENAME_LEN];
  int b_found = FALSE;
  char path[MAX_FILENAME_LEN * 2];
  File *new_include;
  GList *include_dir = include_dirs;

  assert (size > STATIC_STRLEN ("%include ") + MAX_FILENAME_LEN);

  if (fgets (s, size, LASTINPUT->fd) == NULL) {
    if (inputs->next == NULL)
      return 0;
    free (LASTINPUT->filename);
    fclose (LASTINPUT->fd);
    inputs = g_list_delete_link (inputs, g_list_last (inputs));
/* FIXME: At this stage, we can emit a #line even if the %no-lines directive
   might be read later in the grammar.  So, this part has been disabled until
   the scanner/parser is revamped to handle this case.  */
#if 0
    if (state != 1 && lines_p)
      output ("#line %d \"%s\"\n",
	      LASTINPUT->yylineno + 1,
	      LASTINPUT->filename);
#endif
    return fgets_inc(s, size);
  }

  LASTINPUT->yylineno++;
  LASTINPUT->yylinepos = 1;

  n_length = strlen(s);
  if (BOUND_STRCMP(s, "%include ") == 0) {
    if (n_length - STATIC_STRLEN ("%include ") > MAX_FILENAME_LEN)
      yyerror ("`%%include' is referring to a too long filename");
    if (s[n_length - 1] == '\n')
      s[n_length - 1] = 0;
    strcpy (filename, s + 9);
    new_include = g_new (File, 1);
    if (filename[0] == '/')
      b_found = (new_include->fd = fopen (filename, "r")) != 0;
    else
      while ((b_found == FALSE) && include_dir)
      {
	if (strcmp ((char *) include_dir->data, "") == 0)
	  sprintf (path, "%s", filename);
	else
	  sprintf (path, "%s/%s", (char *) include_dir->data, filename);
	if ((new_include->fd = fopen (path, "r")))
	  b_found = TRUE;
	include_dir = include_dir->next;
      }
    if (b_found == FALSE)
    {
      g_free (new_include);
      yyerror ("`%%include %s': %s",
	       filename, strerror(errno));
    }
    if (state != 1 && lines_p)
      output ("#line %d \"%s\"\n", 1, path);
    new_include->yylineno = 0;
    new_include->filename = g_strdup (path);
    inputs = g_list_append (inputs, new_include);
    return fgets_inc(s, size);
  }
  return s;
}

void
yyerror (char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  fprintf (stderr, "%s: line %d(%d): ",
	   LASTINPUT->filename,
	   LASTINPUT->yylineno, LASTINPUT->yylinepos);
  vfprintf (stderr, fmt, ap);
  fprintf(stderr, "\n");

  va_end (ap);

  exit (-1);
}

void
reset_parser (void)
{
  state = 0;
}

struct pplist {
  struct pplist *next;
  gboolean ignore;
};

static struct pplist *pp = NULL;

static char*
getvar (const char *input)
{
    char *var = g_strchug (g_strdup (input));
    char *ptr;

    for (ptr = var; *ptr && *ptr != '\n'; ++ptr) {
        if (g_ascii_isspace (*ptr)) {
            break;
        }
    }
    *ptr = '\0';

    return var;
}

static void
push_if (char *input, gboolean flip)
{
  struct pplist *new_pp = g_new (struct pplist, 1);
  char *var = getvar (input);

  new_pp->ignore = (g_hash_table_lookup (definedvars, var) == NULL) ^ flip;
  new_pp->next = pp;

  new_pp->ignore |= (pp ? pp->ignore : 0);
  pp = new_pp;
  g_free (var);
}

static void
flip_if (void)
{
  if (!pp)
      yyerror ("%%else without %%if");

  pp->ignore = !pp->ignore | (pp->next ? pp->next->ignore : 0);
}

static void
pop_if (void)
{
  struct pplist *prev_pp = pp;

  if (!pp)
      yyerror ("%%endif without %%if");

  pp = pp->next;
  g_free (prev_pp);
}

static char
nextchar (void)
{
  int next_state ;
  gboolean ll;

    if (!*next) {
      next = input;
      *next = 0;
      do {
	if (!fgets_inc (input, sizeof (input)))
	  return 0;

	ll = (input[0] == '%' && input[1] == '%');
	next_state = state;

        if (state == 1) {
          if (!ll && input[0] == '%') {
            if (!BOUND_STRCMP(&input[1], "ifdef")) {
              push_if (&input[STATIC_STRLEN ("ifdef") + 1], FALSE);
              ll = TRUE;
              continue;
            }
	    else if (!BOUND_STRCMP(&input[1], "ifndef")) {
              push_if (&input[STATIC_STRLEN ("ifndef") + 1], TRUE);
              ll = TRUE;
              continue;
            }
            else if (!BOUND_STRCMP(&input[1], "else")) {
              flip_if ();
              ll = TRUE;
              continue;
            }
            else if (!BOUND_STRCMP(&input[1], "endif")) {
              pop_if ();
              ll = TRUE;
              continue;
            }
          }
          if (pp && pp->ignore) {
            ll = TRUE;
            continue;
          }
        }

	switch (state) {
	case 0:
	  if (ll) {
	    next_state = 1;
	  } else
	    output ("%s", input);
	  break;
	case 1:
	  if (ll) {
	    next_state = 2;
	    *next = 0;
	    return 0;
	  }
	  break;
	default:
	  return 0;
	}
	ll = state != 1 || input[0] == '#';
	state = next_state;
      } while (ll);
    }

    return *next++;
}

void
yyparsetail (void)
{
  if (lines_p)
    output ("#line %d \"%s\"\n", LASTINPUT->yylineno,
	    LASTINPUT->filename);
  while (fgets_inc (input, sizeof (input)))
    output ("%s", input);
}

int
yylex (void)
{
  char c;

  do {

    if (!(c = nextchar ()))
      return 0;

    LASTINPUT->yylinepos = next - input;

    if (isspace (c))
      continue;

    if (c == '%') {
      if (IS_TOKEN ("start", next)) {
	EAT ("start", next);
	return START;
      }

      if (IS_TOKEN ("termprefix", next)) {
	EAT ("termprefix", next);
	return TERMPREFIX;
      }

      if (IS_TOKEN ("term", next)) {
	EAT ("term", next);
	return TERM;
      }

      if (IS_TOKEN ("namespace", next)) {
	EAT ("namespace", next);
	return NAMESPACE;
      }

      if (IS_TOKEN ("no-lines", next)) {
	EAT ("no-lines", next);
	return NO_LINES;
      }

      if (IS_TOKEN ("no-glib", next)) {
	EAT ("no-glib", next);
	return NO_GLIB;
      }

      if (IS_TOKEN ("no-exported-symbols", next)) {
	EAT ("no-exported-symbols", next);
	return NO_EXPORTED_SYMBOLS;
      }

      if (IS_TOKEN ("cxx-ref", next)) {
	EAT ("cxx-ref", next);
	return CXX_REF;
      }

      return c;
    }

    if (isdigit (c)) {
	    int num = 0;

	    do {
		    num = 10*num + (c - '0');
	    } while (isdigit (c = (*next++)));

	    yylval.ivalue = num;
	    return INTEGER;
    }

    if (isalpha (c)) {
      char *n = next;
      int l;

      if (IS_TOKEN ("cost", next - 1)) {
	EAT ("cost", next);
	return COST;
      }

      while (isalpha (*n) || isdigit (*n) || *n == '_')
	      n++;

      l = n - next + 1;
      yylval.text = g_strndup (next - 1, l);
      next = n;
      return IDENT;
    }

    if (c == '"') {
      int i = 0;
      static char buf [100000];

      while ((c = *next++) != '"' && c)
	buf [i++] = c;

      buf [i] = '\0';
      yylval.text = g_strdup (buf);

      return STRING;
    }

    if (c == '{') {
      unsigned i = 3, d = 1;
      static char buf [100000];

      g_memmove (buf, "\t{\n", 4);
      if (lines_p)
	i += sprintf (buf + 3, "#line %d \"%s\"\n", LASTINPUT->yylineno,
		      LASTINPUT->filename);
      while (d && (c = nextchar ())) {
	buf [i++] = c;
	assert (i < sizeof (buf));

	switch (c) {
	case '{': d++; break;
	case '}': d--; break;
	default:
		break;
	}
      }
      g_memmove (buf + --i, "\n\t}", 4);
      yylval.text = g_strdup (buf);

      return CODE;
    }

    return c;

  } while (1);
}

#undef EAT
#undef IS_TOKEN
#undef BOUND_STRCMP
#undef STATIC_STRLEN
