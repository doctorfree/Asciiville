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

#include "aewl.h"
#include "field.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

/* field buffer dynamic reallocation settings */
#define FIELD_DEFAULT_CAP 20
#define FIELD_CAP_ADD 20

static void aewl_field_ensure_cap(AewlField *f, int cap);
static void buf_insert_char(char *buf, int cap, int pos, int ch);
static void buf_del_char(char *buf, int cap, int pos, int ch);

AewlWidget *aewl_field_create(int x, int y, int width, int shortcut,
                              int result, const char *text) {
   AewlField *f = malloc(sizeof(AewlField));
   AewlWidget *base;

   if (!f) return NULL;

   memset(f, 0, sizeof(AewlField));
   base = &f->base;

   aewl_widget_fill_defaults(base);
   base->x = x;
   base->y = y;
   base->width = width;
   base->height = 1;
   base->focusable = true;
   base->shortcut = shortcut;
   base->fn_repaint = aewl_field_repaint;
   base->fn_destroy = aewl_field_destroy;
   base->fn_handlekey = aewl_field_handlekey;

   f->result = result;
   f->pos = 0;
   f->buffer = malloc(f->cap = FIELD_DEFAULT_CAP);
   if (!f->buffer) return NULL;

   if (text) aewl_field_set_text(f, text);

   return (AewlWidget*) f;
}

void aewl_field_set_text(AewlField *f, const char *text) {
   int len = strlen(text);
   aewl_field_ensure_cap(f, len + 1);
   strcpy(f->buffer, text);

   if (f->pos > len) f->pos = len;
}

const char *aewl_field_get_text(AewlField *f) {
   return f->buffer;
}

void aewl_field_destroy(AewlWidget *w) {
   AewlField *f = (AewlField*) w;
   if (f->buffer) free(f->buffer);
   free(f);
}

void aewl_field_repaint(AewlWidget *w, bool focused) {
   int i;
   AewlField *f = (AewlField*) w;
   const char *str = f->buffer;

   wattrset(w->win, AEWL_ATTR(field, focused, false));
   
   /* print text and pad rest of field with spaces */
   wmove(w->win, w->y, w->x);
   for (i = 0; i < w->width; i++) {
      if (*str) {
         waddch(w->win, *str);
         str++;
      }
      else {
         wattrset(w->win, aewl_attrs.field_pad);
         waddch(w->win, '_');
      }
         
   }

   /* position cursor where it should be */
   wmove(w->win, w->y, w->x + f->pos);
}

int aewl_field_handlekey(AewlWidget *w, int ch) {
   AewlField *f = (AewlField*) w;
   int len = strlen(f->buffer);

   /* if it is a 'printable' character, insert it into the field */
   if (ch >= 32 && ch <= 255 && ch != 127) {
      aewl_field_ensure_cap(f, len + 2);
      buf_insert_char(f->buffer, f->cap, f->pos, ch);
      f->pos++;
      return 0;
   }

   /* test for control keys and such */
   switch (ch) {
      case 10: return f->result;
      case KEY_RIGHT: if (f->pos < len) f->pos++; /* it is ok for pos == len */
                      break;
      case KEY_LEFT: if (f->pos) f->pos--; break;
      case KEY_HOME: f->pos = 0; break;
      case KEY_END:  f->pos = len; break;
      case 'U'-'A'+1: aewl_field_set_text(f, ""); break;

      case KEY_BACKSPACE: case 8:
         if (f->pos) {
            buf_del_char(f->buffer, f->cap, f->pos - 1, ch);
            f->pos--;
         }
         break;
      
      case KEY_DC:
         if (f->pos < len) buf_del_char(f->buffer, f->cap, f->pos, ch);
         break;
   }

   return 0;
}


static void aewl_field_ensure_cap(AewlField *f, int cap) {
   if (f->cap >= cap) return;

   if (! (f->buffer = realloc(f->buffer, f->cap = cap + FIELD_CAP_ADD)) )
      abort();
}

static void buf_insert_char(char *buf, int cap, int pos, int ch) {
   /* this function does NOT reallocate the buffer to make room, it
    * expects there to be room already */
   int i;
   for (i = cap - 1; i > pos; i--) buf[i] = buf[i-1];
   buf[pos] = ch;
}

static void buf_del_char(char *buf, int cap, int pos, int ch) {
   int i;
   for (i = pos; i < cap - 1; i++) buf[i] = buf[i+1];
}


