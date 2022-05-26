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

#include "kurses.h"
#include "util.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* mapping between color codes and curses pairs and attributes:
 *
 * color pair depends on the three least significant bits of the foreground
 * and background. If a pair is (fg, bg),
 *      let FG = fg % 8 (i.e. the three least significant bits)
 *      let BG = bg % 8
 *
 * the curses pair for it will be BG * 8 + (7 - FG)
 *
 * Valid curses pairs are 1..63, but the color (7, 0) maps to pair number 0.
 * This is no problem, because (7, 0) is the default color so it needs
 * no curses color pair.
 *
 * After the color pair is selected, the most significant bit of
 * fg and bg are used to activate A_BOLD and A_BLINK attributes.
 */

/* --------- internal data and declarations of private functions ---------- */

/* current foreground and background colors */
static int cur_fg, cur_bg;

/* Given a pair of color codes, returns the corresponding curses pair */
static int kurses_pair_for(int fg, int bg);

/* node type for the saved screen linked list (see below) */
struct SavedScreenNode_ {
   WINDOW *win;  /* the curses window that holds the screenshot */
   struct SavedScreenNode_* next;
};
typedef struct SavedScreenNode_ SavedScreenNode;

/* Stack of saved screens, implemented as a linked list */
SavedScreenNode *sss_top; /* sss = 'saved screen stack' */

/* -------------- implementation of public functions ---------------------- */

void kurses_init() {
   int fg, bg;
   
   initscr();               /* enter screen-oriented mode */
   raw();                   /* don't treat Ctrl+C, etc. as special. */
   noecho();                /* don't echo typed characters back to terminal */
   keypad(stdscr, TRUE);    /* convert ugly escape sequences to more
                             * palatable curses KEY_* codes */
   
   cur_fg = 7;
   cur_bg = 0;
   sss_top = NULL;

   if (!has_colors()) {
      fprintf(stderr, "*** Fatal error: Terminal has no color support.\n");
      exit(1);
   }

   start_color(); 

   /* initialize all curses pairs */
   for (fg = 0; fg < 8; fg++) for (bg = 0; bg < 8; bg++) {
      int pair = kurses_pair_for(fg, bg);
      if (!pair) continue; /* don't try to initialize pair 0,
                            * that corresponds to color (7, 0). */
      
      init_pair(pair, fg, bg);
   }
   
   clear();
   refresh();
}

void kurses_finalize() {
   clear();
   refresh();
   endwin();
}

void kurses_color(int fg, int bg) {
   int pair;
   
   if (fg < 0) fg = cur_fg;
   if (bg < 0) bg = cur_bg;

   /* see what color pair corresponds to fg, bg */
   pair = kurses_pair_for(fg, bg);
   
   /* if that maps to 0, just reset attribute; otherwise set color pair */
   if (pair) attrset(COLOR_PAIR(pair));
   else      attrset(A_NORMAL);
   
   /* now turn on A_BOLD and A_BLINK attribute as indicated by the most
    * significant bit of fg and bg, respectively */
   if (fg & 0x08) attron(A_BOLD);
   if (bg & 0x08) attron(A_BLINK);
}

/* Same as kurses_color(attr >> 4, attr & 0x0F) */
void kurses_color_at(int attr) {
   kurses_color(attr >> 4, attr & 0x0F);
}

int kurses_width() {
   int maxy, maxx;
   getmaxyx(stdscr, maxy, maxx);
   return maxx;
}

int kurses_height() {
   int maxy, maxx;
   getmaxyx(stdscr, maxy, maxx);
   return maxy;
}

int kurses_pos_valid(int x, int y) {
   return (x >= 0 && x < kurses_width() &&
           y >= 0 && y < kurses_height());
}

int kurses_move(int x, int y) {
   if (!kurses_pos_valid(x, y)) return 0;
   move(y, x);
   return 1;
}

int kurses_line_input(char *buf, int buf_size) {
   return kurses_line_input_ex(buf, buf_size, NULL, 0);
}

int kurses_line_input_ex(char *buf, int buf_size, int *skeys, int flags) {
   int pos = 0;
   int ch;
   const int *p;
   
   while (1) {
      refresh();
      ch = getch();
      if (ch < 0) continue;

      /* first check if character is in skeys */
      if ( (p = skeys) ) while (*p) if (*(p++) == ch) return -ch;

      /* proceed to normal handling */
      switch (ch) {
         case 10: buf[pos] = 0; return 1; /* confirm input */
         case 3: 
         case 7: 
         case 27: *buf = 0; return 0; /* cancel input */
         case KEY_BACKSPACE: case 8: case 127: /* backspace */
            if (pos > 0) { --pos; addch('\b'); addch(' '); addch('\b'); } 
            break;
         case 'U' - 0x40: /* kill */
            while (pos--) { addch('\b'); addch(' '); addch('\b'); } 
            pos = 0;
            break;
         default:
            if (pos <= buf_size - 2 && printable_char(ch)) {
               /* buffer not full (remember to reserve a byte for the NULL),
                * and character is not a control character: append it */
               buf[pos++] = ch;
               addch((flags & KURSES_LI_HIDE) ? '*' : ch);
            }
      }
   }
}

void draw_window(int x0, int y0, int w, int h, const char *title) {
   int x, y, x1, y1;
   int gr_ch[3][3];
   
   gr_ch[0][0]=ACS_ULCORNER, gr_ch[0][1]=ACS_HLINE, gr_ch[0][2]=ACS_URCORNER;
   gr_ch[1][0]=ACS_VLINE,    gr_ch[1][1]=' ',       gr_ch[1][2]=ACS_VLINE;
   gr_ch[2][0]=ACS_LLCORNER, gr_ch[2][1]=ACS_HLINE, gr_ch[2][2]=ACS_LRCORNER;
   
   x1 = x0 + w - 1;
   y1 = y0 + h - 1;

   for (y = y0; y < y0 + h; y++)
      for (x = x0; x < x0 + w; x++)
         if (!kurses_move(x, y)) continue;
         else addch(gr_ch [(y == y0) ? 0 : (y == y1) ? 2 : 1]
                          [(x == x0) ? 0 : (x == x1) ? 2 : 1]);

   if (title) if (kurses_move(x0 + w/2 - strlen(title) / 2, y0)) addstr(title);
   kurses_move(x0 + 1, y0 + 1);
}

void draw_centered_window(int w, int h, const char *title, int *x, int *y) {
   int x0 = (kurses_width() - w) / 2;
   int y0 = (kurses_height() - h) / 2;
   if (x) *x = x0 + 1;
   if (y) *y = y0 + 1;

   draw_window(x0, y0, w, h, title);
}

void draw_hline(int x0, int y0, int w, 
                int left_endpt, int interim, int right_endpt)
{
   int x;
   if (!kurses_move(x0, y0)) return;
   for (x = x0; x < x0 + w; x++)
      addch( (x == x0) ? left_endpt : 
             (x == x0 + w - 1) ? right_endpt : interim);
}

void draw_vline(int x0, int y0, int h,
                int upper_endpt, int interim, int lower_endpt) {
   int y;
   if (!kurses_move(x0, y0)) return;
   for (y = y0; y < y0 + h; y++) {
      kurses_move(x0, y);
      addch( (y == y0) ? upper_endpt :
             (y == y0 + h - 1) ? lower_endpt : interim);
   }
}

void push_screen(void) {
   SavedScreenNode *n = zalloc(sizeof(SavedScreenNode));
   n->win = newwin(0,0,0,0);
   overwrite(stdscr, n->win);
   n->next = sss_top;

   sss_top = n;
}

void restore_screen(void) {
   if (!sss_top) {
      /* if there are no saved screens, just clear the screen and return.
       * Doesn't make much sense, but beats doing nothing or segfaulting. */
      erase();
      return;
   }

   overwrite(sss_top->win, stdscr);
   touchwin(stdscr);
}

void pop_screen(void) {
   SavedScreenNode *newtop;
   if (!sss_top) return;

   newtop = sss_top->next;
   free(sss_top);
   sss_top = newtop;
}

/* -------------- implementation of private functions --------------------- */
static int kurses_pair_for(int fg, int bg) {
   fg &= 0x07; bg &= 0x07;   /* only keep the three least significant bits */
   return (bg << 3) + (7 - fg);
}



