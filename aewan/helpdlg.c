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

#include "helpdlg.h"

static char *helptext[2] = {
/* 345678901234567890123456789012345 */
"\001Legend\002\n"
"^ means the Ctrl key and % means\n"
"the Alt key. Thus, ^A means \n"
"Ctrl+A and %A means Alt-A.\n"
"\n"
"\001General keys\002\n"
"^X,^Z  fast horizontal movement\n"
"^O     open a file\n"
"^S     save file\n"
"\n"
"\001Layers\002\n"
"^W     go to next layer\n"
"^Q     go to previous layer\n"
"^L     bring up Layer Manager\n"
"^A     add layer (defaults)\n"
"%A     add layer (specify)\n"
"^D     add copy of current layer\n"
"^R     rename this layer\n"
"%R     resize this layer\n"
"\n"
"\001Selection\002\n"
"%S     enter select mode.\n"
,
"\001Colors\002\n"
"TAB    bring up color dialog\n"
"%P     pick color from cell\n"
"       under the cursor\n"
"%Z     apply color to cell\n"
"       under the cursor\n"
"\n"
"\001Mode toggles\002\n"
"INS    toggle insert mode\n"
"^G     toggle line-drawing mode\n"
"       (0-9,|,- will draw lines)\n"
"^P     toggle composite view\n"
"\n"
"\001Others\002\n"
"^E     edit document metainfo\n"
"^C     quit program\n"
"F1-Fn  show menu\n"
"\n*************************************\n" /* will become ACS_HLINE */
"Copyright (c) 2003 by\n"
"Bruno T. C. de Oliveira\n"
"All rights reserved."
};

void show_help_dlg(void) {
   int x0, y0; int x, y; int i;
   const char *p;
   
   clear();
   refresh();
   kurses_color(4, 0);
   addstr("Keys Quick Reference\n\n");
   kurses_color(7, 0);

   x0 = 0;
   y0 = 2;
   
   x = x0;
   y = y0;
   for (i = 0; i < 2; i++) {
      x = x0 + i * 40;
      y = y0;
      p = helptext[i];
      
      while (*p) {
         kurses_move(x, y);
         switch (*p) {
            case 1: kurses_color(4, 0); break;
            case 2: kurses_color(7, 0); break;
            case '\n': x = x0 + i * 40; y++; break;
            case '*': addch(ACS_HLINE); x++; break;
            default: addch(*p); x++; 
         }

         p++;
      }
   }

   /* draw vertical line separating the two columns */
   draw_vline(x0 + 37, y0, 24 - y0, ACS_VLINE, ACS_VLINE, ACS_VLINE);
   
   refresh();
   getch();
}

