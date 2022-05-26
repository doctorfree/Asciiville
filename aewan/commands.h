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

#ifndef btco_aewan_commands_h
#define btco_aewan_commands_h

/* Commands are what make the editor _do_ things. The keys that the
 * user presses are translated to commands and then fed into
 * the main command-handling function handle_command() */
#define COMMAND_UNDEFINED             0  /* special value used as error flag */
#define COMMAND_ADD_LAYER_DEFAULTS    1
#define COMMAND_ADD_LAYER_SPECIFY     2
#define COMMAND_SET_FOREGROUND        3
#define COMMAND_SET_BACKGROUND        4
#define COMMAND_SHOW_COLOR_DLG        5
#define COMMAND_TOGGLE_INSERT         6
#define COMMAND_SHOW_HELP_DLG         7 
#define COMMAND_LOAD_FILE             8
#define COMMAND_SAVE_FILE             9
#define COMMAND_SAVE_FILE_AS         10
#define COMMAND_EDIT_META            11
#define COMMAND_MOVE_LEFT            12
#define COMMAND_MOVE_RIGHT           13
#define COMMAND_MOVE_UP              14
#define COMMAND_MOVE_DOWN            15
#define COMMAND_CARRIAGE_RETURN      16
#define COMMAND_PAGE_DOWN            17
#define COMMAND_PAGE_UP              18
#define COMMAND_START_OF_LINE        19
#define COMMAND_END_OF_LINE          20
#define COMMAND_FAST_LEFT            21
#define COMMAND_FAST_RIGHT           22
#define COMMAND_PICK_COLOR           23
#define COMMAND_TINT_CELL            24
#define COMMAND_TOGGLE_SELECTION     25
#define COMMAND_CANCEL               26
#define COMMAND_SHOW_LAYER_DLG       27
#define COMMAND_NEXT_LAYER           28
#define COMMAND_PREV_LAYER           29
#define COMMAND_DUP_LAYER            30
#define COMMAND_RENAME_LAYER         31
#define COMMAND_TOGGLE_LINE_MODE     32
#define COMMAND_TOGGLE_COMPOSITE     33
#define COMMAND_QUIT                 34
#define COMMAND_NEW_FILE             35
#define COMMAND_SHOW_ABOUT_DLG       36
#define COMMAND_RESIZE_LAYER         37
#define COMMAND_TOTAL                38  /* total number of commands 
                                            (for sizing arrays and the like) */

#endif


