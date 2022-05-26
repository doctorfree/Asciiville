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
#include "button.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

AewlWidget *aewl_button_create(int x, int y, int width, int shortcut,
                               int result, const char *caption) {
   AewlButton *b = malloc(sizeof(AewlButton));
   if (!b) return NULL;
   memset(b, 0, sizeof(AewlButton));

   if (width <= 0) width = aewl_util_strlen_esc(caption) + 2;

   aewl_widget_fill_defaults(&(b->base));
   b->base.x = x;
   b->base.y = y;
   b->base.height = 1;
   b->base.width = width;
   b->base.focusable = true;
   b->base.shortcut = shortcut;

   b->base.fn_repaint = aewl_button_repaint;
   b->base.fn_destroy = aewl_button_destroy;
   b->base.fn_handlekey = aewl_button_handlekey;

   b->caption = strdup(caption);
   b->result = result;

   return (AewlWidget*) b;
}

void aewl_button_destroy(AewlWidget *button) {
   AewlButton *b = (AewlButton*) button;
   if (b->caption) free(b->caption);
   free(b);
}

void aewl_button_repaint(AewlWidget *button, bool focused) {
   AewlButton *b = (AewlButton*) button;
   int i;
   int cx;

   wattrset(b->base.win, AEWL_ATTR(button, focused, false));

   /* print button background by just placing spaces */
   wmove(b->base.win, b->base.y, b->base.x);
   for (i = 0; i < b->base.width; i++) waddch(b->base.win, ' ');

   /* center caption and print it */
   cx = (2 * b->base.x + b->base.width - strlen(b->caption)) / 2;
   aewl_util_mvwaddstr(b->base.win, b->base.y, cx, 
      AEWL_ATTR(button, focused, false),
      AEWL_ATTR(button, focused, true),
      b->caption);
}

int aewl_button_handlekey(AewlWidget *button, int ch) {
   AewlButton *b = (AewlButton*) button;
   return (ch == 10 || ch == ' ') ? b->result : 0;
}


