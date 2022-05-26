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


#ifndef _btco_aewan_document_h
#define _btco_aewan_document_h
#include "layer.h"
#include "aeff.h"

struct _Document {
   int layer_count;   /* how many layers in document */
   Layer **layers;    /* vector of pointers to layers */

   char *metainfo;    /* additional textual information. The program will
                       * invoke an external editor when the user wants
                       * to edit the contents of this string. */
};
typedef struct _Document Document;

/* Creates a new, empty document. */
Document *document_create(void);

/* Destroys a previously created document */
void document_destroy(Document *doc);

/* Adds a new layer to the given document. After the call, the layer
 * will belong to the document and no other references to it should
 * be kept. */
void document_add_layer(Document *doc, Layer *layer);

/* Add a new layer to the given document, inserting it just before
 * the layer whose index is i. Prerequsite: 0 <= i <= doc->layer_count.
 * Notice that document_add_layer(doc, l) is equivalent to
 * document_insert_layer(doc, doc->layer_count, l) */
void document_insert_layer(Document *doc, int i, Layer *layer);

/* Deletes layer i from the document. */
void document_del_layer(Document *doc, int i);

/* Convenience function that obtains the "nominal width and height"
 * of the document, which are the dimensions of the first layer, 
 * if the document has any layers, or some sensible defaults if
 * the document has none */
void document_get_nom_dim(Document *doc, int *w, int *h);

/* Saves document to file f */
void document_save(Document *doc, AeFile *f);

/* Loads a document from file f. If the loading succeeds, returns
 * a dynamically allocated document. If the loading fails, returns NULL.
 * This function reads the OLD, binary file format. */
Document *document_load_OLD(FILE *f);

/* Loads a document from file f. If the loading succeeds, returns
 * a dynamically allocated document. If the loading fails, returns NULL.
 * This function reads the NEW, zipped-text format.
 *
 * On error, this function returns NULL and aeff_get_error can be
 * called in order to obtain an error description.
 */
Document *document_load(AeFile *f);

/* A wrapper that opens the file, detects the format and calls 
 * either document_load or document_load_OLD. 
 *
 * On error, this function returns NULL and aeff_get_error can
 * be called in order to obtain an error description.
 */
Document *document_load_from(const char *filename);

/* Gets the character and attribute at position x, y when viewing
 * the image as a composite. That is, go through all layers and see
 * which ones are transparent, visible, etc, and thus calculate the
 * resulting char/attr of the given cell. */
Cell document_calc_effective_cell(Document *doc, int x, int y);

#endif

