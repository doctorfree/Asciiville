/* Copyright (C) 2001 by Alex Kompel <shurikk@pacbell.net> */
/* NetHack may be freely redistributed.  See license for details. */

/* font management functions */

#ifndef MSWINFont_h
#define MSWINFont_h

#include "winMS.h"

HGDIOBJ mswin_get_font(int win_type, int attr, HDC hdc, BOOL replace);
void mswin_init_splashfonts(HWND hWnd);
void mswin_destroy_splashfonts(void);
UINT mswin_charset(void);

#endif /* MSWINFont_h */
