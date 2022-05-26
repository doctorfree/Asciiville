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

#include "bores/bores.h"
#include <ncurses.h>
#include <stdlib.h>

#include "absmenu.h"

AbsMenu *menu_create(int *ic, int x0, int y0, int w, int h, 
                  int attr_sel, int attr_unsel,
                  void (*painter)(int, struct _AbsMenu *)) 
{
   AbsMenu *new_menu = zalloc(sizeof(struct _AbsMenu));
   new_menu->item_count = ic ? ic : &(new_menu->__ic);
   new_menu->x0 = x0;
   new_menu->y0 = y0;
   new_menu->w  = w;
   new_menu->h  = h;
   new_menu->attr_sel = attr_sel;
   new_menu->attr_unsel = attr_unsel;
   new_menu->painter = painter;
   return new_menu;
}

void menu_destroy(AbsMenu *m) {
   if (m) free(m);
}

void menu_set_count(AbsMenu *m, int i) {
   m->__ic = i;
}

int menu_get_count(AbsMenu *m) {
   return *(m->item_count);
}

void menu_paint(AbsMenu *m) {
   int i; int index; int j;
   int sely = m->y0;
   for (i = 0; i < m->h; i++) {
      index = i + m->item_at_top;
      if (index == m->sel_item) sely = m->y0 + i;
      
      kurses_move(m->x0, m->y0 + i);
      kurses_color_at((m->sel_item == index) ? m->attr_sel : m->attr_unsel);
      for (j = 0; j < m->w; j++) addch(' ');
      
      kurses_move(m->x0, m->y0 + i);
      if (index >= 0 && index < *(m->item_count)) (*m->painter)(index, m);
   }

   /* place cursor on currently selected item */
   kurses_move(m->x0, sely);
}

static void menu_correct(AbsMenu *m) {
   /* corrige m->sel_item */
   while (m->sel_item < 0) m->sel_item = 0;
   while (m->sel_item >= *(m->item_count)) m->sel_item = -1 + *(m->item_count);
   
   /* corrige m->item_at_top de forma que o item selecionado
    * seja visível */
   if (m->item_at_top > m->sel_item)
      m->item_at_top = m->sel_item;
   else if (m->sel_item >= m->item_at_top + m->h)
      m->item_at_top = m->sel_item - m->h + 1;
}

void menu_handle_key(AbsMenu *m, int ch) {
   switch (ch) {
      case KEY_UP: m->sel_item--; break;
      case KEY_DOWN: m->sel_item++; break;
      case KEY_NPAGE: m->sel_item += m->h; break;
      case KEY_PPAGE: m->sel_item -= m->h; break;
   }

   menu_correct(m);
}

void menu_select_item(AbsMenu *m, int i) {
   m->sel_item = i;
   menu_correct(m);
}

