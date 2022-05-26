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

/* hashdict.h
 * A general purpose dictionary implemented by means of a hashtable.
 * This dictionary maps string keys to values of arbitrary type.
 * It allows the specification of a value-destroy function that will
 * be called whenever values are to be overwritten or deleted.
 *
 * Restrictions: 
 *
 * - keys must be strings of positive length, and must of course be
 * unique.
 * 
 * - values may not be NULL, because NULL is used as a
 * return value to indicate absense of keys, and because when writing/reading
 * to a file handling of NULL values would complicate things.
 *
 * Copyright (c) 2003 by Bruno T. C. de Oliveira
 * All rights reserved.
 *
 * 2003.01.07
 */

#ifndef btco_bores_hashdict_h
#define btco_bores_hashdict_h

#include <stdio.h>
#include <stdbool.h>

/* opaque dictionary handle */
struct HashDict_;
typedef struct HashDict_ HashDict;

/* iterator type */
struct HashDictIt_ {
   /* indicates whether this iterator is past the end of the dictionary */
   bool pastend;

   /* key and value of entry iterator is currently pointing to. Read only. */
   const char *key;
   const void *value;

   /* internal data. Do not tamper with. */
   HashDict* dict;  /* the dictionary this iterator pertains to */
   int nextbucket;  /* the next bucket to be visited after this one ends */
   void *nextnode;  /* the next node in this bucket to be visited. If
                       NULL, next advancement will cause the iterator
                       to go to the next nonempty bucket */
};
typedef struct HashDictIt_ HashDictIt;

/* Creates a new dictionary with default bucketcount and no value destroyer
 * function. */
HashDict* hashdict_create(void);

/* Creates a new dictionary given bucketcount and a value destroyer function.*/
HashDict* hashdict_create_ex(int bucketcount, void (*valuedestroyer)(void*));

/* Destroy a dictionary previously created by hashdict_create */
void hashdict_destroy(HashDict* hd);

/* Look up key <key> in the dictionary. Returns the associated value,
 * or NULL if the value is not found. */
void* hashdict_get(HashDict* hd, const char *key);

/* Sets the value associated to the given key. If the key already exists,
 * the value associated to it will be overwritten (and the value-
 * destroyer function will be called); otherwise, a new entry will be added.
 *
 * As a convenience, if value is NULL, this function is equivalent
 * to hashdict_unset(hd, key) */
void hashdict_set(HashDict* hd, const char *key, const void *value);

/* Removes from the dictionary the entry whose key is <key>, if it exists.
 * If it was removed, returns true; if it was not found, returns false */
bool hashdict_unset(HashDict* hd, const char *key);

/* Writes dictionary to file f in a binary format. valuewriter is a
 * function that will be used to write the values onto the stream. */
void hashdict_write(HashDict *hd, FILE *f, void (*valuewriter)(void*, FILE*));

/* Reads a dictionary previously saved with hashdict_write.
 * valuereader is a function that will be used to read values from
 * the stream. If valuereader returns NULL at any time, this
 * function will interpret it as an error. If this happens,
 * or if any other error occurs, this function will return NULL. */
HashDict* hashdict_read(HashDict *hd, FILE *f, void* (*valuereader)(FILE*));

/* Returns a new hash dict iterator that points BEFORE the first entry
 * in the dictionary. Iterators are lightweight objects, i.e.,
 * they don't need to be deallocated. That's why there is no
 * hashdict_it_destroy function. 
 *
 * Please note that the iterator returned is not in a valid position
 * (it is BEFORE the start). You must advance it before using the value.
 * This behavior is useful because you can write neat loops:
 *
 *    it = hashdict_it_start(hd);
 *    while (hashdict_it_advance(it)) {
 *        // use it->key and it->value here
 *    }
 */
HashDictIt hashdict_it_start(HashDict *hd);

/* Advances the given iterator. Returns true if advancement was successful,
 * or false if the end of the dictionary was reached. */
bool hashdict_it_advance(HashDictIt *hdi);

#endif

