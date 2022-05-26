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

/* This is a set of convenience macros and functions for enabling
 * "automatic" destruction of dynamically-allocated objects when
 * exitting the scope of a function. This is especially useful
 * in functions that can return at several points, and it would be
 * difficult to keep track of what is allocated at which point in order
 * to free everything correctly. 
 *
 * The philosophy is this:
 *
 * void my_func(...) {
 *    Brick *my_brick;
 *    Camel *my_camel;
 *    char *my_string;
 *
 *    autod_begin;
 *    autod_register(my_brick, brick_destroy);  // also sets my_brick = NULL
 *    autod_register(my_camel, camel_destroy);  // also sets my_camel = NULL
 *    autod_register(my_string, free);          // also sets my_string = NULL
 *
 *    // do what you want with my_brick, my_camel and my_string
 *
 *    if (somefunc() == ERROR)
 *       autod_return;  // this takes care of destroying my_brick,
 *                      // my_camel and my_string, by calling the
 *                      // destructor function specified when registering
 *
 *    // do more stuff with my_brick, my_camel, my_string
 *
 *    autod_return;  // don't forget to put this at the end
 * }
 *
 * Sometimes you will want to return a registered object, which means
 * that it should not be free'd (the caller will get ownership of the
 * object). In this case, use autod_return_obj(v), and all registered
 * pointers will be free'd except v, which will be returned.
 *
 * Also, notice you should never do:
 *
 *      my_camel = other_camel;
 *
 * Because the original value of my_camel will be lost and will not
 * be free'd (memory leak). Instead, you should do:
 *
 *      if (my_camel) camel_destroy(my_camel);
 *      my_camel = other_camel;
 *
 * Since this occurs often, a convenience macro is provided:
 *
 *      autod_assign(my_camel, other_camel);
 *
 * This takes care of destroying the old camel before assigning the new one
 * (this sentence really sounds weird).
 */


#ifndef btco_bores_autod_h
#define btco_bores_autod_h

#include <stdlib.h>

typedef void (*autod_destructor_t)(void*);

typedef struct AutodRegistryNode_ {
   void **ptr;
   autod_destructor_t destructor;
   struct AutodRegistryNode_ *next;
} AutodRegistryNode;

#define autod_begin  AutodRegistryNode autodregistry_head; \
                     autodregistry_head.next = 0

#define autod_register(ptr, d) do_autod_register(&autodregistry_head, \
                                     (void**)&ptr, (autod_destructor_t)d)

#define autod_return_value(v) { do_autod_cleanup(&autodregistry_head, 0); return v; }
#define autod_return_obj(v) { do_autod_cleanup(&autodregistry_head, (void*)v); return v; }
#define autod_return { do_autod_cleanup(&autodregistry_head, 0); return; }
#define autod_assign(ptr, newvalue) do_autod_assign(&autodregistry_head, \
                                      (void**)&ptr, newvalue)

void do_autod_register(AutodRegistryNode *head, void **ptr, 
                                        autod_destructor_t destr);

void do_autod_assign(AutodRegistryNode *head, void **ptr, void *newvalue);

void do_autod_cleanup(AutodRegistryNode *head, void *ignore_ptr);


#endif


