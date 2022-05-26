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

#ifndef _btco_aewl_form_h
#define _btco_aewl_form_h

#include <ncurses.h>
#include <stdbool.h>
#include "widget.h"

/* Defines a form, an abstract entity used as a container for widgets.
 * The structure is opaque; see form.c if curious about its internals */
struct AewlForm_;
typedef struct AewlForm_ AewlForm;

/* Creates a new form. The passed curses window is where the widgets will 
 * be rendered. Pass stdscr to render the form to the main screen rather
 * than a window. Returns NULL if a problem occurs (i.e., out of memory)  */
AewlForm *aewl_form_create(WINDOW *w);

/* Adds a widget to a form. The form will be responsible for deleting
 * it, you needn't (and mustn't) mind */
void aewl_form_add_widget(AewlForm *frm, AewlWidget *wid);

/* Paints all the form's widgets to the screen (i.e. to the window that
 * the form is tied with). You only need to call this once, or when
 * the screen contents are somehow lost. Widgets redraw themselves
 * when appropriate without your interference. 
 *
 * This function will call wrefresh on the form's window if refresh == true.
 */
void aewl_form_paint(AewlForm *frm, bool refresh);

/* Process a keystroke. This will check what is the widget that currently
 * has the focus and will deliver the keypress to its handler. Also handles
 * keys for switching focus and so on. Returns a result code (just a plain
 * integer -- you can configure your widgets to return certain result codes
 * when they are activated), or 0 (special result code meaning the key
 * was not handled by any widget).
 *
 * A typical construct for "running" a form is
 *
 * aewl_form_paint(f);
 * while (42 != aewl_form_dispatch_key(ch));
 * aewl_form_destroy(f);
 *
 * The above code will run the form until some widget returns the result
 * code 42 in response to a keypress.
 *
 * We don't provide you a function to do that (such as aewl_form_run or
 * something) because you will always want to intercept keystrokes and
 * process them somehow before handing it over to aewl_form_dispatch,
 * (if nothing else, for knowing when to close the form!) so this 'event 
 * loop' must be written by you.
 */
int aewl_form_dispatch_key(AewlForm *frm, int ch);

/* Destroys (deallocates) the given form and ALL WIDGETS WITHIN. Do not
 * destroy widgets separately! */
void aewl_form_destroy(AewlForm *frm);

/* Returns which is the focused widget in the form */
AewlWidget *aewl_form_get_focus_widget(AewlForm *fn);


#endif

