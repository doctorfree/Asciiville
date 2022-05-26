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


#ifndef _btco_bores_kurses_h
#define _btco_bores_kurses_h

/* Color codes are 4-bit values. The bits are SBGR, where B, G and R are
 * the blue, green and red components, and the S bit is the bold attribute
 * for foreground colors, or the blink attribute for background colors.
 *
 * Refer to the implementation file for information about how color code
 * pairs are mapped to curses pairs */

/* Initializes curses, sets screen mode, initializes pairs, and so on.  */
void kurses_init();

/* Finalize curses */
void kurses_finalize();

/* Sets the active color to (fg, bg). Either can be -1, in which case
 * the previous value is kept. */
void kurses_color(int fg, int bg);

/* Same as kurses_color(attr >> 4, attr & 0x0F) */
void kurses_color_at(int attr);

/* Returns the screen width */
int kurses_width();

/* Returns the screen height */
int kurses_height();

/* Moves cursor to position (x, y). Returns true if motion was successful,
 * false if given coordinates are invalid */
int kurses_move(int x, int y);

/* Returns whether position x,y falls within screen bounds */
int kurses_pos_valid(int x, int y);

/* Reads a line of input from the keyboard, storing read characters in
 * <buf>. At most <buf_size> characters are stored in the buffer. 
 * Interprets the "backspace" and "kill" keys. Returns true if the
 * user ended input by pressing RETURN; false if the user cancelled
 * input by means of ESC, Ctrl+C or Ctrl+G. */
int kurses_line_input(char *buf, int buf_size);

/* Does the same as kurses_line_input, but with an extended feature: 
 * the array skeys of ints specifies special keys; when the user
 * presses any of those special keys, input will stop and the negative
 * of the value of the key will be returned. Character '\0' must
 * be at the end the skeys array. The flags parameter are an OR'ed
 * combination of the KURSES_LI_* flags defined below. 
 *
 * If the user cancels input, returns 0.
 * If the user confirms input without pressing any special key,
 * returns 1.
 */
int kurses_line_input_ex(char *buf, int buf_size, int* skeys, int flags);
         #define KURSES_LI_HIDE 1  /* echoes '*' instead of characters */

/* Draws a window with the specified position, dimensions and title.
 * Uses currently active color. After operation, cursor will be
 * placed on top-left character of CLIENT AREA of window. */
void draw_window(int x, int y, int w, int h, const char *title);

/* Draws a window centered onscreen. Returns in *x and *y the position
 * of the top-left character cell within the window's CLIENT AREA,
 * which is also where the cursor is located after the call. */
void draw_centered_window(int w, int h, const char *title, int *x, int *y);

/* Draws a horizontal row of <w> characters starting at position x0, y0.
 * The first character draw will be left_endpt; then character interim
 * will be drawn w-2 times, then the right_endpt will be drawn. */
void draw_hline(int x0, int y0, int w, 
                int left_endpt, int interim, int right_endpt);

/* Same idea as draw_hline, but draws a vertical line instead. */
void draw_vline(int x0, int y0, int h,
                int upper_endpt, int interim, int lower_endpt);

/* Saves a snapshot of the screen (stdscr) onto an internal stack.
 * That snapshot can be later restored through a call to
 * pop_screen */
void push_screen(void);

/* Restores a screen snapshot previously saved with push_screen.
 * Does NOT pop it from the stack: it will stay there in case
 * you want to restore it again. To discard it from the stack,
 * call pop_screen(void)
 *
 * Does NOT call refresh; you will have to do that yourself. */
void restore_screen(void);

/* Removes the top of the internal screen snapshot stack.
 * Does NOT paint the removed snapshot to the screen. If you
 * want that, call restore_screen() _before_ calling
 * pop_screen */
void pop_screen(void);


#endif

