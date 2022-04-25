/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Copyright 2020 Christoph Raitzig
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "terminal.h"

#include "config.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CURSES_H
#include <curses.h>
#endif

#ifdef FEAT_TERMLIB
#ifdef HAVE_TERM_H
#include <term.h>
#else
 #ifdef HAVE_NCURSES_TERM_H
 #include <ncurses/term.h>
 #endif
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif

#include <string.h>

int get_termsize(int* _width, int* _height, char** err) {
	static char errstr[1024];

	#ifndef WIN32
	char *termtype = getenv("TERM");
	char term_buffer[2048];
	int i;
	#endif

	errstr[0] = 0;

	if ( err != NULL )
		*err = errstr;

#ifdef FEAT_TERMLIB

#ifdef WIN32
	CONSOLE_SCREEN_BUFFER_INFO io;
	if ( !GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &io) ) {
		strcpy(errstr, "Could not determine console window size.");
		return -1;
	}
	*_width = io.srWindow.Right - io.srWindow.Left;
	*_height = io.srWindow.Bottom - io.srWindow.Top;
	return 1;
#else

	if ( !termtype ) {
		strcpy(errstr, "Environment variable TERM not set.");
		return -2;
	}

	i = tgetent(term_buffer, termtype);

	// There seems to be some confusion regarding the tgetent return
	// values.  The following two values should be swapped, according
	// to the man-pages, but on Mac OS X at least, they are like this.
	// I've also seen some indication of a bug in curses on USENET, so
	// I leave this one like this.

	if ( i == 0 ) {
		snprintf(errstr, sizeof(errstr)/sizeof(char) - 1,
			"Terminal type '%s' not recognized.", termtype);
		return 0;
	}

	if ( i < 0 ) {
		strcpy(errstr, "Could not access the termcap database.");
		return -1;
	}

	*_width = tgetnum("co");
	*_height = tgetnum("li");

	return 1;

#endif // non-WIN32

#else
	strcpy(errstr, "Compiled without termlib support.");
	return 0;

#endif // FEAT_TERMLIB
}

int supports_true_color() {
	char *colorterm = getenv("COLORTERM");
	if ( colorterm==NULL ) {
		return 0;
	}
	// some terminals have COLORTERM set, although true color is not supported
	if ( strcmp(colorterm, "rxvt")==0 ) {
		return 0;
	}
	return 1;
}
