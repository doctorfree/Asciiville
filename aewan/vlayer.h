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

#ifndef _btco_aewan_vlayer_h
#define _btco_aewan_vlayer_h

#include "document.h"

typedef void (*vlayer_get_cell_t)(Cell *r_cell, int x, int y, void *ud);

/* A virtual layer. A virtual layer is a structure used when one wishes
 * to represent a layer abstractly, where a callback function is responsible
 * for returning the content of any given cell upon request. The layer
 * does not exist physically (or perhaps it does, but we don't know),
 * and we only rely on the function to retrieve the virtual layer's content. 
 * It is primarily used for export and display purposes where one wishes
 * to have a unified interface to deal with actual layers, composites
 * and all such things that are implemented differently but have the
 * same "outside appearance". */
typedef struct VirtualLayer_ {
   int width, height;  /* dimensions of the virtual layer */
   vlayer_get_cell_t get_cell; /* the callback that
                        * retrieves what is at cell (x,y) and records that
                        * in the structure pointed by r_cell */
   void *user_data; /* arbitrary data passed as the ud parameter of get_cell */
} VirtualLayer;

/* Creates a virtual layer with the given dimensions, callback function,
 * and user data */
VirtualLayer *vlayer_create(int width, int height, vlayer_get_cell_t get_cell,
                                        void *user_data);

/* Creates a virtual layer based on the physical layer l. You MUST NOT DESTROY
 * the layer l while the virtual layer exists. */
VirtualLayer *vlayer_create_from_layer(Layer *l);

/* Creates a virtual layer formed by making a composite from all the layers 
 * of the given document. You MUST NOT DESTROY the document while the
 * virtual layer exists. */
VirtualLayer *vlayer_create_from_composite(Document *doc);

/* Destroys a virtual layer previously created with one of the creator 
 * functions */
void vlayer_destroy(VirtualLayer *vl);

/* Gets a cell from a virtual layer. The cell is filled in r_cell. */
void vlayer_get_cell(VirtualLayer *vl, Cell *r_cell, int x, int y);

#endif

