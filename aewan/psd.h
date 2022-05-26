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


#ifndef _btco_aewan_psd_h
#define _btco_aewan_psd_h

#include "bores/bores.h"

#include "version.h"

/* selection modes: */
#define SM_NONE    0     /* no selection */
#define SM_SELECT  1     /* grow/shrink selection box */
#define SM_FLOAT   2     /* paste mode: clipboard floats around screen */

struct _Document;

/* global program state data declarations */
extern struct _Document *_doc; 
                         /* the current document. Must ALWAYS be non-null. */

extern char *_filename;  /* the name of the file from which it was loaded; 
                          * NULL if new file */

extern int _lyr;      /* currently active layer. This is a valid layer
                       * index if and only if _doc->layer_count >= 1 */
extern int _x, _y;    /* cursor position (logical coordinates) */
extern int _svx, _svy;/* "smallest visible x" and "smallest visible y"
                       * These are used in order to convert logical coordinates
                       * to screen coordinates. Logical coordinates _svx,_svy
                       * are mapped to screen (0,0). */
extern int _fg, _bg;  /* currently selected foreground and background colors */
extern int _selmode;  /* selection mode: SM_NONE, SM_SELECT, SM_FLOAT */
extern int _ax, _ay;  /* the anchor-point (i.e. where the selection begins).
                       * Valid only if _selmode == true */
extern bool _insmode;/* whether insert mode is on. In insert mode, typing
                       * a character on a line shifts the other characters
                       * to the right. */

extern bool _lgmode; /* line graphics mode. When this mode is active,
                         certain keystrokes produce different characters */

extern bool _compmode; /* composite mode. In this mode, all layers will
                         * be visible. */

extern struct _Layer *_clipboard; 
                      /* clipboard layer. NULL if nothing in clipboard. */

/* --- functions that operate on psd ------------------------------------- */

/* Zeroes program state data */
void zero_state(void);

/* Convenience function that returns whether document is empty or not */
bool doc_empty(void); 

/* Obtains normalized selection coordinates. Returned coordinates x0, y0,
 * x1, y1 represent the selection rectangle, with the property that
 * x0 <= x1 and y0 <= y1 */
void get_norm_sel(int *x0, int *y0, int *x1, int *y1);

/* Switches active layer to l. Does appropriate coordinate bound
 * checking and other administrative tasks */
void switch_to_layer(int l);

#endif

