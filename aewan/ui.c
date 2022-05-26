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
#include <string.h>
#include <stdlib.h>

#include "ui.h"

int ui_cancel;  /* global cancel flag */

/* clears bottom line of screen and puts cursor at the beginning of that line.
 * Also lowers the global ui_cancel flag. */
static void ui_prepare_ex(int fg, int bg) {
   int w = kurses_width();
   kurses_move(0, kurses_height() - 1);
   kurses_color(fg, bg);
   while (w--) addch(' ');
   kurses_move(0, kurses_height() - 1);
   ui_cancel = 0;
}

static void ui_prepare(void) {
   ui_prepare_ex(15, 4);
}


int ui_ask_i(const char *prompt, int def, int min, int max) {
   static char buf[32]; int ans;
   
   do {
      ui_prepare();
      printw("%s [%d]: ", prompt, def);
      if (!kurses_line_input(buf, 32)) return (ui_cancel = -1);
      if (!*buf) return def;  /* user just pressed ENTER, so use default */
      ans = atoi(buf);
   } while (!(ans >= min && ans <= max));

   return ans;
}

char *ui_ask_s(const char *prompt, const char *def) {
   static char buf[256];
   ui_prepare();

   if (def) printw("%s [%s]: ", prompt, def);
   else     printw("%s: ", prompt);

   if (!kurses_line_input(buf, 256)) { ui_cancel = -1; return NULL; }

   /* if user didn't type anything and there is a default, return default;
    * otherwise just return whatever it is the user typed (even if it's an
    * empty string) */
   if (!*buf && def) return strdup(def);
   else return strdup(buf);
}

int ui_ask_color(const char *prompt) {
   int i;
   static char hexdigit[] = "0123456789abcdef";
   ui_prepare();
   printw("%s ", prompt);

   /* print the colors */
   for (i = 0; i <= 15; i++) {
      kurses_color(i, i ? 0 : 7);
      printw(" %c ", hexdigit[i]);
   }

   kurses_color(0, 7);
   printw(": ");

   while (1) {
      int ch, color_code;
      ch = getch(); color_code = (ch >= 'a' && ch <= 'f') ? ch - 'a' + 10 : 
                                 (ch >= 'A' && ch <= 'F') ? ch - 'A' + 10 :
                                                            ch - '0';
      if (ch == 27 || ch == 3 || ch == 7) return (ui_cancel = -1);
      if (color_code >= 0 && color_code <= 15) return color_code;
   }
}

int ui_ask_yn(const char *prompt, int defyes) {
   ui_prepare();
   printw("%s [%s] ", prompt, defyes ? "Yes" : "No");
   while (1) {
      int ch = getch();
      switch (ch) {
         case 10:            return defyes;
         
         case 'y': case 'Y': return 1;
         case 'n': case 'N': return 0;
         
         case 27: 
         case 3: 
         case 7: return (ui_cancel = -1);
      }
   }
}

void ui_message(const char *msg, int error) {
   if (error)
      ui_prepare_ex(15, 1);
   else
      ui_prepare();
      
   addstr(msg);
   getch();
}

