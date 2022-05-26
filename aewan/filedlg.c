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

/*
DIALOG LAYOUT (FULL SCREEN):

 DIALOG TITLE
 File Name: [filename_fld]  [ cancel_btn ] [ ok_btn ]
 Directory: [directory_lbl]
 +--Directories---------+   +--Files-----------------+
 | dirs_box             |   | files_box              |
 +----------------------+   +------------------------+
*/

#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#include "bores/bores.h"
#include "aewl/aewl.h"
#include "aewl/allwidgets.h"
#include "keys.h"

/* widget positions (w, h and screen dimensions) */
/* btw, STLBL = static label, that is, labels that
 * are just there to say things that never change
 * (didn't that come out somewhat poetic?) */
#define FILENAME_STLBL_X 1
#define FILENAME_STLBL_Y 2
#define DIRECTORY_STLBL_X 1
#define DIRECTORY_STLBL_Y 3
#define CANCEL_BTN_W 11
#define OK_BTN_W 11
#define CANCEL_BTN_X (w - CANCEL_BTN_W - OK_BTN_W - 2)
#define CANCEL_BTN_Y 2
#define OK_BTN_X (w - OK_BTN_W)
#define OK_BTN_Y 2
#define FILENAME_FLD_X 13
#define FILENAME_FLD_Y 2
#define FILENAME_FLD_W (CANCEL_BTN_X - FILENAME_FLD_X - 3)
#define DIRECTORY_LBL_X 13
#define DIRECTORY_LBL_Y 3
#define DIRECTORY_LBL_W (w - DIRECTORY_LBL_X)
#define DIRS_BOX_X 1
#define DIRS_BOX_Y 5
#define DIRS_BOX_W (w / 3)
#define DIRS_BOX_H (h - 6)
#define FILES_BOX_X (DIRS_BOX_X + DIRS_BOX_W + 2)
#define FILES_BOX_Y 5
#define FILES_BOX_W (w - FILES_BOX_X - 1)
#define FILES_BOX_H (h - 6)

/* result codes for internal event handling */
#define FILENAME_FLD_RES 1001
#define FILES_BOX_RES 1002
#define DIRS_BOX_RES 1003
#define CANCEL_BTN_RES 1004
#define OK_BTN_RES 1005

static AewlWidget *filename_fld, *cancel_btn, *ok_btn, 
                  *directory_lbl, *dirs_box, *files_box,
                  *filename_stlbl, *directory_stlbl;
static AewlForm *form;

static char *curdir = NULL;  /* the currently selected directory (full path).
                       * This will persist across invocations of the dialog. */

/* fills the dirs listbox with the subdirectories of the current directory,
 * updates the file listbox with the files in the current directory and
 * all such things */
static void _update_widgets(void);

static bool _is_dir_valid(const char *dir) {
   DIR *d = opendir(dir);
   if (d) {
      closedir(d);
      return true;
   }

   return false;
}

static char *my_getcwd() { /* returns newly allocated buffer. NULL on error */
   int cap;
   char *s = malloc(cap = 32);

   while (!getcwd(s, cap)) {
      if (errno != ERANGE) return NULL;
      s = malloc(cap += cap);
   }

   return s;
}

static char *_get_canon_dir_path(const char *base, const char *relative) {
   /* Warning: this is a ridiculous way to go about this */
   char *oldpwd = my_getcwd();
   char *retval;

   if (!oldpwd) return NULL;
   
   if (0 != chdir(base)) return NULL;
   if (0 != chdir(relative)) return NULL;

   retval = my_getcwd();

   if (0 != chdir(oldpwd)) {
      fprintf(stderr, "PANIC: Can't go back to old wd '%s'\n%s\n",
                        oldpwd, strerror(errno));
      abort();
   }

   return retval;
}

char *filedlg_show(const char *title) {
   int w, h, r, ch, s;
   char *retval = NULL;

   /* if the current directory is unset or is invalid (for example
    * has been deleted since the file dialog was last invoked),
    * reset it to the application's current working directory */
   if (!curdir || !_is_dir_valid(curdir)) {
      curdir = my_getcwd();
      if (!curdir) curdir = strdup("/"); /* last resort -- this will 
                                          * happen if the current working
                                          * directory of the application is
                                          * removed, for example. */
   }

   push_screen();
   clear();

   kurses_color(14, 0);
   kurses_move(0, 0);
   addstr(title);

   w = kurses_width();
   h = kurses_height();
   
   form = aewl_form_create(stdscr);
   filename_stlbl = aewl_label_create(FILENAME_STLBL_X, FILENAME_STLBL_Y, 0, 
                            "File &Name:");
   directory_stlbl = aewl_label_create(DIRECTORY_STLBL_X, DIRECTORY_STLBL_Y, 0, 
                            "Directory:");
   filename_fld = aewl_field_create(FILENAME_FLD_X, FILENAME_FLD_Y,
                            FILENAME_FLD_W, KEY_ALT_N, FILENAME_FLD_RES, "");
   directory_lbl = aewl_label_create(DIRECTORY_LBL_X, DIRECTORY_LBL_Y,
                            DIRECTORY_LBL_W, "");
   cancel_btn = aewl_button_create(CANCEL_BTN_X, CANCEL_BTN_Y, CANCEL_BTN_W,
                            0, CANCEL_BTN_RES, "Cancel");
   ok_btn = aewl_button_create(OK_BTN_X, OK_BTN_Y, OK_BTN_W, 
                            0, OK_BTN_RES, "OK");
   files_box = aewl_listbox_create(FILES_BOX_X, FILES_BOX_Y,
                                   FILES_BOX_W, FILES_BOX_H,
                                   KEY_ALT_F, "&Files", FILES_BOX_RES);
   dirs_box = aewl_listbox_create(DIRS_BOX_X, DIRS_BOX_Y,
                                  DIRS_BOX_W, DIRS_BOX_H,
                                  KEY_ALT_D, "&Directory", DIRS_BOX_RES);

   /* set widget focus peers */
   aewl_label_set_focus_peer((AewlLabel*) filename_stlbl, filename_fld); 

   /* add all widgets to the form */
   aewl_form_add_widget(form, filename_stlbl);
   aewl_form_add_widget(form, directory_stlbl);
   aewl_form_add_widget(form, filename_fld);
   aewl_form_add_widget(form, directory_lbl);
   aewl_form_add_widget(form, cancel_btn);
   aewl_form_add_widget(form, ok_btn);
   aewl_form_add_widget(form, dirs_box);
   aewl_form_add_widget(form, files_box);

   /* show the form */
   _update_widgets();
   aewl_form_paint(form, true);
   while (1) {
      /* if files box has focus, set the field's text to match current
       * selection */
      if (aewl_form_get_focus_widget(form) == files_box) {
         const char *s = aewl_listbox_get((AewlListBox*) files_box,
                            aewl_listbox_get_sel((AewlListBox*) files_box));
         if (s) aewl_field_set_text((AewlField*) filename_fld, s);
         aewl_form_paint(form, true);
      }
      
      /* get next key and dispatch it */
      ch = getch();
      r = aewl_form_dispatch_key(form, ch);

      /* interpret returned result code */
      if (r == CANCEL_BTN_RES || ch == 27 || ch == 3 || ch == 7) {
         /* if user pressed the cancel button, pressed ESC or Ctrl+C or Ctrl+G, 
          * cancel dialog */
         retval = NULL;
         break;
      }
      else if (r == FILENAME_FLD_RES || r == OK_BTN_RES || r == FILES_BOX_RES) {
         /* if user confirmed selection, quit dialog and return the
          * currently selected file, that is, the current directory
          * plus the field's contents (remember that it will already have 
          * been updated to match the filebox's selection, if appropriate) */
         const char *fieldtext = aewl_field_get_text((AewlField*)filename_fld);

         /* TODO: work needs to be done here to enable the user to,
          * for example, type a full path to save the file, or
          * type the name of a directory to jump to it. At the very
          * least, this should disallow slashes in the file name! */
         
         retval = malloc(strlen(fieldtext) + strlen(curdir) + 6);
         sprintf(retval, "%s/%s", curdir, fieldtext);
         break;
      }
      else if (r == DIRS_BOX_RES &&
                    (s = aewl_listbox_get_sel((AewlListBox*) dirs_box)) >= 0) {
         /* user pressed enter in the directories box, so change into that
          * directory */
         const char *d = aewl_listbox_get((AewlListBox*) dirs_box, s);

         /* get the canonicalized name of the new directory */
         char *canon_path = _get_canon_dir_path(curdir, d);

         if (!canon_path) return NULL; /* FIXME: error message? */
         
         if (_is_dir_valid(canon_path)) {
            if (curdir) free(curdir);
            curdir = canon_path;

            /* clear file name text field */
            aewl_field_set_text((AewlField*) filename_fld, "");
         
            _update_widgets();
            aewl_form_paint(form, true);
         }
         else
            free(canon_path);
         /* FIXME: Should't we display a message if the directory is invalid? */
      }
   }

   aewl_form_destroy(form);
   pop_screen();

   return retval; /* return filename if user pressed ok */
}

static void _update_widgets(void) {
   DIR *d;
   struct dirent *dent;
   struct stat st;

   d = opendir(curdir);

   if (!d) {
      fprintf(stderr, "ERROR in opendir(\"%s\"): %s\n", curdir, 
                                                        strerror(errno));
      abort();
   }

   aewl_listbox_clear((AewlListBox*) dirs_box);
   aewl_listbox_clear((AewlListBox*) files_box);
   while ( (dent = readdir(d)) ) {
      char *compname = zalloc(strlen(dent->d_name) + strlen(curdir) + 2);
      sprintf(compname, "%s/%s", curdir, dent->d_name);
      
      if (0 != stat(compname, &st)) continue; /* broken links and such */

      free(compname);
   
      if (S_ISDIR(st.st_mode))
         aewl_listbox_add((AewlListBox*) dirs_box, dent->d_name);
      else
         aewl_listbox_add((AewlListBox*) files_box, dent->d_name);
   }
   aewl_listbox_sort((AewlListBox*) dirs_box);
   aewl_listbox_sort((AewlListBox*) files_box);

   closedir(d);
   
   /* update the directory label */
   aewl_label_set_text((AewlLabel*)directory_lbl, curdir);
}

