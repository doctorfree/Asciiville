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

#include "hashdict.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define DEFAULT_BUCKET_COUNT 30

struct Node_ {  /* each node in the linked lists */
   char *key;   
   const void *value;

   struct Node_ *next;
};
typedef struct Node_ Node;

struct HashDict_ {
   int bucketcount;  /* how many buckets in this dictionary */
   Node *bucketheads;/* array of head nodes, one for each bucket.
                      * The data in the head node means nothing. The linked
                      * list begins with the node that head->next points to */
   void (*value_destroyer)(void*);  /* function called to destroy values.
                                     * If NULL, no function is called */
};

/* declaration of private functions -----------------------------*/
static void _destroy_node(HashDict *hd, Node* n);
static int _which_bucket(const char *key, int bucketcount);
static Node* _node_lookup(HashDict *hd,
      const char *key); /* warning: this function has counter-intuitive 
                           (but useful) beheavior (q.v.) */
/* --------------------------------------------------------------*/

HashDict* hashdict_create_ex(int bucketcount, void (*value_destroyer)(void*)) {
   HashDict *hd = zalloc(sizeof(struct HashDict_));
   hd->bucketcount = bucketcount;
   hd->value_destroyer = value_destroyer;
   hd->bucketheads = zalloc(sizeof(Node) * bucketcount);

   return hd;
}

HashDict* hashdict_create(void) {
   return hashdict_create_ex(DEFAULT_BUCKET_COUNT, 0);
}

void hashdict_destroy(HashDict* hd) {
   int i; Node *n;
   if (!hd) return;
   
   /* destroy all nodes */
   for (i = 0; i < hd->bucketcount; i++) {
      while ( (n = hd->bucketheads[i].next) ) {
         /* delete node n */
         hd->bucketheads[i].next = n->next;
         _destroy_node(hd, n);
      }
   }

   free(hd->bucketheads);
   free(hd);
}

void* hashdict_get(HashDict *hd, const char *key) {
   Node *n = _node_lookup(hd, key);

   /* be mindful that _node_lookup returns the node BEFORE
      the one we are looking for */
   return (void*) (n->next ? n->next->value : 0);
}

void hashdict_set(HashDict *hd, const char *key, const void *value) {
   Node *n;
   if (!value) { hashdict_unset(hd, key); return; }

   n = _node_lookup(hd, key);
   if (n->next) {
      /* node already exists: replace value */
      if (hd->value_destroyer) (*hd->value_destroyer)((void*)n->next->value);
      n->next->value = value;
   }
   else {
      /* does not exist yet: append to list */
      Node *newnode = zalloc(sizeof(Node));
      newnode->key = sstrdup(key);
      newnode->value = value;
      newnode->next = NULL;
      n->next = newnode;
   }
}

bool hashdict_unset(HashDict *hd, const char *key) {
   Node *n = _node_lookup(hd, key);
   if (n->next) {
      _destroy_node(hd, n->next);
      n->next = n->next->next;
      return true; /* found and deleted */
   }
   return false; /* not found */
} 

void hashdict_write(HashDict *hd, FILE *f, void (*value_writer)(void*, FILE*)) {
   /* TODO */
}

HashDict* hashdict_read(HashDict *hd, FILE *f, void* (*valuereader)(FILE*)) {
   /* TODO */
   return NULL;
}

HashDictIt hashdict_it_start(HashDict *hd) {
    HashDictIt it;

    /* put iterator before beginning */
    it.pastend = false;
    it.key = it.value = NULL;
    it.dict = hd;
    it.nextbucket = 0;
    it.nextnode = NULL;
    
    return it;
}

bool hashdict_it_advance(HashDictIt *hdi) {
   if (hdi->pastend) return false; /* already at the end */

   if (hdi->nextnode) {
      hdi->key = ((Node*)hdi->nextnode)->key;
      hdi->value = ((Node*)hdi->nextnode)->value;
      hdi->nextnode = ((Node*)hdi->nextnode)->next;
   }
   else {
      /* go to next nonempty bucket */
      HashDict *hd = hdi->dict;
      int i = hdi->nextbucket;
      while (i < hd->bucketcount && !hd->bucketheads[i].next) i++;

      if (i < hd->bucketcount) {
         /* we know hd->buckethead[i].next is not NULL */
         hdi->key = hd->bucketheads[i].next->key;
         hdi->value = hd->bucketheads[i].next->value;
         hdi->nextbucket = i+1;
         hdi->nextnode = hd->bucketheads[i].next->next;
      }
      else {
         /* passed the end */
         hdi->key = hdi->value = NULL;  /* for safety */
         hdi->pastend = true;
         return false;
      }
   }

   return true;
}

/* private functions ----------------------------------------------------- */
static Node* _node_lookup(HashDict *hd, const char *key) { 
   /* ATTENTION: return value is not the node you are looking for; 
    * rather, it is the node BEFORE it. This is useful for a variety
    * of reasons, including node deletion and appending */

   /* returns ptr to node n such that n->next->key is equal to <key>.
    * If not found, returns the last node in the bucket that <key>
    * hashes into. (in this case, the 'next' field of the return
    * value will be NULL, since it is the last node in a list). */
   Node *n = &(hd->bucketheads[_which_bucket(key, hd->bucketcount)]);
   while (n->next && strcmp(n->next->key, key)) n = n->next;
   return n;
}

static void _destroy_node(HashDict *hd, Node* n) {
   if (n) {
      if (hd->value_destroyer && n->value)
         (*hd->value_destroyer)((void*)n->value);
      zfree(&n->key);
      free(n);
   }
}

static int _which_bucket(const char *key, int bucketcount) {
   /* this function computes the hashcode of the given key
    * in a dictionary with <bucketcount> buckets. If str is non-NULL
    * and bucketcount > 0, the return value is guaranteed to be 
    * nonnegative and smaller than bucketcount. */
   
   /* FIXME: write a serious hash function. This one is laughable. */

   int x = 0;
   while (*key) x += *(key++);
   return x % bucketcount;
}

