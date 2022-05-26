#include "debug_aewl.h"

#include "aewl/aewl.h"
#include "aewl/form.h"
#include "aewl/button.h"
#include "aewl/listbox.h"
#include "aewl/field.h"
#include "aewl/label.h"

#include "keys.h"

#include <ncurses.h>
#include <stdbool.h>

static int curses_attr_for_pair(int fg, int bg, bool bold) {
   short int cp = bg * 8 + 7 - fg;
   if (!cp) return bold ? A_BOLD : A_NORMAL;
   return COLOR_PAIR(cp) | (bold ? A_BOLD : 0);
}

void debug_aewl() {
   char *s = filedlg_show("Test Dialog");
   clear();

   printw("Answer: '%s'\n", s);
   getch();
}

void debug_aewl_OLD() {
   AewlWidget *b1, *b2, *b3, *lbl;
   AewlListBox *lb;
   AewlWidget *fld;
   AewlForm *f;
   int r;

   AewlAttrSettings ats = {
      {
         curses_attr_for_pair(7, 0, false), /* text */
         curses_attr_for_pair(7, 0, true),  /* text focus */
         curses_attr_for_pair(1, 0, false), /* text shortcut */
         curses_attr_for_pair(1, 0, true)   /* text shortcut focus */
      },

      {
         curses_attr_for_pair(7, 4, false), /* button */
         curses_attr_for_pair(7, 2, true),  /* button focus */
         curses_attr_for_pair(1, 4, false), /* button shortcut */
         curses_attr_for_pair(1, 2, true)   /* button shortcut focus */
      },

      {
         curses_attr_for_pair(7, 0, false), /* frame */
         curses_attr_for_pair(7, 0, true),  /* frame focus */
         curses_attr_for_pair(1, 0, false), /* frame shortcut */
         curses_attr_for_pair(1, 0, true)   /* frame shortcut focus */
      },

      {
         curses_attr_for_pair(7, 0, false), /* field */
         curses_attr_for_pair(7, 0, true),  /* field focus */
         curses_attr_for_pair(7, 4, false), /* field selection */
         curses_attr_for_pair(7, 1, true)   /* field selection focus */
      },

      curses_attr_for_pair(0, 0, true)      /* field pad char */
   };

   aewl_init(&ats);

   clear();
   refresh();
   f = aewl_form_create(stdscr);

   b1 = aewl_button_create(10, 10, 20, KEY_ALT_A, 1, "Button &Ay One");
   b2 = aewl_button_create(10, 12, 20, KEY_ALT_B, 2, "Button &Bee Two");
   b3 = aewl_button_create(10, 14, 20, KEY_ALT_C, 3, "Button &Cee Three");
   lb = (AewlListBox*) 
        aewl_listbox_create(40, 10, 30, 10, KEY_ALT_L, "My &List", 123);
   fld = aewl_field_create(10, 20, 20, KEY_ALT_T, 4, "Test field");
   lbl = aewl_label_create(4, 20, -1, "&Text:");

   aewl_listbox_add(lb, "Ut queant laxis");
   aewl_listbox_add(lb, "Resonare fibris");
   aewl_listbox_add(lb, "Mira gestorum");
   aewl_listbox_add(lb, "Famuli tuorum");
   aewl_listbox_add(lb, "Solve polluti");
   aewl_listbox_add(lb, "Labii reatum");
   aewl_listbox_add(lb, "Sancte Ioannes");
   aewl_listbox_add(lb, "Lacrymosa dies illa");
   aewl_listbox_add(lb, "Qua resurget ex favilla");
   aewl_listbox_add(lb, "Iudicandus homo reus");
   aewl_listbox_add(lb, "Huic ergo parce Deus");
   aewl_listbox_add(lb, "Pie iesu domine");
   aewl_listbox_add(lb, "Dona eis requiem");
   aewl_listbox_add(lb, "Stabat mater dolorosa");
   aewl_listbox_add(lb, "Iuxta crucem lacrymosam");
   aewl_listbox_add(lb, "Dum pendebat filius");
   aewl_listbox_add(lb, "Quis est homo qui non fleret");
   aewl_listbox_add(lb, "Matrem Christi se videret");
   aewl_listbox_add(lb, "In tanto suplicio?");

   aewl_form_add_widget(f, b1);
   aewl_form_add_widget(f, b2);
   aewl_form_add_widget(f, b3);
   aewl_form_add_widget(f, (AewlWidget*) lb);
   aewl_form_add_widget(f, fld);
   aewl_form_add_widget(f, lbl);
   
   aewl_form_paint(f, true);
   while (0 == (r = aewl_form_dispatch_key(f, getch())));
   aewl_form_destroy(f);

   clear();
   refresh();
   printw("Selected value %d", r);
   getch();
   
   clear();
   refresh();
}

