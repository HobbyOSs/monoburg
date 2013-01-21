/**
 ** parser.h: this file is part of MonoBURG.
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
 ** \brief Parser's header file.
 */

#ifndef __MONO_PARSER_H__
# define __MONO_PARSER_H__

# include "monoburg.h"

# define MAX_FILENAME_LEN 100

typedef struct _File File;
struct _File {
        int yylineno;
        int yylinepos;
        char *filename;
        FILE *fd;
};

extern GList *inputs;

extern GList *namespaces;
extern GList *include_dirs;

extern GHashTable *definedvars;

void     yyerror (char *fmt, ...);
int      yylex   (void);
int      yyparse (void);
void     yyparsetail    (void);
void     reset_parser (void);

#endif /* __MONO_PARSER_H__ */
