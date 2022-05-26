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

#include "document.h"
#include "colordlg.h"
#include "psd.h"
#include "absmenu.h"

/*                 DLG_W
-      ,---------------------------,
D      | -                         |
L      | : LIST_H                  |
G      | :                         |
|      | :                         |
H      | -                         |
:      +---------------------------+
:      | A                         |
:      |                           |
:      |                           |
:      |                           |
-      `---------------------------'
*/

#define DLG_W 42
#define DLG_H 19
#define LIST_H 11

static AbsMenu *menu = 0;

static void painter(int i, AbsMenu *m) {
   Layer *l = _doc->layers[i];
   printw("%2d: %c%c (%3d x %3d) %s", i, 
           l->visible ? 'v' : ' ',
           l->transp ? 't' : ' ',
           l->width, l->height, (l->name ? l->name : "(untitled)"));
}

static void paint_dlg(void) {
   int x0, y0; int x, y;
   
   kurses_color(7, 0);
   draw_centered_window(DLG_W, DLG_H, "Layer Manager", &x0, &y0);
   
   if (!menu) {
      menu = menu_create(&(_doc->layer_count), x0,
                         y0, DLG_W - 2, LIST_H,
                         0x71, 0x70, painter);
      menu_select_item(menu, _lyr);
   }
   
   draw_hline(x0 - 1, y0 + LIST_H, DLG_W, ACS_LTEE, ACS_HLINE, ACS_RTEE);

   /* print instructions */
   kurses_move(x = x0 + 1, y = y0 + LIST_H + 1);

   kurses_color(2, 0); addch('<'); addch(ACS_LRCORNER); addch(' ');
   kurses_color(7, 0);
   printw(": go to layer");

   kurses_move(x, ++y);
   kurses_color(2, 0); addstr("d  "); kurses_color(7, 0);
   printw(": delete layer");

   kurses_move(x, ++y);
   kurses_color(2, 0); addstr("+/-"); kurses_color(7, 0);
   printw(": move layer up/down");

   kurses_move(x, ++y);
   kurses_color(2, 0); addstr("v  "); kurses_color(7, 0);
   printw(": toggle visibility");
   
   kurses_move(x, ++y);
   kurses_color(2, 0); addstr("t  "); kurses_color(7, 0);
   printw(": toggle transparency");

   menu_paint(menu);
}

void show_layer_dlg(void) {
   int ch; int quit = 0;
   Layer *lyr;

   menu = 0;
   
   while (!quit) {
      paint_dlg();
      refresh();
      ch = getch();
      switch (ch) {
         case 10: switch_to_layer(menu->sel_item);
                  /* fall through */
         case 27: case 3: case 7: quit = 1; break;
         case 'd': /* delete layer */
                  document_del_layer(_doc, menu->sel_item);
                  if (_doc->layer_count == 0) /* no layers, so close dialog */
                     quit = 1;
                  else if (menu->sel_item >= _doc->layer_count)
                     menu->sel_item = _doc->layer_count - 1;
                  break;
         case '+': case '=': /* move layer up */
                  if (menu->sel_item > 0) {
                     int i = menu->sel_item; Layer **ls = _doc->layers;
                     Layer* l = ls[i-1];
                     ls[i-1] = ls[i];
                     ls[i] = l;

                     menu->sel_item--;
                  }
                  break;
         case '-': case '_':
                  if (menu->sel_item < _doc->layer_count - 1) {
                     int i = menu->sel_item; Layer **ls = _doc->layers;
                     Layer* l = ls[i+1];
                     ls[i+1] = ls[i];
                     ls[i] = l;

                     menu->sel_item++;
                  }
                  break;
         case 'v': /* toggle layer visibility */
                  lyr = _doc->layers[menu->sel_item];
                  lyr->visible = !lyr->visible;
                  break;
         case 't': /* toggle layer transparency */
                  lyr = _doc->layers[menu->sel_item];
                  lyr->transp = !lyr->transp;
                  break;
         
         default: if (menu) menu_handle_key(menu, ch);
      }
   }

   if (menu) menu_destroy(menu);
}

