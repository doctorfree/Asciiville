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

#include "view.h"

#ifndef ALIENS
#define ALIENS

typedef struct Aliens Aliens;

struct Aliens {
  int posX; // horizontal position of aliens
  int posY; // vertical position of aliens
  int right;
  int left;
  int bottom;
  int speed; // 0: no movement; 1: one per turn; etc.
};

extern Aliens aliens;

extern int shipnum;

#define ALIENS_MAX_NUMBER_X 10
#define ALIENS_MAX_NUMBER_Y 5
#define ALIENS_MAX_MISSILES 10

// todo: move to structure
extern int lowest_ship[ALIENS_MAX_NUMBER_X];
extern int alienshotx[ALIENS_MAX_MISSILES];
extern int alienshoty[ALIENS_MAX_MISSILES];
extern int alienshotnum;
extern int alienBlock[ALIENS_MAX_NUMBER_Y][ALIENS_MAX_NUMBER_X];
extern int bunker[BUNKERHEIGHT][BUNKERWIDTH + 1];

void aliensReset();
void bunkersReset();
int aliensMove();
int aliensMissileMove();
void render();
int aliensHitCheck(int shotx, int shoty);
int bunkersHitCheck(int shotx, int shoty);

// methods that handle graphic display, from view.c
extern void aliensDisplay(int x, int y, int wid, int hgt);
extern void aliensClear(int x, int y, int wid, int hgt);
extern void aliensRefresh(int level, int *pAliens);
extern void aliensMissileDisplay(int x, int y);
extern void aliensMissileClear(int x, int y);
extern void bunkersClearElement(int x, int y);
extern void bunkersClear();
extern void bunkersRefresh();

#endif
