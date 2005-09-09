%{
/**
 ** parser.y: this file is part of MonoBURG.
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
 ** \brief Burg files' grammar description.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>

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
	| rule_list optcost optcode optcfunc {
			GList *tmp;
			for (tmp = $1; tmp; tmp = tmp->next) {
				rule_add (tmp->data, $3, $2, $4);
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
	| STRING
	| INTEGER { $$ = g_strdup_printf ("%d", $1); }
	;

optcfunc : /*empty */ { $$ = NULL; }
	 | COST CODE { $$ = $2; }
	 ;

optvarname: /* empty */ { $$ = NULL; }
	  | ':' IDENT { $$ = $2; }

%%

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

  /* 9 == strlen("%include "); */
  assert (size > 9 + MAX_FILENAME_LEN);

  if (fgets (s, size, LASTINPUT->fd) == NULL) {
    if (inputs->next == NULL)
      return 0;
    free (LASTINPUT->filename);
    fclose (LASTINPUT->fd);
    inputs = g_list_delete_link (inputs, g_list_last (inputs));
    if (state != 1 && with_line)
      output ("#line %d \"%s\"\n",
	      LASTINPUT->yylineno + 1,
	      LASTINPUT->filename);
    return fgets_inc(s, size);
  }

  LASTINPUT->yylineno++;
  LASTINPUT->yylinepos = 1;

  n_length = strlen(s);
  if (strncmp (s, "%include ", 9) == 0) {
    if (n_length - 9 > MAX_FILENAME_LEN)
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
    if (state != 1 && with_line)
      output ("#line %d \"%s\"\n", 1, path);
    new_include->yylineno = 0;
    new_include->filename = strdup (path);
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
reset_parser ()
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
flip_if ()
{
  if (!pp)
      yyerror ("%%else without %%if");

  pp->ignore = !pp->ignore | (pp->next ? pp->next->ignore : 0);
}

static void
pop_if ()
{
  struct pplist *prev_pp = pp;

  if (!pp)
      yyerror ("%%endif without %%if");

  pp = pp->next;
  g_free (prev_pp);
}

static char
nextchar ()
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
            if (!strncmp (&input[1], "ifdef", 5)) {
              push_if (&input[6], FALSE);
              ll = TRUE;
              continue;
            }
            else if (!strncmp (&input[1], "ifndef", 6)) {
              push_if (&input[7], TRUE);
              ll = TRUE;
              continue;
            }
            else if (!strncmp (&input[1], "else", 4)) {
              flip_if ();
              ll = TRUE;
              continue;
            }
            else if (!strncmp (&input[1], "endif", 5)) {
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
  if (with_line)
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
      if (!strncmp (next, "start", 5) && isspace (next[5])) {
	next += 5;
	return START;
      }

      if (!strncmp (next, "termprefix", 10) && isspace (next[10])) {
	next += 10;
	return TERMPREFIX;
      }

      if (!strncmp (next, "term", 4) && isspace (next[4])) {
	next += 4;
	return TERM;
      }

      if (!strncmp (next, "namespace", 9) && isspace (next[9])) {
	next += 9;
	return NAMESPACE;
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

      if (!strncmp (next - 1, "cost", 4) && isspace (next[3])) {
	next += 4;
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
      unsigned i = 0, d = 1;
      static char buf [100000];

      if (with_line)
	i = sprintf (buf, "#line %d \"%s\"\n", LASTINPUT->yylineno,
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
      buf [--i] = '\0';
      yylval.text = g_strdup (buf);

      return CODE;
    }

    return c;

  } while (1);
}
