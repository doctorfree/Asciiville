/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "curses.h"
#include "hack.h"
#include "wincurs.h"
#include "cursinvt.h"
#include "cursdial.h"

/* Permanent inventory for curses interface */

/* Runs when the game indicates that the inventory has been updated */
void
curses_update_inv(void)
{
    WINDOW *win = curses_get_nhwin(INV_WIN);

    /* Check if the inventory window is enabled in first place */
    if (!win) {
        /* It's not. Re-initialize the main windows if the
           option was enabled. */
        if (iflags.perm_invent) {
            curses_create_main_windows();
            curses_last_messages();
            doredraw();
        }
        return;
    }

    boolean border = curses_window_has_border(INV_WIN);

    /* Figure out drawing area */
    int x = 0;
    int y = 0;
    if (border) {
        x++;
        y++;
    }

    /* Clear the window as it is at the moment. */
    werase(win);

    /* The actual inventory will override this if we do carry stuff */
    wmove(win, y + 1, x);
    wprintw(win, "Not carrying anything");

    display_inventory(NULL, FALSE);

    if (border)
        box(win, 0, 0);

    wnoutrefresh(win);
}

/* Adds an inventory item. */
void
curses_add_inv(
    int y,              /* line index; 1..n rather than 0..n-1 */
    int glyph,          /* glyph to display with item */
    CHAR_P accelerator, /* selector letter for items, 0 for class headers */
    attr_t attr,        /* curses attribute for headers, 0 for items */
    const char *str)    /* formatted inventory item, without invlet prefix,
                         * or class header text */
{
    WINDOW *win = curses_get_nhwin(INV_WIN);
    int x, width, height, available_width;
    int border = curses_window_has_border(INV_WIN) ? 1 : 0;

    x = border; /* same for every line; 1 if border, 0 otherwise */

    curses_get_window_size(INV_WIN, &height, &width);

    /* exit early if we're outside the visible area */
    if (y > height + border) {
        return;
    }

    wmove(win, y - 1 + border, x);

    if (accelerator) {
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
        wprintw(win, " %c", accelerator);
        curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
        wprintw(win,  ") ");
    }

    if (accelerator && glyph != NO_GLYPH && !iflags.vanilla_ui_behavior) {
        unsigned dummy = 0; /* Not used */
        int color = 0;
        int symbol = 0;
        mapglyph(glyph, &symbol, &color, &dummy, 0, 0, 0);
        attr_t glyphclr = curses_color_attr(color, 0);
        wattron(win, glyphclr);
        wprintw(win, "%c ", symbol);
        wattroff(win, glyphclr);
    }

    int color = NO_COLOR;
#ifdef MENU_COLOR
    /* colorize categorizes */
    if (iflags.use_menu_color) {
        if (curses_get_menu_coloring(str, &color, &attr)) {
            attr = curses_convert_attr(attr);
        }
    }
#endif
    if (color == NO_COLOR) {
        color = NONE;
    }

    curses_menu_color_attr(win, color, attr, ON);
    wprintw(win, "%s", str);
    curses_menu_color_attr(win, color, attr, OFF);

    wclrtoeol(win);
}
