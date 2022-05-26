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

#ifndef btco_bores_darray_h
#define btco_bores_darray_h

/* opaque dynamic array type */
struct DArray_;
typedef struct DArray_ DArray;

/* creates a new dynamic array of default initial capacity and no
 * value destroyer function. */
DArray* darray_create();

/* creates a new dynamic array of initial capacity cap (must be positive)
 * and value destroyer function f. The value destroyer is a function
 * called whenever a value stored in the array must be destroyed or
 * will be overwritten. */
DArray* darray_create_ex(int cap, void (*vd)(void*));

/* destroys the given dynamic array. The value destroyer function will
 * be called for all non-NULL values in the array at this point. */
void darray_destroy(DArray*);

/* obtains the length of a dynamic array. This is NOT the capacity. The
 * length is how many elements are actually there. You should not be
 * interested in the capacity, since it is internally managed. */
int darray_len(DArray*);

/* obtains the item from the array whose index is i. If i is out of
 * bounds, NULL is returned (and no error occurs). PLEASE notice
 * that this function is for consultation only, the value will still
 * be owned by the darray (and thus can be destroyed at any moment
 * by the value destroyer function). If you want to gain ownership
 * over the value you get, use darray_snatch instead. */
void* darray_get(DArray*, int i);

/* sets the value of item index i to NULL, without calling the value
 * destroyer function for it. Returns the value the item had before
 * being set to NULL. If the index i is out of bounds, nothing is done
 * and NULL is returned. */
void* darray_snatch(DArray*, int i);

/* sets the value of item index i to v. If the item had a non-NULL value
 * before, the value destroyer function will be called for it before
 * it is overwritten. If the index is out of bounds and positive, the
 * array will be dynamically expanded to make it valid. If it is
 * negative a fatal error occurs. 
 *
 * If i is out of bounds and v is NULL, this function does absolutely 
 * nothing, because expanding the array to put a NULL value is
 * really the same as just leaving it as it is, because darray_get
 * returns NULL if you pass it an out-of-bounds index.
 *
 * Whenever the array expands, the newly created items are set to NULL,
 * that is, they are not left with trash in them.
 */
void darray_set(DArray*, int i, const void *v);

/* Same as darray_set(da, darray_len(da), v) */
void darray_append(DArray*, const void *v);

/* removes item i from array, shifting all others up */
void darray_remove(DArray*, int i);

#endif

