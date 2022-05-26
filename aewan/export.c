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

#include <string.h>
#include "export.h"

void decode_cell(const Cell* cell, DecodedCell *dc) {
   dc->fg     = (cell->attr & 0xf0) >> 4;
   dc->bg     = cell->attr & 0x0f;
   dc->ch     = cell->ch;

   dc->fg3   = (cell->attr & 0x70) >> 4;
   dc->bg3   = cell->attr & 0x07;
   dc->bold  = (cell->attr & 0x80) ? 1 : 0;
   dc->blink = (cell->attr & 0x08) ? 1 : 0;
}

bool export_vlayer_to_ansi(VirtualLayer *vl, bool use_ansi, bool use_newlines, 
                                bool quotemeta, FILE *f) {
   int x, y;
   DecodedCell dec;
   Cell c;
   int cur_fg = -1;
   int cur_bg = -1; 
   const char *must_quote = "\\`$";

   for (y = 0; y < vl->height; y++) {
      for (x = 0; x < vl->width; x++) {
         vlayer_get_cell(vl, &c, x, y);
         decode_cell(&c, &dec);

         /* set up color if necessary */
         if (use_ansi && (dec.fg != cur_fg || dec.bg != cur_bg)) {
            fputs("\e[", f);
            fputs(dec.bold ? "1;" : "0;", f);
            if (dec.blink) fputs("5;", f);
            fprintf(f, "%d;%dm", 30 + dec.fg3, 40 + dec.bg3);

            cur_fg = dec.fg;
            cur_bg = dec.bg;
         }

         /* output the character */
         if (dec.ch >= 0 && dec.ch <= 32) dec.ch = ' ';

         if (quotemeta && strchr(must_quote, dec.ch)) fputc('\\', f);
         fputc(dec.ch, f);
      }
      
      if (use_newlines) fputc('\n', f);
   }

   if (use_ansi) fputs("\e[0m", f);
   return true;
}

