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


#ifndef _btco_aewan_ui_h
#define _btco_aewan_ui_h

#include <stdbool.h>
#include "bores/bores.h"

extern int ui_cancel;  /* raised if the user cancels input in any of the
                        * ui_* functions. */

/* Asks the user to input an integer number. Presents the given prompt.
 * If the user just presses ENTER, returns <def>. Otherwise, returns
 * the user's input if min <= input <= max. If the user input is not
 * in the [min,max] interval, keeps asking again until the user inputs
 * a valid value. If the user presses ESC, or Ctrl+C or any such thing,
 * the return value is undefined and the global flag ui_cancel is raised.
 * The ui_cancel flag is lowered at the beginning of the operation, so
 * there is no need to lower it before calling this function. */
int ui_ask_i(const char *prompt, int def, int min, int max);

/* Asks the user to input a string. Behavior is similar to ui_ask_i.
 * The returned string is dynamically allocated and should be free()'d
 * by the caller. Even if the default value is returned, the return value
 * is an strdup()'ed value, so it should be free()'d by caller even in
 * that case. */
char *ui_ask_s(const char *prompt, const char *def);

/* Asks the user a color. Similar to the other ui_ask_ functions. 
 * Returns the selected color code, between 0 and 15 inclusive. */
int ui_ask_color(const char *prompt);

/* Asks a yes/no question. If <defyes>, the default is yes, otherwise
 * the default is no. Returns whether the user chose yes or not.
 * If the user cancels, ui_cancel is set to true and return value
 * is undefined. */
int ui_ask_yn(const char *prompt, int defyes);

/* Presents a message to the user and awaits for a keypress.
 * If <level> is UIMSG_ERROR, message will be shown as an error condition
 * (i.e. in red and possibly accompanied by a beep). */
void ui_message(const char *msg, int level);
                                 #define UIMSG_INFORM 0
                                 #define UIMSG_ERROR 1


#endif

