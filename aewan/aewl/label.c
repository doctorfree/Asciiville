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

INFORMA��ES DE LICEN�A:
Este programa � um software de livre distribui��o; voc� pode
redistribu�-lo e/ou modific�-lo sob os termos da GNU General
Public License, conforme publicado pela Free Software Foundation,
pela vers�o 2 da licen�a ou qualquer vers�o posterior.

Este programa � distribu�do na esperan�a de que ele ser� �til
aos seus usu�rios, por�m, SEM QUAISQUER GARANTIAS; sem sequer
a garantia impl�cita de COMERCIABILIDADE ou DE ADEQUA��O A
QUALQUER FINALIDADE ESPEC�FICA. Consulte a GNU General Public
License para obter mais detalhes (uma c�pia acompanha este
programa, armazenada no arquivo COPYING).
*/

#include "aewl.h"
#include "label.h"
#include "util.h"
#include "form_int.h"
#include <string.h>
#include <stdlib.h>

/* creates a label with the given parameters */
AewlWidget *aewl_label_create(int x, int y, int width, const char *text) {
   AewlLabel *l = malloc(sizeof(AewlLabel));
   AewlWidget *base;
   if (!l) return NULL;
   memset(l, 0, sizeof(AewlLabel));

   base = &l->base;

   aewl_widget_fill_defaults(base);
   base->x = x;
   base->y = y;
   base->width = (width > 0) ? width : aewl_util_strlen_esc(text);
   base->height = 1;
   base->focusable = false;
   base->shortcut = 0;

   base->fn_repaint = aewl_label_repaint;
   base->fn_destroy = aewl_label_destroy;
   base->fn_handlekey = NULL;

   l->caption = strdup(text);
   if (!l->caption) return NULL;

   return (AewlWidget*) l;
}

/* destroys a label */
void aewl_label_destroy(AewlWidget *w) {
   AewlLabel *l = (AewlLabel*) w;
   free(l->caption);
   free(w);
}

/* repaint a label */
void aewl_label_repaint(AewlWidget *w, bool focused) {
   AewlLabel *l = (AewlLabel*) w;
   int n = w->width;

   if (l->focus_peer && (l->focus_peer == w->form->focus_widget))
      focused = true;

   /* clear the label first */
   wmove(w->win, w->y, w->x);
   while (n--) waddch(w->win, ' ');

   /* now print contents */
   aewl_util_mvwaddstr(w->win, w->y, w->x,
        AEWL_ATTR(text, focused, false),
        AEWL_ATTR(text, focused, true),
        l->caption);
}

/* sets the label's text to the given text */
void aewl_label_set_text(AewlLabel *l, const char *text) {
   if (l->caption) free(l->caption);
   l->caption = strdup(text);
   if (!l->caption) abort();
}

void aewl_label_set_focus_peer(AewlLabel *l, AewlWidget *peer) {
   l->focus_peer = peer;
}

