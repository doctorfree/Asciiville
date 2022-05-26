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

#ifndef _btco_aewan_chtr_h
#define _btco_aewan_chtr_h

/* Since a control character may never be in a cell, aewan uses some
 * values in the 0-31 range to represent special characters. It does
 * not use the curses ACS_ characters because their values are 
 * implementation dependent and are all greater than 255 (so they would 
 * not fit in an unsigned char anyway)
 *
 * The chtr in "chtr.h" stands for "character translation"
 */

/* aewan special values for a cell's character */
#define AEWAN_CHAR_HLINE         1
#define AEWAN_CHAR_VLINE         2
#define AEWAN_CHAR_ULCORNER      3
#define AEWAN_CHAR_TTEE          4
#define AEWAN_CHAR_URCORNER      5
#define AEWAN_CHAR_LTEE          6
#define AEWAN_CHAR_PLUS          7
#define AEWAN_CHAR_RTEE          8
#define AEWAN_CHAR_LLCORNER      9
#define AEWAN_CHAR_BTEE         10
#define AEWAN_CHAR_LRCORNER     11
#define AEWAN_CHAR_CKBOARD      12

/* Maps an aewan character to a curses character. This is not restricted
 * to special characters: regular characters will map to themselves. 
 * Control characters that do not have special meanings will map
 * to some bogus (but printable) character. */
int chtr_a2c(unsigned char aewan_ch);

#endif

