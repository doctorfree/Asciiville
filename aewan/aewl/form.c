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

#include <string.h>
#include <stdlib.h>
#include "form_int.h"

AewlForm *aewl_form_create(WINDOW *w) {
   AewlForm *f = malloc(sizeof(struct AewlForm_));
   if (!f) return NULL;

   f->win = w;
   f->first_widget = NULL;
   f->last_widget = NULL;
   f->focus_widget = NULL;
   return f;
}

void aewl_form_add_widget(AewlForm *f, AewlWidget *wid) {
   wid->win = f->win;  /* tell widget where it is supposed to draw itself */
   wid->form = f;      /* tell widget which form it belongs to */
   wid->next = NULL;   /* will be the end of the list */

   if (!f->first_widget)        /* form's first widget */
      f->first_widget = f->last_widget = wid;
   else                         /* append to end of list */
      f->last_widget->next = f->last_widget = wid;

   /* if this widget is focusable, and form does not yet have a focused
    * widget, this one becomes the focused widget */
   if (!f->focus_widget && wid->focusable) f->focus_widget = wid;
}

void aewl_form_paint(AewlForm *f, bool refresh) {
   AewlWidget *w;

   /* paint all the nonfocused widgets */
   for (w = f->first_widget; w; w = w->next)
      if (w != f->focus_widget && w->fn_repaint) w->fn_repaint(w, false);

   /* now paint focus widget */
   if (f->focus_widget && f->focus_widget->fn_repaint)
      f->focus_widget->fn_repaint(f->focus_widget, true);

   if (refresh) wrefresh(f->win);
}

int aewl_form_dispatch_key(AewlForm *f, int ch) {
   AewlWidget *w;
   int result;

   if (ch == 9 && f->focus_widget) {
      /* tab key: move focus to next focusable widget */

      /* notice that if f->focus_widget == NULL, that means that the
       * form does not have ANY focusable widget, in which case
       * we don't even enter this 'if' */
      do {
         f->focus_widget = f->focus_widget->next;
         if (!f->focus_widget) f->focus_widget = f->first_widget;
      } while (!f->focus_widget->focusable);

      aewl_form_paint(f, true);
      return 0;
   }

   /* check if the key is a shortcut for some widget; if so, it gets the 
    * focus */
   for (w = f->first_widget; w; w = w->next) {
      if (w->shortcut == ch && w->focusable) {
         f->focus_widget = w;
         aewl_form_paint(f, true);
         return 0;
      }
   }

   /* deliver key to focused widget, if there is one */
   if (f->focus_widget && f->focus_widget->fn_handlekey) {
      result = f->focus_widget->fn_handlekey(f->focus_widget, ch);
      aewl_form_paint(f, true);
      return result;
   }
   
   return 0;  /* nothing done */
}

void aewl_form_destroy(AewlForm *f) {
   AewlWidget *tmp, *w = f->first_widget;

   while (w) {
      tmp = w->next;
      w->fn_destroy(w);
      w = tmp;
   }

   free(f);
}

AewlWidget *aewl_form_get_focus_widget(AewlForm *fn) {
   return fn->focus_widget;
}


