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


#ifndef _btco_aewan_export_h
#define _btco_aewan_export_h

#include <stdio.h>
#include <stdbool.h>
#include "layer.h"
#include "vlayer.h"

/* Exports the given virtual layer to text format, writing the output to the
 * given file. The parameter use_ansi_escape determines whether the function
 * should include ANSI escape sequences to reproduce the colors. 
 * The use_newlines parameter indicates whether a newline character should
 * be included after each row of output. The output will be written
 * to the file f. The quotemeta parameter indicates that special UNIX
 * characters such as backslashes and backticks should be escaped. */
bool export_vlayer_to_ansi(VirtualLayer *vl, bool use_ansi_escape, 
                           bool use_newlines, 
                           bool quotemeta, FILE *f);

/* Represents a decoded cell (see decode_cell()) below */
typedef struct {  /* represents a decoded document cell. Returned
                          * by decode_cell */

   int fg, bg;      /* the foreground and background color codes (4 bit, 0-15)
                     * for the cell */
   int ch;          /* the character in the cell */

   /* now some of the same information, but in a more broken-down form: */
   bool bold, blink;  /* bold and blink attributes */
   int fg3, bg3;       /* 3-bit color codes (0-7) for the foreground and
                        * background of the cell */
} DecodedCell;

/* Decodes a cell into a 'decoded cell' structure that gives organized
 * information about a cell, such as foreground and background colors 
 * and such. Returns the decoded cell information in *r_dc. */
void decode_cell(const Cell *c, DecodedCell *r_dc);

#endif

