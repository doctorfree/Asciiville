#include "autod.h"
#include <stdio.h>

void do_autod_register(AutodRegistryNode *head, void **ptr, 
                                        autod_destructor_t destr) {
   AutodRegistryNode *newnode = (AutodRegistryNode*) 
                                        malloc(sizeof(AutodRegistryNode));
   newnode->ptr = ptr;
   newnode->destructor = destr;
   newnode->next = head->next;
   head->next = newnode;

   *(ptr) = 0; /* zero the pointer to make sure it doesn't contain
                * garbage */
}

void do_autod_assign(AutodRegistryNode *head, void **ptr, void *newvalue) {
   AutodRegistryNode *n;

   if (*ptr) {
      /* destroy previous value before attributing a new value; for that,
       * we must look for the corresponding node in the registry */
      n = head->next;
      while (n) {
         if (n->ptr == ptr) {
            /* got it, call destructor */
            (*(n->destructor))(*ptr);
            break;
         }
      }

      if (!n) {
         /* not found: abort */
         fprintf(stderr, "autod: FATAL ERROR: attempt to assign to\n"
                         "unregistered pointer.\n\n");
         abort();
      }
   }

   /* we are clear to assign a new value */
   *ptr = newvalue;
}

void do_autod_cleanup(AutodRegistryNode *head, void *ignore_ptr) {
   AutodRegistryNode *n = head->next;
   AutodRegistryNode *next;

   while (n) {
      if (*(n->ptr) && *(n->ptr) != ignore_ptr) 
         (*(n->destructor))(*(n->ptr));  /* call destructor */
      
      next = n->next;
      free(n);  /* free the node */

      n = next;  /* advance to next */
   }
}

