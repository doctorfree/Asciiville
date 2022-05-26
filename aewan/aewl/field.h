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

#ifndef _btco_aewl_field_h
#define _btco_aewl_field_h

#include "widget.h"

/* definition of AewlField */
typedef struct AewlField_ {
   AewlWidget base;     /* the widget */
   int result;          /* result code returned when user presses ENTER
                         * while editing this field */
   char *buffer;        /* text buffer containing this field's text */
   int cap;             /* allocated capacity of above buffer */
   int pos;             /* position of cursor within field */
} AewlField;

/* creates a field with the given parameters */
AewlWidget *aewl_field_create(int x, int y, int width, int shortcut,
                              int result, const char *text);

/* destroys a field */
void aewl_field_destroy(AewlWidget *w);

/* repaint a field */
void aewl_field_repaint(AewlWidget *w, bool focused);

/* key handler function for fields */
int aewl_field_handlekey(AewlWidget *w, int ch);

/* sets the field's text to the given text */
void aewl_field_set_text(AewlField *f, const char *text);

/* returns the field's current text */
const char *aewl_field_get_text(AewlField *f);

#endif

