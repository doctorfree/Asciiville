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

#ifndef _btco_aewl_widget_h
#define _btco_aewl_widget_h

#include <ncurses.h>
#include <stdbool.h>

struct AewlForm_;

/* definition of AewlWidget */
struct AewlWidget_;
struct AewlWidget_ {
   WINDOW *win;                 /* the window where this widget draws
                                 * itself. This is set when the
                                 * widget is added to a form, you don't
                                 * need to assign it explicitly. */

   struct AewlForm_ *form;      /* the form to which this widget belongs. This
                                 * is set when the widget is added to a form */

   int x, y, width, height;     /* widget position and dimensions */

   int shortcut;                /* shortcut key to focus widget */
   bool focusable;              /* indicates whether the widget can
                                 * receive input focus */

   struct AewlWidget_* next;    /* link to next widget in chain; for use
                                 * when chaining widgets in a linked list */

   /* functions */

   /* repaints widget onscreen. <focused> indicates whether the widget
    * is focused or not. Can be NULL (does not repaint)  */
   void (*fn_repaint)(struct AewlWidget_ *self, bool focused);

   /* handles a keypress directed to this widget. Returns a result code
    * (an integer value, plain and simple). This function pointer
    * can be NULL (handles no keys, result code returned always 0) */
   int (*fn_handlekey)(struct AewlWidget_ *self, int ch);

   /* destroys the widget */
   void (*fn_destroy)(struct AewlWidget_ *self);
};
typedef struct AewlWidget_ AewlWidget;

/* There is no function to create a widget out of thin air. This is like
 * an "abstract class" if you want to think about it that way. */

/* Fills in (in-place) the default values for the fields of the given widget.
 * This function is to be called from concrete widget implementation code */
void aewl_widget_fill_defaults(AewlWidget* w);

#endif

