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

#ifndef _btco_aewl_h
#define _btco_aewl_h

#include <ncurses.h>
#include <stdbool.h>
#include "widget.h"

typedef struct AewlAttrQuad {
   int normal, focus, highlight, highlight_focus;
} AewlAttrQuad;

/* defines what curses attributes to use for various interface components */
typedef struct AewlAttrSettings_ {
   AewlAttrQuad text;
   AewlAttrQuad button;
   AewlAttrQuad frame;
   AewlAttrQuad field;
   int field_pad;  /* color of the character with which text fields 
                    * are padded */
} AewlAttrSettings;

#define AEWL_ATTR(type, focused, highlighted) ( focused ? \
 ( highlighted ? aewl_attrs.type.highlight_focus : aewl_attrs.type.focus ) \
 : \
 ( highlighted ? aewl_attrs.type.highlight : aewl_attrs.type.normal) )


extern AewlAttrSettings aewl_attrs;

/* Initializes the widget library. You must fill an AewlAttrSettings structure
 * and pass it to this function to define which attributes will be used
 * for each interface component */
void aewl_init(const AewlAttrSettings *s);

#endif

