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

#ifndef _btco_aewl_util_h
#define _btco_aewl_util_h

#include <ncurses.h>

/* Prints a string of text on the given window at the given position,
 * interpreting the '&' escape character as meaning "set the next character's
 * attribute to attr2. Rest of string is printed in attr1 */
void aewl_util_mvwaddstr(WINDOW *w, int y, int x, int attr1, int attr2, 
                         const char *str);

/* Calculates the printed length of the given string, considering the
 * '&' escape character. So the printed length of "&Abc" is 3, although
 * strlen() would return 4. */
int aewl_util_strlen_esc(const char *str);

/* Clears the given rectangle by overwriting it with spaces*/
void aewl_util_erase(WINDOW *win, int x, int y, int w, int h);

/* Draws a box on the given window with the given coordinates. x, y are the
 * top-left, h,w are the dimensions. */
void aewl_util_drawbox(WINDOW *win, int x, int y, int w, int h);

/* Draws a horizontal line in the given window, from (x0, y) to (x1, y).
 * The leftmost character will be lc, the middle will be filled with mc
 * characters, and the rightmost character will be rc */
void aewl_util_drawline_h(WINDOW *w, int x0, int x1, int y,
                                int lc, int mc, int rc);

/* Draws a vertical line in the given window, from (x, y0) to (x, y1).
 * The topmost character will be tc, the middle will be filled with mc
 * characters, and the bottommost character will be bc */
void aewl_util_drawline_v(WINDOW *w, int x, int y0, int y1,
                                int tc, int mc, int bc);


/* Prints the given string as in addstr(), except that no more than n
 * characters will be printed */
void aewl_util_addnstr(WINDOW *w, int n, const char *str);


#endif

