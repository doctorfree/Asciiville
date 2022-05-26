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


#include <stdbool.h>
#include "bores/bores.h"
#include <limits.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <errno.h>
#include <pwd.h> /* For getpwuid() */
#include <unistd.h>

#include "handlekey.h"
#include "document.h"
#include "psd.h"
#include "ui.h"
#include "keys.h"
#include "clipboard.h"
#include "colordlg.h"
#include "layerdlg.h"
#include "helpdlg.h"
#include "chtr.h"
#include "editmeta.h"
#include "keybind.h"
#include "commands.h"
#include "menubar.h"
#include "welcomedlg.h"
#include "filedlg.h"

#define FAST_MOVE_AMOUNT 8  /* how much the cursor moves each time the fast
                             * movent keys (Ctrl+Z, Ctrl+X) are pressed */

/* Interacts with the user in order to allow him to add a new layer to the 
 * document. If <interactive>, asks user for parameters; else just
 * adds using the defaults. */
static void u_add_layer(bool interactive) {
   int width, height; int nom_width, nom_height;
   char *name; Layer *lyr;

   document_get_nom_dim(_doc, &nom_width, &nom_height);
   if (interactive) {
      width = ui_ask_i("NEW LAYER: Enter width", 
                        nom_width, 2, INT_MAX);  if (ui_cancel) return;
      height = ui_ask_i("NEW LAYER: Enter height", 
                        nom_height, 2, INT_MAX); if (ui_cancel) return;
      name = ui_ask_s("NEW LAYER: Name", "unnamed"); if (ui_cancel) return;
   }
   else {
      width = nom_width;
      height = nom_height;
      name = strdup("unnamed");
   }

   lyr =  layer_create(name, width, height);

   /* if this is the first layer we are adding, make it opaque;
    * otherwise make it transparent */
   if (_doc->layer_count) lyr->transp = true;
   else                   lyr->transp = false;

   document_insert_layer(_doc, 0, lyr);
   
   free(name);
}

/* Interacts with the user in order to allow him to rename the current
 * layer */
static void u_rename_layer(void) {
   Layer *l = _doc->layers[_lyr];
   char *new_name;
   
   new_name = ui_ask_s("Rename layer to", l->name);
   if (!ui_cancel)
      dstrset(&l->name, new_name);

   if (new_name) free(new_name);
}

/* Interacts with the user in order to allow him to resize the current
 * layer */
static void u_resize_layer(void) {
   Layer *l = _doc->layers[_lyr];
   Layer *new_l;
   int h, w, x, y;
   
   w = ui_ask_i("RESIZE LAYER: Enter new width", 
                     l->width,  2, INT_MAX); if (ui_cancel) return;
   h = ui_ask_i("RESIZE LAYER: Enter new height", 
                     l->height, 2, INT_MAX); if (ui_cancel) return;

   new_l = layer_create(l->name, w, h);

   w = l->width  > new_l->width  ? new_l->width  : l->width;
   h = l->height > new_l->height ? new_l->height : l->height;

   for (y = 0; y < h; y++)
      for (x = 0; x < w; x++) {
         new_l->cells[x][y].ch   = l->cells[x][y].ch;
         new_l->cells[x][y].attr = l->cells[x][y].attr;
      }

   layer_destroy(l);
   _doc->layers[_lyr] = new_l;
}

/* Duplicates current layer, adding a duplicate to the end of the
 * layer stack. */
static void u_dup_layer(void) {
   char buf[32];
   Layer *l;
   sprintf(buf, "copy of %d", _lyr);

   l = _doc->layers[_lyr];
   document_add_layer(_doc, layer_dup(buf, l));
   switch_to_layer(_doc->layer_count - 1);
}

/* Return home directory of specified user name.
 * If name is omitted, returns current user's home dir. */
char *get_home_dir(char *name) {
   char msg[128];
   char *home_dir;
   struct passwd *pw;
   
   if(name) {
      pw = getpwnam(name);
      if(!pw)
         goto doesnt_have;
      return pw->pw_dir;
   } else {
      home_dir = getenv("HOME");
      if(home_dir) {
         return home_dir;
      } else {
         /* $HOME isn't set. Get directory from /etc/passwd. */
         pw = getpwuid(getuid());
         if(!pw)
            goto doesnt_have;
         return pw->pw_dir;
      }
   }

   /* Only reached with goto. */
doesnt_have:
   sprintf(msg, "ERROR: User '%s' doesn't have a home directory.", name);
   ui_message(msg, UIMSG_ERROR);
   return NULL;
}

/* Returns filename with '~' replaced with the user's
 * home directory. */
static char *expand_tilde(char *filename) {
   char *home_dir;
   char *slash;
   
   if(filename[1]) {
      char *username, *filename_dup;
      char *ret;
      
      filename_dup = strdup(filename);
      slash = strchr(filename_dup, '/');
      username = filename_dup + 1;
      
      if(!slash) {
         /* Only "~user" was given, use it as a filename. */
         free(filename_dup);
         return filename;
      }
      
      *slash = '\0';
      
      if(!strlen(username))
         username = NULL;
      home_dir = get_home_dir(username);
      if(!home_dir)
         return NULL;
      
      slash++; /* Everything after the tilde and possible username. */
      ret = malloc(strlen(home_dir) + strlen(slash) + 1);
      sprintf(ret, "%s/%s", home_dir, slash);
      free(filename_dup);
      return ret;
   } else {
      /* Only a tilde was given, use it as a filename. */
      return filename;
   }
}

/* Interacts with the user in order to allow him to load a different
 * file into the editor. If the <filename> argument is not NULL,
 * this routine will not ask the user for the filename but will
 * rather use the supplied filename as the file to load. */
void u_load_file(const char *supplied_filename) {
   char *filename;
   FILE *f = NULL;
   char msg[128];
   Document *newdoc;

   if (!supplied_filename) {
      filename = filedlg_show("Load File");
      if (!filename) return;
   }
   else filename = strdup(supplied_filename);

   if ( !(newdoc = document_load_from(filename)) ) {
      sprintf(msg, "ERROR: Failed to load file.  -- More --");
      ui_message(msg, UIMSG_ERROR);
      ui_message(aeff_get_error(), UIMSG_ERROR);
      goto cleanup;
   }

   /* now that we have the new document, do the actual replacement */
   document_destroy(_doc);
   _doc = newdoc;

   /* set the filename of the loaded file */
   dstrset(&_filename, filename);

cleanup:
   if (f) fclose(f);
   if (filename) free(filename);
}

/* Interacts with the user allowing him to save current file */
static void u_save_file(bool forceAsk) {
   char *filename, *expanded_filename;
   FILE *tmpf;
   AeFile *f = 0;
   char msg[128];

   if (forceAsk || !_filename || !strlen(_filename)) {
      filename = filedlg_show("Save File As");
      if (!filename) return;
   }
   else filename = strdup(_filename);

   if (filename[0] == '~') {
      expanded_filename = expand_tilde(filename);
      if (!expanded_filename) return;

      /* Comparing pointers. */
      if (expanded_filename != filename) {
         free(filename);
         filename = expanded_filename;
      }
   }

   if ( _filename && strcmp(_filename, filename) && 
                           (tmpf = fopen(filename, "r")) ) {
      /* file exists. Ask if user wants to overwrite */
      fclose(tmpf); tmpf = 0;
      if (!ui_ask_yn("FILE EXISTS. Overwrite?", 0) || ui_cancel) {
         ui_message("File was NOT saved!", UIMSG_INFORM);
         goto cleanup;
      }
   }

   if ( !(f = aeff_open(filename, 'w')) ) {
      sprintf(msg, "ERROR: Can't write to file. %s.", aeff_get_error());
      ui_message(msg, UIMSG_ERROR);
      goto cleanup;
   }

   document_save(_doc, f);

   dstrset(&_filename, filename);

   sprintf(msg, "File saved: %s.", filename);
   ui_message(msg, UIMSG_INFORM);

cleanup:
   if (f) aeff_close(f);
   if (filename) free(filename);
}

/* erases current selection */
static void erase_sel() {
   int x, y;
   int x0, x1, y0, y1;

   get_norm_sel(&x0, &y0, &x1, &y1);
   for (x = x0; x <= x1; x++)
      for (y = y0; y <= y1; y++)
         _doc->layers[_lyr]->cells[x][y] = BLANK_CELL;
}

/* if <fg>, sets foreground color of selection to current foreground
 * color. Else, sets background color of selection to current background. */
static void tint_sel(bool fg) {
   int x0, y0, x1, y1, x, y;
   get_norm_sel(&x0, &y0, &x1, &y1);
   
   for (x = x0; x <= x1; x++)
      for (y = y0; y <= y1; y++)
         if (fg)
            _doc->layers[_lyr]->cells[x][y].attr =
               (_fg << 4) | (_doc->layers[_lyr]->cells[x][y].attr & 0x0F);
         else
            _doc->layers[_lyr]->cells[x][y].attr =
                      _bg | (_doc->layers[_lyr]->cells[x][y].attr & 0xF0);
}

/* Remaps character ch to its lgmode equivalent. When in _lgmode == true,
 * this function is executed for every character typed. */
static int lgmode_map_ch(int ch) {
   switch (ch) {
      case '7': return AEWAN_CHAR_ULCORNER;
      case '8': return AEWAN_CHAR_TTEE;
      case '9': return AEWAN_CHAR_URCORNER;
      case '4': return AEWAN_CHAR_LTEE;
      case '5': return AEWAN_CHAR_PLUS;
      case '6': return AEWAN_CHAR_RTEE;
      case '1': return AEWAN_CHAR_LLCORNER;
      case '2': return AEWAN_CHAR_BTEE;
      case '3': return AEWAN_CHAR_LRCORNER;
      case '-': return AEWAN_CHAR_HLINE;
      case '|': return AEWAN_CHAR_VLINE;
      case '0': return AEWAN_CHAR_CKBOARD; 
      default : return ch;
   }
}

/* handle special keys for SM_SELECT mode */
static void sm_select_handle_key(int ch) {
   int x0, y0, x1, y1;
   int tmp;
   get_norm_sel(&x0, &y0, &x1, &y1);
   
   /* we are in SM_SELECT mode */
   switch (ch) {
      case 'm':
         _selmode = SM_FLOAT;
         copy_sel_to_clipboard();
         erase_sel();
         _x = x0; _y = y0;
         break;
      case 'c':
         _selmode = SM_FLOAT;
         copy_sel_to_clipboard();
         _x = x0; _y = y0;
         break;
      case 'e':
         _selmode = SM_NONE;
         erase_sel();
         break;
      case 'f':
         tint_sel(true);
         break;
      case 'b':
         tint_sel(false);
         break;
      case 'o':
         /* swap the anchor with the cursor position *
          * that will enable the user to manipulate the "other edge"
          * of the rectangle */
         tmp = _x; _x = _ax; _ax = tmp; 
         tmp = _y; _y = _ay; _ay = tmp; 
         break;
   }
}

/* handle special keys for SM_FLOAT mode */
static void sm_float_handle_key(int ch) {
   switch (ch) {
      case 's':  /* stamp */
         paste_clipboard();
         break;
      case 'x': case 'X': /* flip x */
         layer_flip_x(_clipboard, ch == 'x');
         break;
      case 'y': case 'Y': /* flip y */
         layer_flip_y(_clipboard, ch == 'y');
         break;
      case 't': case 'T': /* toggle transparency */
         _clipboard->transp = !_clipboard->transp;
         break;
   }
}

/* Warns the user that he will be taken into an editor to edit the
 * document's meta-info, then fires up the editor */
void u_edit_meta(void) {
   int x0, y0, x, y, ch;
   kurses_color(7, 0);
   draw_centered_window(40, 5, "Edit Metainfo", &x0, &y0);
   
   x = x0, y = y0;
   kurses_move(x, y++);
   addstr("You will now be taken into an editor");
   kurses_move(x, y++);
   addstr("in which you will be able to edit this");
   kurses_move(x, y++);
   addstr("document's metainfo (arbitrary text).");

   ch = getch();
   if (ch == 7 || ch == 27) return;

   edit_metainfo();
}

static void correct_coords(void) {
   Layer *lyr;
   int scr_width  = kurses_width();
   int scr_height = kurses_height();

   if (!_doc->layer_count) return;
   lyr = _doc->layers[_lyr];

   /* clamp coordinates to bounds */
   if (_x < 0) _x = 0;
   if (_x >= lyr->width) _x = lyr->width - 1;
   if (_y < 0) _y = 0;
   if (_y >= lyr->height) _y = lyr->height - 1;

   /* now correct scrolling */
   if      (_x >= _svx + scr_width) _svx = _x - scr_width + 1;
   else if (_x < _svx) _svx = _x;
   if      (_y >= _svy + scr_height - 1) _svy = _y - scr_height + 2;
   else if (_y < _svy) _svy = _y;
}


void handle_key(int ch) {
   int command;
   Layer *lyr;

   /* ENABLE THIS TO DEBUG AEWL... */
   #if 0
      if (ch == KEY_F(10)) {
         debug_aewl();
         return;
      }
   #endif

   /* first check for function keys (they open the menu) */
   if (ch > KEY_F0 && ch <= KEY_F(12)) {
      /* show menu and perform command mandated by it */
      command = menubar_show(ch - KEY_F0 - 1);
      if (command != COMMAND_UNDEFINED) handle_command(command);
      return;
   }

   if ( (command = keybind_translate(ch)) ) {
      /* key triggers a command: execute command */
      handle_command(command);
      if (ch != 27) return; /* we also want to handle 27 (escape) ourselves
                             * below */
   }

   /* key does not trigger a command; handle it normally if there are
    * layers in the document; ignore it if there aren't */
   if (!_doc->layer_count) return;
   lyr = _doc->layers[_lyr];

   /* interpret character keys for no-selection mode */
   if (printable_char(ch) && _selmode == SM_NONE) {
      int ch_to_put = _lgmode ? lgmode_map_ch(ch) : ch;

      if (_insmode) {
         int x;
         for (x = lyr->width - 1; x > _x; x--)
            lyr->cells[x][_y] = lyr->cells[x-1][_y];
      }
      
      lyr->cells[_x][_y].ch = ch_to_put;
      lyr->cells[_x][_y].attr = _fg << 4 | _bg;
      _x++;
   }

   /* interpret character keys for SM_SELECT and SM_FLOAT modes */
   if (_selmode == SM_SELECT) sm_select_handle_key(ch);
   else if (_selmode == SM_FLOAT) sm_float_handle_key(ch);

   /* interpret the backspace and delete keys, also for no-sel mode  */
   if (_selmode == SM_NONE && 
            ((_x > 0 && (ch == 8 || ch == KEY_BACKSPACE || ch == 127)) 
                                                        || ch == KEY_DC)) {
      int x;
      _selmode = false;
      if (ch != KEY_DC) _x--; /* we know this can't make _x negative, the
                               * condition on the "if" above guarantees that */
      for (x = _x; x < lyr->width - 1; x++)
         lyr->cells[x][_y] = lyr->cells[x+1][_y];
      
      lyr->cells[lyr->width - 1][_y] = BLANK_CELL;
   }

   correct_coords();
};

void handle_command(int command) {
   int newfg, newbg;
   int ret;
   Layer *lyr;

   /* handle commands that work regardless of whether document is empty */
   switch (command) {
      case COMMAND_QUIT: 
         ret = ui_ask_yn("Really quit aewan?", 0);
         if (!ui_cancel && ret) exit(0);
         break;
      case COMMAND_ADD_LAYER_DEFAULTS:
      case COMMAND_ADD_LAYER_SPECIFY:
         u_add_layer(command == COMMAND_ADD_LAYER_SPECIFY);
         if (!doc_empty()) _lyr = 0;  /* now we have a layer, so correct _lyr
                                       * so that it makes sense */
         return;
      case COMMAND_SET_FOREGROUND:
         newfg = ui_ask_color("Foreground color"); 
         if (!ui_cancel) _fg = newfg;
         return;
      case COMMAND_SET_BACKGROUND:
         newbg = ui_ask_color("Background color");
         if (!ui_cancel) _bg = newbg;
         return;
      case COMMAND_SHOW_COLOR_DLG: show_color_dlg(); return;
      case COMMAND_TOGGLE_INSERT:  _insmode = !_insmode; return;
      case COMMAND_SHOW_HELP_DLG:  show_help_dlg(); return;
      case COMMAND_SHOW_ABOUT_DLG: show_welcome_dlg(); return;
      case COMMAND_LOAD_FILE: u_load_file(NULL); return;
      case COMMAND_SAVE_FILE: u_save_file(false); return;
      case COMMAND_SAVE_FILE_AS: u_save_file(true); return;
      case COMMAND_NEW_FILE:
         if (_doc->layer_count > 0) {
            ret = ui_ask_yn("Really start a new document?", 0);
            if (ui_cancel || !ret) break;
         }
         document_destroy(_doc);
         zero_state();
         return;
      case COMMAND_EDIT_META: u_edit_meta(); return;
   }

   /* from this point on, we handle the keys that only work if the document
    * is not empty */
   if (doc_empty()) return;
   lyr = _doc->layers[_lyr];
  
   /* if clipboard exists and we are not in SM_FLOAT mode, delete it */
   if (_clipboard && _selmode != SM_FLOAT) clear_clipboard();
  
   switch (command) {
      case COMMAND_MOVE_LEFT:  _x--; break;
      case COMMAND_MOVE_RIGHT: _x++; break;
      case COMMAND_MOVE_UP:    _y--; break;
      case COMMAND_MOVE_DOWN:  _y++; break;
      case COMMAND_CARRIAGE_RETURN: _y++; _x = 0; break;
      case COMMAND_PAGE_DOWN: _y += kurses_height() - 1; break;
      case COMMAND_PAGE_UP:   _y -= kurses_height() - 2; break;
      case COMMAND_START_OF_LINE:  _x = 0; break;
      case COMMAND_END_OF_LINE:    _x = lyr->width - 1; break;
      case COMMAND_FAST_RIGHT: _x += FAST_MOVE_AMOUNT; break;
      case COMMAND_FAST_LEFT:  _x -= FAST_MOVE_AMOUNT; break;
      case COMMAND_PICK_COLOR: /* pick up color from current position */
          if (_selmode != SM_NONE) break;
          _fg = lyr->cells[_x][_y].attr >> 4;
          _bg = lyr->cells[_x][_y].attr & 0x0F;
          break;
      case COMMAND_TINT_CELL: /* tint cell under cursor with current color */
          if (_selmode != SM_NONE) break;
          lyr->cells[_x][_y].attr = _fg << 4 | _bg;
          _x++;
          break;
          
      case COMMAND_TOGGLE_SELECTION: /* start or stop selection mode */
          if (_selmode == SM_NONE) {
             _selmode = SM_SELECT;
             _ax = _x;
             _ay = _y;
          }
          else _selmode = SM_NONE;
          break;

      case COMMAND_CANCEL: _selmode = SM_NONE; break;
      case COMMAND_SHOW_LAYER_DLG: 
                     show_layer_dlg(); /* bring up layer manager dialog */
                     /* make sure current layer is still valid */
                     if (_lyr > _doc->layer_count) _lyr = _doc->layer_count - 1;
                     break;
      case COMMAND_NEXT_LAYER:   switch_to_layer(_lyr+1); break;
      case COMMAND_PREV_LAYER:   switch_to_layer(_lyr-1); break;
      case COMMAND_DUP_LAYER:    u_dup_layer(); break;
      case COMMAND_RENAME_LAYER: u_rename_layer(); break;
      case COMMAND_RESIZE_LAYER: u_resize_layer(); break;

      case COMMAND_TOGGLE_LINE_MODE:  _lgmode = !_lgmode; break;
      case COMMAND_TOGGLE_COMPOSITE:  _compmode = !_compmode; break;
   }
   
   correct_coords();
}

