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
#include "listbox.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

#define DEFAULT_CAPACITY 20
#define CAPACITY_INCREMENT 20

AewlWidget *aewl_listbox_create(int x, int y, int width, int height,
                             int shortcut, const char *caption, int result) {
   AewlListBox *lb = malloc(sizeof(AewlListBox));
   AewlWidget *base = &lb->base;
   if (!lb) return NULL;
   memset(lb, 0, sizeof(AewlListBox));

   aewl_widget_fill_defaults(&(lb->base));
   base->x = x;
   base->y = y;
   base->width = width;
   base->height = height;
   base->focusable = true;
   base->shortcut = shortcut;

   base->fn_repaint = aewl_listbox_repaint;
   base->fn_destroy = aewl_listbox_destroy;
   base->fn_handlekey = aewl_listbox_handlekey;

   lb->caption = caption ? strdup(caption) : NULL;
   lb->result = result;
   lb->sel = 0;
   lb->iat = 0;

   lb->item_count = 0;
   lb->capacity = DEFAULT_CAPACITY;
   lb->items = malloc(sizeof(char*) * DEFAULT_CAPACITY);
   if (!lb->items) {
      free(lb);
      return NULL;
   }

   return (AewlWidget*) lb;
}


void aewl_listbox_clear(AewlListBox *lb) {
   int i;
   for (i = 0; i < lb->item_count; i++) 
      if (lb->items[i]) free(lb->items[i]);
   lb->item_count = 0;
   lb->iat = 0;
   lb->sel = 0;
}

void aewl_listbox_add(AewlListBox *lb, const char *text) {
   if (lb->capacity <= lb->item_count) {
      lb->capacity += CAPACITY_INCREMENT;
      lb->items = realloc(lb->items, sizeof(char*) * lb->capacity);
      if (!lb->items) abort();
   }

   /* add new item to end of list now that we are sure there is
    * capacity for it */
   if (! (lb->items[lb->item_count++] = strdup(text)) ) abort();
}

const char *aewl_listbox_get(AewlListBox *lb, int i) {
   if (i < 0 || i >= lb->item_count) return NULL;
   return lb->items[i];
}

bool aewl_listbox_set(AewlListBox *lb, int i, const char *newvalue) {
   if (i < 0 || i >= lb->item_count) return false;
   if (lb->items[i]) free(lb->items[i]);
   if (! (lb->items[i] = strdup(newvalue)) ) abort();
   return true;
}

void aewl_listbox_destroy(AewlWidget *listbox) {
   AewlListBox *lb = (AewlListBox*) listbox;
   aewl_listbox_clear(lb);
   free(lb->items);
   if (lb->caption) free(lb->caption);
   free(lb);
}

void aewl_listbox_repaint(AewlWidget *listbox, bool focused) {
   AewlListBox *lb = (AewlListBox*) listbox;
   AewlWidget *base = &lb->base;
   int i, y;

   /* erase and draw bounding box */
   wattrset(base->win, AEWL_ATTR(frame, focused, false));
   aewl_util_erase(base->win, base->x, base->y, base->width, base->height);
   aewl_util_drawbox(base->win, base->x, base->y, base->width, base->height);

   /* draw caption across top, if any */
   if (lb->caption) 
      aewl_util_mvwaddstr(base->win, base->y, base->x + 2, 
         AEWL_ATTR(frame, focused, false),
         AEWL_ATTR(frame, focused, true), lb->caption);

   /* draw items, if there are any */
   if (lb->item_count) {
      for (                y = base->y + 1,                i = lb->iat; 
                           y < base->y + base->height - 1 && i < lb->item_count;
                           i++, y++) {

         wmove(base->win, y, base->x + 1);
         wattrset(base->win, AEWL_ATTR(field, focused, i == lb->sel));
         aewl_util_addnstr(base->win, base->width - 2, lb->items[i]);
      }
   }

   /* if there are more items than can be seen, show an indication */
   wattrset(base->win, AEWL_ATTR(frame, focused, true));

   if (lb->iat > 0)
      mvwaddch(base->win, base->y + 1,  base->x + base->width - 1, '^');
   if (lb->iat + base->height - 2 < lb->item_count)
      mvwaddch(base->win, base->y + base->height - 2, 
                          base->x + base->width - 1, 'v');

   /* move the cursor to the selected item */
   wmove(base->win, base->y + 1 + (lb->sel - lb->iat), base->x + 1);
}

int aewl_listbox_handlekey(AewlWidget *listbox, int ch) {
   AewlListBox *lb = (AewlListBox*) listbox;
   AewlWidget *base = &lb->base;

   switch (ch) {
      case KEY_UP:    lb->sel--; break;
      case KEY_DOWN:  lb->sel++; break;
      case KEY_PPAGE: lb->sel -= base->height - 2; break;
      case KEY_NPAGE: lb->sel += base->height - 2; break;
      case 10: return lb->result; /* action key */
   }

   /* apply corrections */
   if (lb->sel < 0) lb->sel = 0;
   if (lb->sel >= lb->item_count) lb->sel = lb->item_count - 1;

   /* ensure visibility of selected item */
   if (lb->sel < lb->iat) lb->iat = lb->sel;
   if (lb->sel >= lb->iat + (base->height - 2))
      lb->iat = lb->sel - (base->height - 2) + 1;

   return 0;
}

int aewl_listbox_get_sel(AewlListBox *lb) {
   if (lb->sel >= 0 && lb->sel < lb->item_count) return lb->sel;
   return -1;
}

static int _compar(const void *p1, const void *p2) {
   const char **a = (const char**) p1;
   const char **b = (const char**) p2;
   return strcmp(*a, *b);
}

void aewl_listbox_sort(AewlListBox *lb) {
   qsort(lb->items, lb->item_count, sizeof(const char*), _compar);
}


