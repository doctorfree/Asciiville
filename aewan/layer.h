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


#ifndef _btco_aewan_layer_h
#define _btco_aewan_layer_h

#include <stdbool.h>
#include "bores/bores.h"
#include <stdio.h>
#include "aeff.h"

struct _Cell {
   unsigned char ch;   /* character that occupies this cell.
                        * Since a control character may never be in a cell,
                        * aewan uses some characters in the 0-31 range to
                        * represent vt100 line graphics characters and
                        * other special characters. See the chtr.h
                        * file for more details.  */

   unsigned char attr; /* attribute of the character */
};
typedef struct _Cell Cell;

struct _Layer {
   char *name;           /* name of this layer */
   int width, height;    /* width and height of layer, in cells */
   Cell **cells;         /* indexed cells[0][0] to cells[width-1][height-1] */
   bool visible;        /* whether visibility for this layer is enabled */
   bool transp;         /* whether this layer has transparency enabled.
                          * Upon blit and paint operations, transparent
                          * cells will not be drawn. */
};
typedef struct _Layer Layer;

/* A blank cell. */
extern const Cell BLANK_CELL;

/* Creates a new, empty layer with the specified dimensions.
 * Sets the layer name to <layer_name>. Returns a pointer to the newly
 * created layer. */
Layer *layer_create(const char *layer_name, int width, int height);

/* Creates a new layer that is an exact copy of the given layer,
 * except possibly for the name. Returns a pointer to the newly created 
 * layer */
Layer *layer_dup(const char *layer_name, Layer *model);

/* Destroys a layer previously created with layer_create */
void layer_destroy(Layer *layer);

/* Copies part of or the totality of the contents of the source layer onto 
 * the destination layer. The source rectangle for the copy is given
 * by xsrc, ysrc, width and height; the destination rectangle is given
 * by xdst, ydst, width and height. */
void layer_blit(Layer *src, int xsrc, int ysrc, int width, int height,
                Layer *dst, int xdst, int ydst);

/* Paints the given layer on the screen, placing its top-left corner
 * at the given coordinates x0, y0, which may be negative. The clipping
 * rectangle is given by xclip, yclip, wclip and hclip. 
 *
 * Painting code allows custom modification of output through
 * a call to the modify function. If <modify> is not NULL, it
 * will be called for every cell drawn, and it can modify the
 * color.
 * 
 * Please note that this function pays no attention to the layer's
 * 'visible' flag: it draws it regardless. It DOES pay attention
 * to the 'transparent' flag. 
 */
void layer_paint(Layer *layer, int x0, int y0, int xclip, int yclip,
                               int wclip, int hclip,
                        void (*modify)(int x, int y, int *ch, int *attr));

/* Same as layer_paint, but forces opaque rendering, that is, transparent
 * cells will be rendered as white-foreground, black-background spaces. */
void layer_paint_opaque(Layer *layer, int x0, int y0, int xclip, int yclip,
                               int wclip, int hclip,
                        void (*modify)(int x, int y, int *ch, int *attr));

/* Returns whether the given (x, y) coordinates are valid within
 * layer lyr, that is, returns true if and only if 0 <= x <= lyr->width
 * and 0 <= y <= lyr->height */
bool layer_valid_cell(Layer *layer, int x, int y);

/* Performs a vertical mirroring operation on the given layer.
 * The <flipchars> parameter indicates whether characters themselves
 * should be "flipped" to their mirrored counterparts (e.g.
 * change '>' to '<', etc). */
void layer_flip_x(Layer *layer, bool flipchars);

/* Performs a horizontal mirroring operation on the given layer.
 * The <flipchars> parameter indicates whether characters themselves
 * should be "flipped" to their mirrored counterpars (e.g.
 * change '\' to '/' */
void layer_flip_y(Layer *layer, bool flipchars);

/* Writes layer's data to file f */
void layer_save(Layer *lyr, AeFile *f);

/* Attempts to load layer from file f; if loading succeeds, returns
 * a pointer to a dynamically allocated layer that corresponds
 * to the loaded data; if an error occurs, returns NULL. */
Layer *layer_load(AeFile *f);

/* Loads a layer in the OLD binary format */
Layer *layer_load_OLD(FILE *f);

/* Returns whether the given cell is considered transparent or not */
bool is_cell_transp(const Cell* c);

#endif

