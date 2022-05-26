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

#ifndef _btco_aewl_label_h
#define _btco_aewl_label_h

#include "widget.h"

/* definition of AewlLabel */
typedef struct AewlLabel_ {
   AewlWidget base;         /* the widget */
   char *caption;           /* caption to display */
   AewlWidget *focus_peer;  /* the focus peer of the label. The focus peer
                             * is the widget whose focus status determines
                             * whether this label will 'appear' focused or
                             * not. When the peer has the focus, this 
                             * widget will be drawn focused */
} AewlLabel;

/* creates a label with the given parameters. If width is 0,
 * width is calculated from the width of the text. */
AewlWidget *aewl_label_create(int x, int y, int width, const char *text);

/* destroys a label */
void aewl_label_destroy(AewlWidget *w);

/* repaint a label */
void aewl_label_repaint(AewlWidget *w, bool focused);

/* sets the label's text to the given text */
void aewl_label_set_text(AewlLabel *l, const char *text);

/* sets the focus peer for this label. See the description above for
 * what the focus peer is. */
void aewl_label_set_focus_peer(AewlLabel *l, AewlWidget *peer);

#endif

