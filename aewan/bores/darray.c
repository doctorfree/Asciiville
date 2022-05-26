/*
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


#include "darray.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_CAPACITY 20

#define CAPACITY_AHEAD 20  /* when a value is assigned to an out of bounds 
                              item, this is how far beyond it the capacity
                              grows. */

struct DArray_ {
   int len;
   int cap;
   void **items;
   void (*value_destroyer)(void*);
};

DArray* darray_create() {
   return darray_create_ex(DEFAULT_CAPACITY, NULL);
}

DArray* darray_create_ex(int cap, void (*vd)(void*)) {
   DArray* da = zalloc(sizeof(DArray));

   da->len = 0;
   da->cap = (cap > 0) ? cap : DEFAULT_CAPACITY;
   da->items = zalloc(sizeof(void*) * da->cap);
   da->value_destroyer = vd;

   return da;
}

void darray_destroy(DArray* da) {
   int i;
   if (!da) return;
   if (da->value_destroyer) {
      /* call value destroyer for each item */
      for (i = 0; i < da->len; i++)
         if (da->items[i]) (*da->value_destroyer)(da->items[i]);
   }

   /* free up the items array then the darray itself */
   sfree(da->items);
   sfree(da);
}

int darray_len(DArray* da) {
   /* this one is really complicated */
   return da->len;
}

void* darray_get(DArray* da, int i) {
   return (i >= 0 && i < da->len) ? da->items[i] : NULL;
}

void darray_set(DArray* da, int i, const void *v) {
   if (i < 0) {
      fprintf(stderr, "*** FATAL ERROR ***\n"
                      "Attempt to darray_set with negative index %d\n", i);
      abort();
   }
   else if (i >= da->len) {
      da->cap = i + CAPACITY_AHEAD;
      da->items = srealloc(da->items, sizeof(void*) * da->cap);

      /* grow array while assignung NULLs to all newly included items */
      while (i >= da->len)  da->items[da->len++] = NULL;
   }
   else {
      /* call value destroyer if there was anything there */
      if (da->items[i] && da->value_destroyer)
         (*da->value_destroyer)(da->items[i]);
   }

   da->items[i] = (void*) v;
}

void* darray_snatch(DArray *da, int i) {
   void *v;
   if (i >= 0 && i < da->len) {
      v = da->items[i];
      da->items[i] = NULL;
      return v;
   }
   else return NULL;
}

void darray_append(DArray *da, const void *v) {
   darray_set(da, da->len, v);
}

void darray_remove(DArray *da, int i) {
   if (i < 0 || i >= da->len) return;
   if (da->items[i] && da->value_destroyer)
      (*da->value_destroyer)(da->items[i]);

   while (i < da->len - 1) {
      da->items[i] = da->items[i+1];
      i++;
   }

   da->len--;
}

