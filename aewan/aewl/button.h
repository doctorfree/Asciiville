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

#ifndef _btco_aewl_button_h
#define _btco_aewl_button_h

#include "widget.h"

/* definition of AewlButton */
typedef struct AewlButton_ {
   AewlWidget base;     /* the widget */
   char *caption;       /* the text that appears on the button */
   int result;          /* the result code that the button issues
                         * when activated */
} AewlButton;

/* creates a button with the given parameters. If width is 0, button will
 * be large enough to contain its caption. shortcut is the shortcut key
 * to activate the button; result is the result code it yields when
 * activated. The caption accepts '&' escaping to visually indicate the
 * shortcut key. */
AewlWidget *aewl_button_create(int x, int y, int width, int shortcut,
                               int result, const char *caption);

/* destroys a button */
void aewl_button_destroy(AewlWidget *button);

/* repaint a button */
void aewl_button_repaint(AewlWidget *button, bool focused);

/* key handler function for buttons */
int aewl_button_handlekey(AewlWidget *button, int ch);

#endif

