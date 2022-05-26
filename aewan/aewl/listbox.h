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

#ifndef _btco_aewl_listbox_h
#define _btco_aewl_listbox_h

#include "widget.h"

/* definition of AewlListBox */
typedef struct AewlListBox_ {
   AewlWidget base;     /* the widget */
   char *caption;       /* the title that appears at the top of the listbox's
                         * bounding frame */
   int result;          /* the result code that the listbox issues
                         * when the user presses ENTER on it */

   int sel; /* currently selected item */
   int iat; /* index at top, that is, what list item is currently being
             * displayed at the top of the list area. This value changes
             * as the list is scrolled */

   /* dynamic array of listbox items */
   int item_count;
   int capacity;
   char **items;
} AewlListBox;

/* creates a list box with the given parameters. The caption accepts
 * the '&' escape code for highlighting a character. */
AewlWidget *aewl_listbox_create(int x, int y, int width, int height,
                                int shortcut, const char *caption,
                                int result);

/* empties the list box, that is, deletes all items */
void aewl_listbox_clear(AewlListBox *lb);

/* adds an item to the list box */
void aewl_listbox_add(AewlListBox *lb, const char *text);

/* returns the text of the list item whose index is i, or NULL
 * if the index is invalid. */
const char *aewl_listbox_get(AewlListBox *lb, int i);

/* returns what index is currently selected in the listbox.
 * If there is no selection, returns -1. */
int aewl_listbox_get_sel(AewlListBox *lb);

/* sets the text of the item whose index is i to the given text. 
 * Returns false if (and only if) the index i is invalid. */
bool aewl_listbox_set(AewlListBox *lb, int i, const char *newvalue);

/* destroys a list box */
void aewl_listbox_destroy(AewlWidget *listbox);

/* repaint a listbox */
void aewl_listbox_repaint(AewlWidget *listbox, bool focused);

/* key handler function for listboxes */
int aewl_listbox_handlekey(AewlWidget *listbox, int ch);

/* sorts the contents of the list box alphabetically */
void aewl_listbox_sort(AewlListBox *listbox);

#endif

