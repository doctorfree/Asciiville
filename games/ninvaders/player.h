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

#ifndef PLAYER
#define PLAYER

#include "view.h"

void playerReset();

void playerMoveLeft();
void playerMoveRight();
void playerTurboOn();
void playerTurboOff();
int playerHitCheck(int hostileShotX, int hostileShotY);
void playerLaunchMissile();
int playerMoveMissile();
void playerExplode();

// methods that handle graphic display, from view.c
extern void playerInit();
extern void playerDisplay(int x, int y);
extern void playerClear(int x, int y);
extern void playerMissileInit();
extern void playerMissileDisplay(int x, int y);
extern void playerMissileClear(int x, int y);
extern void playerExplosionDisplay(int x, int y);
extern void bunkersClearElement(int x, int y);

extern void doScoring(int alienType);

#endif
