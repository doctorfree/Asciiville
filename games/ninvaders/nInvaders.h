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

#include "globals.h"
#include "view.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef NINVADERS
#define NINVADERS

#define UFO_ALIEN_TYPE 0
#define RED_ALIEN_TYPE 1
#define GREEN_ALIEN_TYPE 2
#define BLUE_ALIEN_TYPE 3

int main(int argv, char **argc); // hey! it has to start somewhere!

extern void render(void);

void game_over(int a);
void drawscore();

void doScoring(int alienType);

// todo: let's try to not having to declare these "public"
extern int weite;
extern int level;
extern int skill_level;

// included from globals.h
extern void doSleep();
extern void showUsage();
extern void showVersion();
extern void showGplShort();

#endif
