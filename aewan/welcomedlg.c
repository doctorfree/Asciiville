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
                                                                                
#include "welcomedlg.h"
#include "psd.h"

#define LOGO_WIDTH 11
#define LOGO_HEIGHT 5

static char* logo[] = {
   "79         ",
   "||         ",
   "46799 97979",
   "||43|||||||",
   "311-1231231",
   NULL
};

static int logo_colors[LOGO_HEIGHT] = { 4, 6, 7, 6, 4 };

void show_welcome_dlg(void) {
   int x0, y0, x, y, ch, *clr;
   char **p, *q;
   int acschars[10] = {
      ACS_CKBOARD,      /* 0 */
      ACS_LLCORNER,     /* 1 */
      ACS_BTEE,         /* 2 */ 
      ACS_LRCORNER,     /* 3 */
      ACS_LTEE,         /* 4 */
      ACS_PLUS,         /* 5 */
      ACS_RTEE,         /* 6 */
      ACS_ULCORNER,     /* 7 */
      ACS_TTEE,         /* 8 */
      ACS_URCORNER      /* 9 */
   };

   kurses_color(4, 0);
   draw_centered_window(46, 18, "Welcome to AEWAN", &x0, &y0);
   y = y0 + 1;

   /* draw logo */
   for (p = logo, clr = logo_colors; *p; p++, clr++) {
      kurses_color(*clr, 0);
      kurses_move(x0, y++);
      for (q = *p; *q; q++) {
         if (*q >= '0' && *q <= '9') ch = acschars[*q - '0'];
         else if (*q == '-')         ch = ACS_HLINE;
         else if (*q == '|')         ch = ACS_VLINE;
         else                        ch = ' ';
         addch(ch);
      }
   }

   /* draw program name next to logo */
   x = x0 + LOGO_WIDTH + 2;
   y = y0 + 1;
   
   kurses_move(x, y++);
   kurses_color(4+8, 0);
   addstr("AEWAN ascii art editor");

   kurses_move(x, y++);
   kurses_color(7, 0);
   addstr("Copyright (c) 2003");

   kurses_move(x, y++);
   addstr("Bruno T. C. de Oliveira");

   kurses_move(x, y++);
   addstr("All rights reserved.");
   
   y+=2; x = x0 + 1;
   kurses_move(x, y++);
   kurses_color(4+8, 0);
   addstr("Program version: ");
   kurses_color(7, 0);
   addstr(AEWAN_PROGRAM_VERSION " (" AEWAN_PROGRAM_VERSION_NAME ")");

   y++;
   kurses_move(x, y++);
   addstr("This program is licensed under the GNU");

   kurses_move(x, y++);
   addstr("General Public License. Please refer to");

   kurses_move(x, y++);
   addstr("the COPYING file for more information.");

   y++;
   kurses_move(x, y++);
   addstr("Press RETURN to continue. When in the");
   kurses_move(x, y++);
   addstr("program, press <F1> to display a menu.");
   
   getch();
}

