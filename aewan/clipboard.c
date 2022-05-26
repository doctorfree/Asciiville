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

#include <stdio.h>

#include "clipboard.h"
#include "document.h"
#include "psd.h"

void clear_clipboard(void) {
   if (_clipboard) layer_destroy(_clipboard);
   _clipboard = NULL;
}

void copy_sel_to_clipboard(void) {
   int selwidth, selheight;
   int x0, x1, y0, y1;

   /* calculate selection dimensions */
   get_norm_sel(&x0, &y0, &x1, &y1);
   selwidth = x1 - x0 + 1;
   selheight = y1 - y0 + 1;
   
   /* size clipboard appropriately */
   clear_clipboard();
   _clipboard = layer_create("clipboard", selwidth, selheight);

   /* blit from currently active document layer to clipboard */
   layer_blit(_doc->layers[_lyr], x0, y0, selwidth, selheight, 
              _clipboard, 0, 0);
   
}

void paste_clipboard(void) {
   if (!_clipboard) return;
   layer_blit(_clipboard, 0, 0, _clipboard->width, _clipboard->height,
              _doc->layers[_lyr], _x, _y);
}


