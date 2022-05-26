/*
Copyright (c) 2003 Bruno T. C. de Oliveira

LICENSE INFORMATION:
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
Copyright (c) 2002 Bruno T. C. de Oliveira

INFORMAÇÕES DE LICENÇA:
Este programa é um software de livre distribuição; você pode
redistribuí-lo e/ou modificá-lo sob os termos da GNU General
Public License, conforme publicado pela Free Software Foundation,
pela versão 2 da licença ou qualquer versão posterior.

Este programa é distribuído na esperança de que ele será útil
aos seus usuários, porém, SEM QUAISQUER GARANTIAS; sem sequer
a garantia implícita de COMERCIABILIDADE ou DE ADEQUAÇÃO A
QUALQUER FINALIDADE ESPECÍFICA. Consulte a GNU General Public
License para obter mais detalhes (uma cópia acompanha este
programa, armazenada no arquivo COPYING).
*/

/*
AEWAN FILE FORMAT: 

Any line can contain as many initial spaces as it wants, but 
TRAILING SPACES may be significant in string values and other places.

A header whose name is BLAH is written as a line containing exactly:
	
	<BLAH

A footer whose name is BLAH is written as a line containing exactly:

	>BLAH

A labeled integer (label blah, value 42) will be a line of the form:

        blah: int: 42

A labeled string (label blah, value "Line 1\nLine 2\nLine 3") will be:

        blah: string: Line 1\:Line 2\:Line 3

Notice that newlines are escaped as '\:'. The rule is that any character
in the range 1 to 31, inclusive, is escaped as '\' followed by ('0' + ch).
A backslash is encoded as '\\'

Notice that all data lines have three fields, delimited by ": " (the
space is significant and is necessary).
*/

#include "aeff.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <zlib.h>
#include "bores/bores.h"

char *err_string = 0;  /* error buffer */

struct AeFile_ {
   gzFile f;
   int indent_level; /* current indent level, for pretty formatting.
                      * Purely cosmetic. */
};

static bool read_parse_data_line(gzFile f, char **label, char **type, 
							 char **value) {
   /* reads and splits a data line, also converting the escape sequences
    * found in the third field. If parse fails, set err_string and
    * returns false. On success, returns true. */
   char *field_start[3];
   char *line;

   autod_begin;
   autod_register(line, free);

   autod_assign( line, freadline_ex(f, gzgetc) );

   /* look for ':' field separators and note their locations */
   field_start[0] = line;
   if ( ! (field_start[1] = strchr(line, ':')) ||
        ! (field_start[2] = strchr(field_start[1] + 1, ':')) ) {

      dstrset(&err_string, "There's a data line with <3 fields.");
      autod_return_value(false);
   }

   /* split string by putting 0's on the field limit locations,
    * and advacing the limits by 2 positions (to skip over the
    * 0 we just put in and the space that immediately succeeds it) */
   *(field_start[1]) = 0; field_start[1] += 2;
   *(field_start[2]) = 0; field_start[2] += 2;
   
   /* discard leading spaces from label that originated from the
    * indentation */
   while (*(field_start[0]) == ' ' || 
          *(field_start[0]) == '\t') field_start[0]++;

   /* if we ended up with an empty field, we have bad syntax */
   if (field_start[0] >= field_start[1]) {
      dstrset(&err_string, "There's a data line with an empty label");
      autod_return_value(false);
   }

   /* now copy the needed portions to the return variables */
   if (label) *label = sstrdup(field_start[0]);
   if (type)  *type  = sstrdup(field_start[1]);
   if (value) *value = sstrdup(field_start[2]);

   /* and decode escape sequences found in value */
   if (value) {
      const char *r = *value;
      char *w = *value;

      while (*r) {
         if (*r == '\\')
	    r++, *w = (*r == '\\') ? '\\' : *r - '0';
	 else 
	    *w = *r;

         r++, w++;
      }

      *w = 0;
   }

   /* and call it a day */
   autod_return_value(true);
}

static void aeff_indent(AeFile *f) {
   int n = f->indent_level;
   while (n-- > 0) gzputc(f->f, '\t');
}

AeFile* aeff_open(const char *filename, char mode) {
   AeFile *f = zalloc(sizeof(AeFile));

   f->f = gzopen(filename, mode == 'r' ? "rb" : "wb");
   if (!f->f) {
      free(f);
      zfree(&err_string);
      err_string = dsprintf("%s: %s", filename, strerror(errno));
      return NULL;
   }

   f->indent_level = 0;

   return f;
}

const char *aeff_get_error(void) {
   return err_string;
}

void aeff_set_error(const char *s) {
   zfree(&err_string);
   err_string = sstrdup(s);
}

void aeff_close(AeFile* f) {
   gzclose(f->f);
   sfree(f);
}

void aeff_write_header(AeFile *f, const char *header_name) {
   aeff_indent(f);
   gzprintf(f->f, "<%s\n", header_name);
   f->indent_level++;
}

/* generalization for read_header and read_footer */
static bool aeff_read_mark(AeFile *f, const char *mark_name, char pref) {
   char *line;
   char *s;
   zfree(&err_string);

   autod_begin;
   autod_register(line, free);
   
   autod_assign(line, freadline_ex(f->f, gzgetc));

   /* if EOF was reached before anything could be read, something is wrong */
   if (!line) {
      err_string = dsprintf("EOF reading mark '%c%s'", pref, mark_name);
      autod_return_value(false);
   }

   /* discard leading blanks, store start of real text in s */
   s = line;
   while (*s == ' ' || *s == '\t') s++;

   /* try to recognize the line as a header/footer, and match its name */
   if (strcmp(&s[1], mark_name) || s[0] != pref) {
      err_string = dsprintf("Mark '%c%s' not found.", pref, mark_name);
      autod_return_value(false);
   }

   /* everything checks ok */
   autod_return_value(true);
}

bool aeff_read_header(AeFile *f, const char *header_name) {
   return aeff_read_mark(f, header_name, '<');
}

void aeff_write_footer(AeFile *f, const char *footer_name) {
   f->indent_level--;
   aeff_indent(f);
   gzprintf(f->f, ">%s\n", footer_name);
}

bool aeff_read_footer(AeFile *f, const char *footer_name) {
   return aeff_read_mark(f, footer_name, '>');
}

void  aeff_write_int(AeFile *f, const char *label, int a) {
   aeff_indent(f);
   gzprintf(f->f, "%s: int: %d\n", label, a);
}

bool aeff_read_int(AeFile *f, const char *label, int *ret) {
   char *rlabel, *rtype, *rvalue;

   autod_begin;
   autod_register(rlabel, free);
   autod_register(rtype, free);
   autod_register(rvalue, free);

   if (!read_parse_data_line(f->f, &rlabel, &rtype, &rvalue))
      return false;
   
   if (strcmp(rtype, "int")) {
      zfree(&err_string);
      err_string = dsprintf("Field '%s' not marked as integer", label);
   
      autod_return_value(false);
   }

   if (strcmp(rlabel, label)) {
      zfree(&err_string);
      err_string = dsprintf("Expected label '%s', found '%s'", label, rlabel);
   
      autod_return_value(false);
   }

   if (ret) *ret = atoi(rvalue);

   autod_return_value(true);
}

void aeff_write_bool(AeFile *f, const char *label, bool a) {
   aeff_indent(f);
   gzprintf(f->f, "%s: bool: %s\n", label, a ? "true" : "false");
}

bool aeff_read_bool(AeFile *f, const char *label, bool *ret) {
   char *rlabel, *rtype, *rvalue;

   autod_begin;
   autod_register(rlabel, free);
   autod_register(rtype, free);
   autod_register(rvalue, free);

   if (!read_parse_data_line(f->f, &rlabel, &rtype, &rvalue))
      return false;

   if (strcmp(rtype, "bool")) {
      zfree(&err_string);
      err_string = dsprintf("Field '%s' not marked as bool", label);
   
      autod_return_value(false);
   }

   if (strcmp(rlabel, label)) {
      zfree(&err_string);
      err_string = dsprintf("Expected label '%s', found '%s'", label, rlabel);
   
      autod_return_value(false);
   }

   if (ret) *ret = (*rvalue == 't') ? true : false;

   autod_return_value(true);
}

void aeff_write_string(AeFile *f, const char *label, const char *s) {
   aeff_indent(f);
   gzprintf(f->f, "%s: str: ", label);
   
   /* now encode and write string */
   while (*s) {
      if (*s >= 0 && *s <= 31) {
         gzputc(f->f, '\\');
	 gzputc(f->f, *s + '0');
      }
      else if (*s == '\\') gzputs(f->f, "\\\\");
      else                 gzputc(f->f, *s);
      s++;
   }

   gzputc(f->f, '\n');
}

bool aeff_read_string(AeFile *f, const char *label, char **ret) {
   char *rlabel, *rtype, *rvalue;

   autod_begin;
   autod_register(rlabel, free);
   autod_register(rtype, free);
   autod_register(rvalue, free);

   if (!read_parse_data_line(f->f, &rlabel, &rtype, &rvalue))
      autod_return_value(false);

   if (strcmp(rtype, "str")) {
      zfree(&err_string);
      err_string = dsprintf("Field '%s' not marked as string", label);
   
      autod_return_value(false);
   }

   if (strcmp(rlabel, label)) {
      zfree(&err_string);
      err_string = dsprintf("Expected label '%s', found '%s'", label, rlabel);
   
      autod_return_value(false);
   }

   if (ret) *ret = sstrdup(rvalue);

   autod_return_value(true);
}


