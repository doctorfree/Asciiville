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

#include "bores/bores.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "document.h"

#define DEFAULT_NOM_WIDTH   80
#define DEFAULT_NOM_HEIGHT  25

Document *document_create(void) {
   Document *doc = (Document*) zalloc(sizeof(Document));
   return doc;
}

void document_destroy(Document *doc) {
   int i;
   if (!doc) return;
   if (doc->layers) {
      for (i = 0; i < doc->layer_count; i++)
         free(doc->layers[i]);
      free(doc->layers);
   }

   dstrset(&doc->metainfo, 0);
   free(doc);
}

void document_add_layer(Document *doc, Layer *layer) {
   (doc->layers = srealloc(doc->layers, (++doc->layer_count) * sizeof(Layer*)))
                  [doc->layer_count-1] = layer;
}

void document_insert_layer(Document *doc, int i, Layer *layer) {
   int j;
   if (i < 0) i = 0;
   if (i > doc->layer_count) i = doc->layer_count;

   /* first reallocate the array (and increment doc->layer_count) */
   doc->layers = srealloc(doc->layers, (++doc->layer_count) * sizeof(Layer*));

   /* now shift layers to make room */
   for (j = doc->layer_count - 1; j > i; j--)
      doc->layers[j] = doc->layers[j-1];

   /* position i is now free */
   doc->layers[i] = layer;
}

void document_del_layer(Document *doc, int i) {
   if (i < 0 || i >= doc->layer_count) return;

   /* FIXME: this doesn't realloc the layers array, so it does not
    * reclaim memory allocated by document_add_layer. */
   layer_destroy(doc->layers[i]);
   for (; i < doc->layer_count - 1; i++)
      doc->layers[i] = doc->layers[i+1];

   doc->layer_count--; 
}

void document_get_nom_dim(Document *doc, int *w, int *h) {
   if (!doc || !doc->layer_count) {
      if (w) *w = DEFAULT_NOM_WIDTH;
      if (h) *h = DEFAULT_NOM_HEIGHT;
   }
   else {
      if (w) *w = doc->layers[0]->width;
      if (h) *h = doc->layers[0]->height;
   }
}

void document_save(Document *doc, AeFile *f) {
   int i;

   aeff_write_header(f, "Aewan Document v1");
   aeff_write_int(f, "layer-count", doc->layer_count);
   aeff_write_string(f, "meta-info", doc->metainfo ? doc->metainfo : "");

   for (i = 0; i < doc->layer_count; i++)
      layer_save(doc->layers[i], f);

   aeff_write_footer(f, "Aewan Document v1");
}

Document *document_load_from(const char *filename) {
   static char buf[2];
   int i;
   Document *d = NULL;

   FILE *f = fopen(filename, "r");
   if (!f) {
      char *msg = dsprintf("%s: %s", filename, strerror(errno));
      aeff_set_error(msg);
      zfree(&msg);
      return NULL;
   }

   /* get first 2 bytes from file and close it */
   for (i = 0; i < 2; i++) buf[i] = fgetc(f);
   fclose(f);

   if (!strncmp(buf, "DO", 2)) {
      /* old format (binary). */
      d = document_load_OLD(f = fopen(filename, "r"));
      if (!d) aeff_set_error("Malformed (old) binary format");
      fclose(f);
      return d;
   }
   else if (!strncmp(buf, "\037\213", 2)) {
      /* gzip format: assume new format (aeff) */
      AeFile *aeff = aeff_open(filename, 'r');
      if (!aeff) return NULL;
      return document_load(aeff);
   }
   else {
      aeff_set_error("Unrecognized file format.");
      return NULL;
   }
}

Document *document_load(AeFile *f) {
   int lc;
   Document *d = document_create();
   Layer *l = NULL;
   
   if (!aeff_read_header(f, "Aewan Document v1")) goto exception;
   if (!aeff_read_int(f, "layer-count", &lc)) goto exception;
   if (!aeff_read_string(f, "meta-info", &d->metainfo)) goto exception;


   /* load each layer (there are <lc> layers to load) */
   while (lc--) {
      if ( ! (l = layer_load(f)) ) goto exception;
      document_add_layer(d, l);
      l = 0;  /* forget about the layer, it now belongs to the document */
   }

   if (!aeff_read_footer(f, "Aewan Document v1")) goto exception;
   return d;  /* all went well */

/* I think exception handling is a pretty legitimate use of goto,
 * so don't complain */
exception:
   if (d) document_destroy(d);
   if (l) layer_destroy(l);
   return NULL;
}

Document *document_load_OLD(FILE *f) {
   /* THIS FUNCTION LOADS THE OLD FORMAT. FOR BACKWARD COMPATIBILITY ONLY */
   int i; int lc;
   Document *d; Layer *l;
   if ('D' != fgetc(f) || 'O' != fgetc(f)) return NULL;
   
   fread(&lc, sizeof(int), 1, f);
   
   d = document_create();
   if ( !(d->metainfo = floadstr(f)) ) {
      document_destroy(d);
      return NULL;
   }

   for (i = 0; i < lc; i++) {
      if ( !(l = layer_load_OLD(f)) ) {
         document_destroy(d);
         return NULL;
      }
      
      document_add_layer(d, l);
   }

   return d;
}

Cell document_calc_effective_cell(Document *doc, int x, int y) {
   int i;
   Layer *lyr;
   Cell result;
   result.ch = 0x20, result.attr = 0x70;

   for (i = doc->layer_count - 1; i >= 0; i--) {
      lyr = doc->layers[i];
      if (x < 0 || x >= lyr->width || y < 0 || y >= lyr->width) continue;
      if (!lyr->visible) continue;
      if (lyr->transp && is_cell_transp(&lyr->cells[x][y])) continue;

      result.ch = lyr->cells[x][y].ch;
      result.attr = lyr->cells[x][y].attr;
   }

   return result;
}

