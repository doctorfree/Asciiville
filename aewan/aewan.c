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
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "document.h"
#include "keys.h"
#include "ui.h"
#include "psd.h"
#include "handlekey.h"
#include "welcomedlg.h"
#include "keybind.h"
#include "init_aewl.h"

/* ----------------------------------------------------------------------- */
/* function declarations                                                   */
/* ----------------------------------------------------------------------- */

/* Redraws status bar at the top of the screen */
void paint_status_bar(void);

/* Paints the desktop background */
void paint_desktop(void);

/* Redraws screen: status bar, canvas, etc */
void paint_screen(void);

/* ----------------------------------------------------------------------- */
/* function implementations                                                */
/* ----------------------------------------------------------------------- */
void paint_status_bar() {
   kurses_move(0, kurses_height() - 1);
   kurses_color(3, 0);
   printw("[Lyr %2d] (%3d, %3d) ", _lyr, _x, _y);
   
   if (_selmode == SM_NONE) {
      printw("%-20s", _doc->layer_count ? _doc->layers[_lyr]->name : "[empty]");
      kurses_color(3, 0);
      addstr("   ");

      kurses_color(_fg, _bg);
      addstr("AaBbCc");
      kurses_color(7, 0);
      addstr("    ");

      if (_insmode) {
         kurses_color(0, 6 + 16);
         printw(" INSERT ");
         kurses_color(7, 0);
      }
      else printw("        ");
      
      if (_lgmode) {
         kurses_color(0, 4);
         addstr(" LINES! ");
         kurses_color(7, 0);
      }
      else printw("        ");
      
      if (_compmode) {
         kurses_color(0, 5);
         addstr(" COMPOS ");
         kurses_color(7, 0);
      }
   }
   else if (_selmode == SM_SELECT) {
      kurses_color(0, 2);
      printw(" SELECT ");
      kurses_color(2, 0);
      printw(" (c)opy (m)ove (e)rase (f)g-tint (b)g-tint (o)-edge");
   }
   else {
      kurses_color(0, 1);
      printw(" FLOAT ");
      kurses_color(1, 0);
      printw(" (s)tamp (x) flip-x (y) flip-y (t)ransparent ");
   }
}

void layer_paint_callback(int x, int y, int *ch, int *attr) {
   int x0, x1, y0, y1;
   get_norm_sel(&x0, &y0, &x1, &y1);

   if (x >= x0 && x <= x1 && y >= y0 && y <= y1) {
      /* invert the colors (approximatelly) */
      *attr = ((*attr & 0x0F) << 4) | (*attr >> 4);
      *attr &= 0xF7; /* turn of blinking attribute */
   }
}

void paint_screen() {
   int scr_width, scr_height, i;

   scr_width = kurses_width();
   scr_height = kurses_height();
   
   erase();
   paint_desktop();
   paint_status_bar();
   if (doc_empty()) {
      static char *msg = "[Document contains no layers; press F1 for menu]";
      kurses_move((scr_width - strlen(msg)) / 2, scr_height / 2);
      kurses_color(7, 0);
      printw(msg);
      refresh();
      return;
   }

   /* if in composite mode, paint all visible layers in reverse order */
   if (_compmode) {
      for (i = _doc->layer_count - 1; i >= 0; i--) {
         Layer *l = _doc->layers[i];
         if (l->visible)
            layer_paint(l, -_svx, -_svy, 0, 0,
                        scr_width, scr_height - 1, 
                        (_selmode == SM_SELECT && i == _lyr) ? 
                        layer_paint_callback : 0);
      }
   }
   else {
      /* paint only current layer regardless of visibility, and with no
       * transparency */
      layer_paint_opaque(_doc->layers[_lyr], -_svx, -_svy, 0, 0,
                         scr_width, scr_height - 1, 
                         _selmode == SM_SELECT ?  layer_paint_callback : 0);
   }

   if (_clipboard && _selmode == SM_FLOAT)
      /* paint clipboard */
      layer_paint(_clipboard, _x - _svx, _y - _svy, 0, 0,
                  scr_width, scr_height - 1, NULL);
   
   if (!kurses_move(_x - _svx, _y - _svy)) kurses_move(0, 0);
   refresh();
}

void paint_desktop() {
   int scr_width, scr_height;
   int x, y;

   scr_width = kurses_width();
   scr_height = kurses_height();
   
   kurses_color(8, 0);
   for (y = 0; y < scr_height - 1; y++) {
      kurses_move(0, y);
      for (x = 0; x < scr_width; x++) addch(ACS_CKBOARD);
   }
}

/* ----------------------------------------------------------------------- */
/* main function                                                           */
/* ----------------------------------------------------------------------- */
int main(int argc, char **argv) {
   /* debug output to /tmp/aewan.log, or /dev/null */
   close(2);
   #ifdef DEBUG
      if (2 != open("aewan.log", O_WRONLY | O_CREAT | O_TRUNC)) exit(13);
   #else
      if (2 != open("/dev/null", O_WRONLY)) exit(13);
   #endif
   
   zero_state();   
   kurses_init();
   atexit(kurses_finalize);
   keys_init();
   keybind_init();
   init_aewl();

   if (argc >= 2)
      u_load_file(argv[1]);
   else
      show_welcome_dlg();

   while (1) {
      paint_screen();
      handle_key(getch());
   }
}

