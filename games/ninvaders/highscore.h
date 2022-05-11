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

#ifndef HIGHSCORE
#define HIGHSCORE

/**************************************************************************************************
 *
 * public constants, variables, structures, declarations, macros
 *
 **************************************************************************************************/

#define SIZE_HIGHSCORE_NAME 8 /* number of letters for name        */
typedef struct highscore_entry {
  int score;                          /* score                             */
  char name[SIZE_HIGHSCORE_NAME + 1]; /* player name                       */
} HighScoreEntry;

#define MAX_HIGHSCORE_ENTRIES 10 /* number of highscore entries       */
typedef struct highscore {
  HighScoreEntry beginner[MAX_HIGHSCORE_ENTRIES]; /* beginner */
  HighScoreEntry normal[MAX_HIGHSCORE_ENTRIES];   /* normal   */
  HighScoreEntry expert[MAX_HIGHSCORE_ENTRIES];   /* expert   */
} HighScore;

HighScore readHighScore();
int writeHighScore(HighScore);
void addEntry(char *, int, int);

extern HighScore highscore; /* todo: not a global variable! */

#endif
