#ifndef aewan_aewl_form_int_h
#define aewan_aewl_form_int_h

/* Form internals */

#include "form.h"
struct AewlForm_ {
   WINDOW *win;              /* the curses window that houses this form */
   AewlWidget *first_widget; /* first widget in form. Others follow in
                              * linked list */
   AewlWidget *last_widget;  /* tail of above linked list */
   AewlWidget *focus_widget; /* points to the widget that currently has
                              * focus */
};

#endif

