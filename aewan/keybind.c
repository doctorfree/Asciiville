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

#include <stdlib.h>
#include <ncurses.h>
#include "bores/bores.h"

#include "keybind.h"
#include "keys.h"
#include "commands.h"

/* This array represents the current key map: cmdforkey[key] is
 * the command the key is bound to (0 if the key is not bound) */
#define KEYCODE_MAX 512
static int *cmdforkey;

void keybind_init(void) {
   /* allocate it (remember that zalloc() already zeroes it out) */
   cmdforkey = zalloc(sizeof(int) * KEYCODE_MAX);

   /* assign defaults */
   cmdforkey[KEY_CTRL_C]       = COMMAND_QUIT;
   cmdforkey[KEY_CTRL_A]       = COMMAND_ADD_LAYER_DEFAULTS;
   cmdforkey[KEY_ALT_A]        = COMMAND_ADD_LAYER_SPECIFY;
   cmdforkey[KEY_ALT_F]        = COMMAND_SET_FOREGROUND;
   cmdforkey[KEY_ALT_B]        = COMMAND_SET_BACKGROUND;
   cmdforkey[KEY_CTRL_I]       = COMMAND_SHOW_COLOR_DLG;
   cmdforkey[KEY_CTRL_O]       = COMMAND_LOAD_FILE;
   cmdforkey[KEY_CTRL_S]       = COMMAND_SAVE_FILE;
   cmdforkey[KEY_CTRL_E]       = COMMAND_EDIT_META;
   cmdforkey[KEY_LEFT]         = COMMAND_MOVE_LEFT;
   cmdforkey[KEY_UP]           = COMMAND_MOVE_UP;
   cmdforkey[KEY_DOWN]         = COMMAND_MOVE_DOWN;
   cmdforkey[KEY_RIGHT]        = COMMAND_MOVE_RIGHT;
   cmdforkey[10]               = COMMAND_CARRIAGE_RETURN;
   cmdforkey[KEY_NPAGE]        = COMMAND_PAGE_DOWN;
   cmdforkey[KEY_PPAGE]        = COMMAND_PAGE_UP;
   cmdforkey[KEY_HOME]         = COMMAND_START_OF_LINE;
   cmdforkey[KEY_END]          = COMMAND_END_OF_LINE;
   cmdforkey[KEY_CTRL_Z]       = COMMAND_FAST_LEFT;
   cmdforkey[KEY_CTRL_X]       = COMMAND_FAST_RIGHT;
   cmdforkey[KEY_ALT_P]        = COMMAND_PICK_COLOR;
   cmdforkey[KEY_ALT_Z]        = COMMAND_TINT_CELL;
   cmdforkey[KEY_ALT_S]        = COMMAND_TOGGLE_SELECTION;
   cmdforkey[27]               = COMMAND_CANCEL;
   cmdforkey[KEY_CTRL_L]       = COMMAND_SHOW_LAYER_DLG;
   cmdforkey[KEY_CTRL_W]       = COMMAND_NEXT_LAYER;
   cmdforkey[KEY_CTRL_Q]       = COMMAND_PREV_LAYER;
   cmdforkey[KEY_CTRL_D]       = COMMAND_DUP_LAYER;
   cmdforkey[KEY_CTRL_R]       = COMMAND_RENAME_LAYER;
   cmdforkey[KEY_CTRL_G]       = COMMAND_TOGGLE_LINE_MODE;
   cmdforkey[KEY_CTRL_P]       = COMMAND_TOGGLE_COMPOSITE;
   cmdforkey[KEY_IC]           = COMMAND_TOGGLE_INSERT;
   cmdforkey[KEY_ALT_R]        = COMMAND_RESIZE_LAYER;
}

int keybind_translate(int key) {
   /* do special 'untranslation' of keys for the cases when curses
    * is a little _too_ helpful and gives us key codes that are much
    * more abstract than we need. */
   if (key == KEY_SUSPEND) key = KEY_CTRL_Z;

   /* now translate */
   return cmdforkey[key];
}


