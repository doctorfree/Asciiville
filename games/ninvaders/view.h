/**
 * nInvaders - a space invaders clone for ncurses
 * Copyright (C) 2002-2003 Dettus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * homepage: http://ninvaders.sourceforge.net
 * mailto: ninvaders-devel@lists.sourceforge.net
 *
 */

#include <ncurses.h>
#include <signal.h>
#include <unistd.h>

#ifndef VIEW
#define VIEW

#include "globals.h"

#define SCREENHEIGHT 24
#define SCREENWIDTH 80

#define PLAYERWIDTH 5
#define PLAYERPOSY (SCREENHEIGHT - 2)

#define BUNKERWIDTH 80
#define BUNKERHEIGHT 4
#define BUNKERX 0
#define BUNKERY (SCREENHEIGHT - 8)

#define ALIENS_MAX_NUMBER_X 10
#define ALIENS_MAX_NUMBER_Y 5

#define UFOWIDTH 5
#define UFOPOSY 0

void graphicEngineInit();

void aliensClear(int x, int y, int wid, int hgt);
void aliensDisplay(int x, int y, int wid, int hgt);
void aliensMissileClear(int x, int y);
void aliensMissileDisplay(int x, int y);
void aliensRefresh(int level, int *pAliens);
void battleFieldClear();
void bunkersClear();
void bunkersClearElement(int x, int y);
void bunkersDisplay(int *pBunker);
void gameOverDisplay();
void playerClear(int x, int y);
void playerDisplay(int x, int y);
void playerExplosionDisplay(int x, int y);
void playerMissileClear(int x, int y);
void playerMissileDisplay(int x, int y);
void titleScreenClear();
void titleScreenDisplay();
void ufoClear(int x, int y);
void ufoDisplay(int x, int y);
void ufoRefresh();

void statusDisplay(int level, int score, int lives);
void refreshScreen();

#endif
