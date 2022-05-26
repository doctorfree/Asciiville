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

INFORMA��ES DE LICEN�A:
Este programa � um software de livre distribui��o; voc� pode
redistribu�-lo e/ou modific�-lo sob os termos da GNU General
Public License, conforme publicado pela Free Software Foundation,
pela vers�o 2 da licen�a ou qualquer vers�o posterior.

Este programa � distribu�do na esperan�a de que ele ser� �til
aos seus usu�rios, por�m, SEM QUAISQUER GARANTIAS; sem sequer
a garantia impl�cita de COMERCIABILIDADE ou DE ADEQUA��O A
QUALQUER FINALIDADE ESPEC�FICA. Consulte a GNU General Public
License para obter mais detalhes (uma c�pia acompanha este
programa, armazenada no arquivo COPYING).
*/

#include "util.h"
#define SWAP_VARS(type, x, y) { type tmp; tmp = x; x = y; y = tmp; }

void aewl_util_mvwaddstr(WINDOW *w, int y, int x, int attr1, int attr2, 
                                const char *str) {
   wattrset(w, attr1);
   wmove(w, y, x);

   while (*str) {
      if (*str == '&') {
         str++;
         wattrset(w, attr2);
         waddch(w, *str);
         wattrset(w, attr1);
         str++;
         continue;
      }

      waddch(w, *str);
      str++;
   }
}

int aewl_util_strlen_esc(const char *str) {
   int l = 0;
   while (*str) {
      if (*str == '&') {
         str+=2;
         l++;
         continue;
      }

      str++;
      l++;
   }

   return l;
}

void aewl_util_drawbox(WINDOW *win, int x0, int y0, int w, int h) {
   /* left border */
   aewl_util_drawline_v(win, x0, y0, y0 + h - 1,
                        ACS_ULCORNER, ACS_VLINE, ACS_LLCORNER);
   /* right border */
   aewl_util_drawline_v(win, x0 + w - 1, y0, y0 + h - 1,
                        ACS_URCORNER, ACS_VLINE, ACS_LRCORNER);
   /* top border minus corners */
   aewl_util_drawline_h(win, x0 + 1, x0 + w - 2, y0,
                        ACS_HLINE, ACS_HLINE, ACS_HLINE);
   /* bottom border minus corners */
   aewl_util_drawline_h(win, x0 + 1, x0 + w - 2, y0 + h - 1,
                        ACS_HLINE, ACS_HLINE, ACS_HLINE);
}

void aewl_util_drawline_h(WINDOW *w, int x0, int x1, int y,
                                int lc, int mc, int rc) {
   int x;
   if (x0 > x1) SWAP_VARS(int, x0, x1);

   for (x = x0; x <= x1; x++) {
      wmove(w, y, x);
      if (x == x0)      waddch(w, lc);
      else if (x == x1) waddch(w, rc);
      else              waddch(w, mc);
   }
}

void aewl_util_drawline_v(WINDOW *w, int x, int y0, int y1,
                                int tc, int mc, int bc) {
   int y;
   if (y0 > y1) SWAP_VARS(int, y0, y1);

   for (y = y0; y <= y1; y++) {
      wmove(w, y, x);
      if (y == y0)      waddch(w, tc);
      else if (y == y1) waddch(w, bc);
      else              waddch(w, mc);
   }
}

void aewl_util_addnstr(WINDOW *w, int n, const char *str) {
   while (n-- && *str) waddch(w, *(str++));
}

void aewl_util_erase(WINDOW *win, int x, int y, int w, int h) {
   int j;

   while (h--) {
      j = w;
      wmove(win, y++, x);
      while (j--) waddch(win, ' ');
   }
}


