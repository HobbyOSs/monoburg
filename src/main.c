/**
 ** main.c: this file is part of MonoBURG.
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
 ** \brief Main function. Option parsing, and so on.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib/goption.h>
#include "monoburg.h"
#include "check.h"
#include "parser.h"
#include "emit.h"

static gboolean quiet = FALSE;
static char *header_define = NULL;
static char *cfile = NULL;
static char *deffile = NULL;
static GList *infiles = NULL;
gboolean with_glib = TRUE;
gboolean with_exported_symbols = TRUE;
gboolean with_references = FALSE;
gboolean dag_mode = 0;
gboolean predefined_terms = 0;

#define GOPTION_CALLBACK(Fun, Code)				\
	static gboolean Fun (const gchar *option_name,		\
			     const gchar *value,		\
			     gpointer data, GError **error)	\
	{							\
		(void) data; (void) error;			\
		(void) option_name; (void) value;		\
		{ Code; }					\
	}


GOPTION_CALLBACK (add_to_defined_vars,
		  g_hash_table_insert (definedvars, g_strdup (value),
				       GUINT_TO_POINTER (1));
		  return TRUE;);

GOPTION_CALLBACK (add_to_include_dirs,
		  include_dirs = g_list_append (include_dirs, g_strdup (value));
		  return TRUE);

static void bad_use (const char *program_name, const char *use)
{
	fprintf (stderr,
		 "%s: %s\n"
		 "Try `%s --help' for more information.\n",
		 program_name, use, program_name);
	exit (1);
}

void warn_cxx (const gchar *use_of)
{
	static int b_said = FALSE;

	if (!b_said && !quiet) {
		g_warning ("using %s will lead to produce C++ only code",
			   use_of);
		b_said = TRUE;
	}
}

static void warning_handler (const gchar *log_domain,
			     GLogLevelFlags log_level,
			     const gchar *message,
			     gpointer user_data)
{
	(void) log_domain; (void) log_level;
	(void) fprintf ((FILE *) user_data, "** WARNING **: %s\n", message);
}

static void parse_options (int argc, char **argv)
{
	static gboolean without_glib = FALSE;
	static gboolean without_exported_symbols = FALSE;
	static gboolean version = FALSE;
	int i;
	GError *error = NULL;
	GOptionContext* context = g_option_context_new ("- Generate a code generator. If no file is precised, "
							"the standard input will be processed.");

	static GOptionEntry option_entries[] = {
		{ "cost", 'c', 0, G_OPTION_ARG_INT, &default_cost,
		  "Set the default cost to VALUE.", "VALUE" },
		{ "header-file", 'd', 0, G_OPTION_ARG_FILENAME, &deffile,
		  "FILE will be the header file.", "FILE" },
		{ "define", 'D', 0, G_OPTION_ARG_CALLBACK, &add_to_defined_vars,
		  "SYM will be defined to 1.", "SYM" },
		{ "dag", 'e', 0, G_OPTION_ARG_NONE, &dag_mode,
		  "Enable DAG compatibility.", NULL },
		{ "include-dir", 'I', 0, G_OPTION_ARG_CALLBACK, &add_to_include_dirs,
		  "Add PATH to the list of directories to search for burg files.", "PATH" },
		{ "header-define", 'n', 0, G_OPTION_ARG_STRING, &header_define,
		  "Set DEF to be the guard define of the header file.", "DEF" },
		{ "predefined-terms", 'p', 0, G_OPTION_ARG_NONE, &predefined_terms,
		  "Terms are predefined.", NULL },
		{ "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet,
		  "Do not output warning messages.", NULL },
		{ "source-file", 's', 0, G_OPTION_ARG_FILENAME, &cfile,
		  "Set FILE to be the output source code file.", "FILE" },
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &version,
		  "Output version number and quit.", NULL },
		{ "without-glib", 0, 0, G_OPTION_ARG_NONE, &without_glib,
		  "Output a glib independent code.", NULL },
		{ "without-exported-symbols", 0, 0, G_OPTION_ARG_NONE, &without_exported_symbols,
		  "Avoid exported symbols as much as possible.", NULL },
		{ "with-references", 0, 0, G_OPTION_ARG_NONE, &with_references,
		  "Make emit functions to take references, not pointers.", NULL },
		{ NULL, 0, 0, 0, NULL, NULL, NULL }
	};

	g_option_context_add_main_entries (context, option_entries, NULL);
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free (context);
	if (version)
	{
		printf ("%s\n", PACKAGE_STRING);
		exit (0);
	}
	if (error)
		bad_use (argv[0], error->message);
	if ((cfile && !deffile) || (!cfile && deffile))
		bad_use (argv[0], "-s and -d must be precised together");
	with_glib = !without_glib;
	with_exported_symbols = !without_exported_symbols;
	if (with_references)
	{
		warn_cxx ("`--with-references' option");
		g_hash_table_insert (definedvars, g_strdup ("__WITH_REFERENCES"),
				     GUINT_TO_POINTER (1));
	}
	for (i = 1; i < argc; ++i)
		infiles = g_list_append (infiles, argv[i]);
}

int main (int argc, char **argv)
{
	File input;
	FILE *deffd = 0;
	FILE *cfd = 0;
	guint handler_id;
#ifndef NDEBUG
	GAllocator *list_allocator;
#endif

	/* GLib log handler. */
	handler_id = g_log_set_handler (NULL, G_LOG_LEVEL_WARNING, warning_handler, stderr);

#ifndef NDEBUG
	/* Install our allocator, to ease leak detection. */
	list_allocator = g_allocator_new ("List Allocator", 1024);
	g_list_push_allocator (list_allocator);
#endif

	/* Initialize vars. */
	definedvars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	include_dirs = g_list_append (include_dirs, ".");
	parse_options (argc, argv);

	/* Start header file. */
	if (deffile) {
		if (!(deffd = fopen (deffile, "w"))) {
			perror ("cant open header output file");
			exit (-1);
		}
		outputfd = deffd;
		output ("#ifndef %s\n",
			header_define ? header_define : "_MONO_BURG_DEFS_");
		output ("# define %s\n\n",
			header_define ? header_define : "_MONO_BURG_DEFS_");
	} else
		outputfd = stdout;

	/* Parse burg files. */
	if (infiles) {
		GList *l = infiles;
		inputs = g_list_append (inputs, &input);
		while (l) {
			char *infile = (char *)l->data;
			input.yylineno = 0;
			if (!(input.fd = fopen (infile, "r"))) {
				perror ("cant open input file");
				exit (-1);
			}
			input.filename = infile;

			output ("#line %d \"%s\"\n", 1, infile);
			yyparse ();

			reset_parser ();

			l->data = g_memdup (&input, sizeof (File));
			l = l->next;
		}
	} else {
		input.fd = stdin;
		input.filename = "stdin";
		input.yylineno = 0;
		inputs = g_list_append (inputs, &input);
		yyparse ();
	}

	/* Check the parsing. */
	if (!quiet)
		check_result ();
	if (!nonterm_list)
		g_error ("no start symbol found");


	/* End header file and write C file. */
	emit_header_file ();
	if (deffd) {
		output ("#endif /* %s */\n",
			header_define ? header_define : "_MONO_BURG_DEFS_");
		fclose (deffd);

		if (cfile == NULL)
			outputfd = stdout;
		else {
			if (!(cfd = fopen (cfile, "w"))) {
				perror ("cant open c output file");
				(void) remove (deffile);
				exit (-1);
			}

			outputfd = cfd;
		}

		output ("#include \"%s\"\n\n", deffile);
	}

	emit_code_file ();

	/* Parse end of burg files inserting them in the C file. */
	if (infiles) {
		GList *l = infiles;
		while (l) {
			input = *((File *) l->data);
			yyparsetail ();
			fclose (input.fd);
			g_free (l->data);
			l = l->next;
		}
	} else {
		yyparsetail ();
	}

	if (cfile)
		fclose (cfd);

	return 0;
}
