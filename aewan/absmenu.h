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


/* absmenu.h - an abstract menu that lets creator specify a paint function,
 *             and even the address of an item count */

#ifndef _btco_aewan_abmenu_h
#define _btco_aewan_abmenu_h

struct _AbsMenu {
   int __ic;    /* itemcount variable, if the caller chose to use
                 * nondynamic count */
   int *item_count; /* pointer to item count variable. If the caller chose
                     * nondynamic count, this points to __ic. Can never be
                     * NULL. */
   int x0, y0;  /* rendering position onscreen */
   int w, h;    /* dimensions of renderable area */

   int attr_sel, attr_unsel;  /* color attributes for selected and 
                                 unselected items  */
   
   int item_at_top;  /* which item index is currently being drawn at
                      * the top of the menu. This defines the scrolling
                      * position of the menu. If *item_count > 0,
                      * this must be a value such that
                      * 0 <= item_at_top < *item_count */
   
   int sel_item;     /* the selected item in the menu. If *item_count > 0,
                      * this must be a value such that
                      * 0 <= sel_item < *item_count */
   
   void (*painter)(int i, struct _AbsMenu *menu);
                      /* painter function that paints menu
                         entry index i */
};
typedef struct _AbsMenu AbsMenu;

/* Create a new abstract menu. 
 * ic - address of a variable that will serve as the menu's item count.
 *      Pass NULL to use an internally maintained count that can be
 *      manipulated with menu_set_count and menu_get_count.
 *
 * x0, y0, w, h - a rectangle that indicates the area of the screen to which
 *      the menu should be rendered
 *
 * attr_sel, attr_unsel - color attributes to use for selected
 *      entries and unselected entries. The lowest four bits of an attribute
 *      is the background color, the next four bits indicate the foreground
 *      color. 
 *
 * painter - the address of a function that will be called in order to
 *      paint the menu items. The cursor will be appropriately placed
 *      before the function call, and the whole line will already have
 *      been filled with the appropriate color; also, the appropriate
 *      color will already be set. The function also receives a pointer
 *      to the menu itself for convenience.
 */
AbsMenu *menu_create(int *ic, int x0, int y0, int w, int h, 
                  int attr_sel, int attr_unsel,
                  void (*painter)(int, struct _AbsMenu *));

/* Destroys a menu previously created with menu_create */
void menu_destroy(AbsMenu *m);

/* Set the item count for menu m. This can only be used if NULL was
 * passed as the <ic> parameter of menu_create when the menu was
 * created. */
void menu_set_count(AbsMenu *m, int i);

/* Obtains menu item count */
int menu_get_count(AbsMenu *m);

/* Sets the selected item of the menu. Applies scrolling
 * corrections to make sure item is visible */
void menu_select_item(AbsMenu *m, int i);

/* Paints menu to screen. Does not call refresh() */
void menu_paint(AbsMenu *m);

/* Performs appropriate action in response to key ch. For example,
 * if ch is KEY_UP, decrement m->sel_item, etc. */
void menu_handle_key(AbsMenu *m, int ch);

#endif

