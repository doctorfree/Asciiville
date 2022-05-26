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
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>

#include "commands.h"
#include "menubar.h"

/* menu colors */
#define MENU_ATT_NORMAL                 0x07
#define MENU_ATT_SEL                    0xF1
#define MENU_ATT_ACCEL_NORMAL           0x17
#define MENU_ATT_ACCEL_SEL              0xE1

#define MAX_ITEMS_PER_MENU 32

typedef struct MenuItem_ { 
   char accel;          /* accelerator (shortcut) char */
   const char *caption; /* text of the item. If text is "", it is a separator
                         * In that case all remaining fields have no meaning */
   int command;         /* the command that this item triggers when
                         * selected - this value will be returned by
                         * menubar_show */
   const char *desc;    /* description of the item, which will appear at
                         * the bottom part of the screen when the item
                         * is highlighted */
} MenuItem;

typedef struct Menu_ {
   const char *caption; /* text */
   int startcol;        /* column at which the left edge of this menu is
                         * placed onscreen */
   int menu_h, menu_w;  /* menu dimensions. If 0, calculated at run-time */

   int item_count;      /* how many items this menu has */
   MenuItem item[MAX_ITEMS_PER_MENU];     /* vector of items */

} Menu;

/* menu data (included as a separate file for better organization) */
#include "menubar-items.inc"
/* defines menu[] as an array of menu's, and menu_count. */

/* menu state */
static int open_menu;  /* which menu is open */
static int menu_sel;   /* which item is currently selected on the 
                        * currently open menu */

/* declaration of internal functions */
static void menubar_paint(); /* paints menu onscreen */
static void menuitem_print(const char *caption, bool sel, int minlen);
static void menu_calc_dims(Menu *m);
static bool chr_icomp(char c1, char c2); /* compares characters,
                                          * case-insensitive */

int menubar_show(int startm) {
   int ch, i;
   int cmd = 0; bool quit = false;
   Menu *omenu;
   menu_sel = 0;

   open_menu = startm;
   if (open_menu < 0) open_menu = 0;
   if (open_menu >= menu_count) open_menu = menu_count - 1;

   push_screen();

   while (!quit) {
      omenu = &menu[open_menu];
      menubar_paint();
      ch = getch();

      switch (ch) {
         case KEY_LEFT:  if (--open_menu < 0) open_menu = menu_count-1;
                         menu_sel = 0;
                         break;
         case KEY_RIGHT: if (++open_menu >= menu_count) open_menu = 0;
                         menu_sel = 0;
                         break;
         case KEY_UP:    
            do {
               if (--menu_sel < 0) menu_sel = omenu->item_count-1;
            } while (!*(omenu->item[menu_sel].caption));
            break;
         case KEY_DOWN:  
            do {
               if (++menu_sel >= omenu->item_count) menu_sel = 0;
            } while (!*(omenu->item[menu_sel].caption));
            break;
         case 10:        quit = true; cmd = omenu->item[menu_sel].command;
                         break;
         case 27:        quit = true; cmd = COMMAND_UNDEFINED;
                         break;
      }

      if (ch > KEY_F0 && ch <= KEY_F(12) && (ch - KEY_F0 - 1) < menu_count) {
         open_menu = ch - KEY_F0 - 1;
         menu_sel = 0;
      }

      /* handle accelerator keys */
      for (i = 0; i < omenu->item_count; i++) {
         if (chr_icomp(omenu->item[i].accel, ch)) {
            /* item selected by accelerator key */
            cmd = omenu->item[i].command;
            quit = true;
            break;
         }
      }
   }

   restore_screen();
   pop_screen();
   refresh();
   return cmd;
}

static bool chr_icomp(char c1, char c2) {
   c1 = (c1 >= 'A' && c1 <= 'Z') ? c1 - 'A' + 'a' : c1;
   c2 = (c2 >= 'A' && c2 <= 'Z') ? c2 - 'A' + 'a' : c2;
   return (c1 == c2);
}

static void menubar_paint() {
   int i;
   Menu *omenu;
   restore_screen();
   /* paint the bar at the top of the screen */
   kurses_color_at(MENU_ATT_NORMAL);
   draw_hline(0, 0, kurses_width(), ' ', ' ', ' ');

   for (i = 0; i < menu_count; i++) {
      kurses_move(menu[i].startcol, 0);
      menuitem_print(menu[i].caption, i == open_menu, -1);
   }
   
   omenu = &menu[open_menu];
   
   /* if open menu's dimensions have not yet been calculated, do so */
   if (!omenu->menu_h) menu_calc_dims(omenu);

   /* paint the open menu */
   kurses_color_at(MENU_ATT_NORMAL);
   draw_window(omenu->startcol, 1, omenu->menu_w + 2, omenu->menu_h + 2, "");
   for (i = 0; i < omenu->item_count; i++) {
      kurses_move(omenu->startcol+1, i + 2);
      menuitem_print(omenu->item[i].caption, i == menu_sel, omenu->menu_w);
   }

   /* paint the description line */
   kurses_move(0, kurses_height()-1);
   kurses_color_at(MENU_ATT_NORMAL);
   i = strlen(omenu->item[menu_sel].desc);
   addstr(omenu->item[menu_sel].desc);
   while (i++ < kurses_width()) addch(' ');

   kurses_move(kurses_width()-1, kurses_height()-1);
   refresh();
}

static void menuitem_print(const char *s, bool sel, int minlen) {
   bool next_is_accel = false;
   int printed = 0;
   while (*s || (printed < minlen)) {
      if (*s == '&') {
         next_is_accel = true;
         s++;
         continue;
      }

      kurses_color_at(sel ? 
          ( next_is_accel ? MENU_ATT_ACCEL_SEL : MENU_ATT_SEL ) :
          ( next_is_accel ? MENU_ATT_ACCEL_NORMAL : MENU_ATT_NORMAL ));
     
      addch(*s ? *s : ' ');
      printed++;

      next_is_accel = false;
      if (*s) s++;
   }
}

static void menu_calc_dims(Menu *m) {
   int i;
   int width = 1, len = 0;

   for (i = 0; i < m->item_count; i++) {
      len = strlen(m->item[i].caption);
      if (len > width) width = len;
   }
   
   m->menu_h = m->item_count;
   m->menu_w = width;
}

