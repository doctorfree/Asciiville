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
#include "globals.h"
#include "highscore.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define CYAN 5
#define MAGENTA 6
#define WHITE 7

WINDOW *wBattleField;
WINDOW *wEmpty;
WINDOW *wScores;

WINDOW *wPlayer;
WINDOW *wPlayerMissile;
WINDOW *wAliens;
WINDOW *wAliensMissile;
WINDOW *wBunkers;
WINDOW *wGameOver;
WINDOW *wUfo;
WINDOW *wStatus;
WINDOW *wTitleScreen;

/**
 * initialize player sprites
 */
static void playerInit() {
  wPlayer = newpad(1, PLAYERWIDTH);      // new pad with appropriate size
  wclear(wPlayer);                       // clear pad
  wattrset(wPlayer, COLOR_PAIR(YELLOW)); // set color
  waddstr(wPlayer, "/-^-\\");            // set sprite
}

/**
 * display player sprite
 */
void playerDisplay(int x, int y) {
  copywin(wPlayer, wBattleField, 0, 0, y, x, y, x + PLAYERWIDTH - 1, 0);
}

/**
 * clear player sprite
 */
void playerClear(int x, int y) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y, x + PLAYERWIDTH - 1, 0);
}

/**
 * initialize missile sprites
 */
static void playerMissileInit() {
  wPlayerMissile = newpad(1, 1);               // new pad with appropriate size
  wclear(wPlayerMissile);                      // clear pad
  wattrset(wPlayerMissile, COLOR_PAIR(WHITE)); // set color
  waddch(wPlayerMissile, '!');                 // set sprite
  wrefresh(wPlayerMissile);
}

/**
 * display missile sprite
 */
void playerMissileDisplay(int x, int y) {
  copywin(wPlayerMissile, wBattleField, 0, 0, y, x, y, x, 0);
}

/**
 * clear missile sprite
 */
void playerMissileClear(int x, int y) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y, x, 0);
}

/**
 * some explosion animation
 */
void playerExplosionDisplay(int x, int y) {
  WINDOW *wPlayerExplosion;
  char playerExplosionChars[16 + 1] = "@~`.,^#*-_=\\/%{}";
  int t, s;

  wPlayerExplosion = newpad(1, PLAYERWIDTH);      // new pad
  wattrset(wPlayerExplosion, COLOR_PAIR(YELLOW)); // set color

  for (t = 0; t < 5; t++) {   // 5 frames
    wclear(wPlayerExplosion); // clear pad
    for (s = 0; s < PLAYERWIDTH; s++) {
      waddch(wPlayerExplosion, playerExplosionChars[rand() % 16]); // sprite
    }

    copywin(wPlayerExplosion, wBattleField, 0, 0, y, x, y, x + PLAYERWIDTH - 1,
            0);             // display explostion
    wrefresh(wBattleField); // refresh battelfield to display explosion frames
    doSleep(100000);        // play animation not too fast
  }

  delwin(wPlayerExplosion);

} // todo: kann man bestimmt noch besser machen.

/**
 * initialize aliens sprites
 */
static void aliensInit() {
  wAliens = newpad(ALIENS_MAX_NUMBER_Y * 2, ALIENS_MAX_NUMBER_X * 3);
  wclear(wAliens);
}

/**
 * display aliens sprite
 */
void aliensDisplay(int x, int y, int wid, int hgt) {
  copywin(wAliens, wBattleField, 0, 0, y, x, y + hgt, x + wid + 2, 0);
}

/**
 * clear aliens sprite
 */
void aliensClear(int x, int y, int wid, int hgt) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y + hgt, x + wid + 2, 0);
}

/**
 * initialize missile sprites
 */
static void aliensMissileInit() {
  wAliensMissile = newpad(1, 1);              // new pad
  wclear(wAliensMissile);                     // clear pad
  wattrset(wAliensMissile, COLOR_PAIR(CYAN)); // set color
  waddch(wAliensMissile, ':');                // set sprite
}

/**
 * display missile sprite
 */
void aliensMissileDisplay(int x, int y) {
  copywin(wAliensMissile, wBattleField, 0, 0, y, x, y, x, 0);
}

/**
 * clear missile sprite
 */
void aliensMissileClear(int x, int y) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y, x, 0);
}

/**
 * refresh aliens sprite
 */
void aliensRefresh(int level, int *pAliens) {
  static int frame =
      0; // used for animation; mod 2 == 0: frame1, mod2 == 1: frame2
  int k, row;
  int c = 0;
  int alienType = 0;
  char ships[2][9][3 + 1] = {
      {",^,", "_O-", "-o-", "o=o", "<O>", "_x_", "*^*", "\\_/", "o o"},
      {".-.", "-O_", "/o\\", "o-o", "<o>", "-x-", "o^o", "/~\\", "oo "}};
  int colors[9] = {RED, GREEN, BLUE, RED, YELLOW, WHITE, WHITE, YELLOW, RED};

  wclear(wAliens);                    // clear pad
  wattrset(wAliens, COLOR_PAIR(RED)); // set color

  frame++; // next frame

  // draw alien if there is one
  for (row = 0; row < ALIENS_MAX_NUMBER_Y * 2; row++) {
    for (k = 0; k < ALIENS_MAX_NUMBER_X; k++) {
      if ((row % 2) == 0) { // display aliens every even row
        alienType = *(pAliens + c * (ALIENS_MAX_NUMBER_X) +
                      k); // get type of alien //alienBlock[c][k]

        if (alienType != 0) { // if there is an alien to display
          wattrset(wAliens, COLOR_PAIR(colors[alienType - 1])); // set color
          waddch(wAliens,
                 ships[frame % 2][alienType - 1 + (3 * ((level - 1) % 3))]
                      [0]); // set char1
          waddch(wAliens,
                 ships[frame % 2][alienType - 1 + (3 * ((level - 1) % 3))]
                      [1]); // set char2
          waddch(wAliens,
                 ships[frame % 2][alienType - 1 + (3 * ((level - 1) % 3))]
                      [2]); // set char3
          if (alienType > 4) {
            *(pAliens + c * ALIENS_MAX_NUMBER_X + k) = (alienType + 1) % 9;
          } // todo: what's that? If alien_type > 4 then do a modulo
            // operation???
        } else {
          waddstr(wAliens, "   "); // no alien
        }

      } else {
        waddstr(wAliens, "   "); // no aliens at odd rows
      }
    }
    if ((row % 2) == 1) {
      c++;
    } // goto next row at alienblock
  }
}

/**
 * initialize bunkers sprites
 */
static void bunkersInit() {
  wBunkers = newpad(BUNKERHEIGHT, BUNKERWIDTH); // new pad data
  wclear(wBunkers);
}

/**
 * display bunkers sprite
 * needs pointer to bunker-array
 */
void bunkersDisplay(int *pBunker) {
  int l, k;
  wclear(wBunkers);
  wattrset(wBunkers, COLOR_PAIR(CYAN));
  for (l = 0; l < BUNKERHEIGHT; l++) {
    for (k = 0; k < BUNKERWIDTH; k++) {
      if (*(pBunker + (l * (BUNKERWIDTH + 1)) + k) ==
          1) { // if (pBunker[l][k]==1) {
        waddch(wBunkers, '#');
      } else {
        waddch(wBunkers, ' ');
      }
    }
  }

  copywin(wBunkers, wBattleField, 0, 0, BUNKERY, BUNKERX,
          BUNKERY + BUNKERHEIGHT - 1, BUNKERX + BUNKERWIDTH - 1, 0);
}

/**
 * clear bunkers sprite
 */
void bunkersClear() {
  copywin(wEmpty, wBattleField, 0, 0, BUNKERY, BUNKERX,
          BUNKERY + BUNKERHEIGHT - 1, BUNKERX + BUNKERWIDTH - 1, 0);
}

/**
 * clear one element of bunkers sprite at position (x, y)
 */
void bunkersClearElement(int x, int y) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y, x, 0);
}

/**
 * set actual sprite for ufo animation
 */
void ufoRefresh() {
  char ufo[4][6] = {"<o o>", "<oo >", "<o o>", "< oo>"};
  static int frame = 0;

  wclear(wUfo);
  wattrset(wUfo, COLOR_PAIR(MAGENTA));
  waddstr(wUfo, ufo[frame % 4]);

  frame++;
}

/**
 * initialize ufo sprite
 */
static void ufoInit() {
  wUfo = newpad(1, UFOWIDTH);          // new pad with appropriate size
  wclear(wUfo);                        // clear pad
  wattrset(wUfo, COLOR_PAIR(MAGENTA)); // set color
}

/**
 * display ufo sprite
 */
void ufoDisplay(int x, int y) {
  copywin(wUfo, wBattleField, 0, 0, y, x, y, x + UFOWIDTH - 1, 0);
}

/**
 * clear ufo sprite
 */
void ufoClear(int x, int y) {
  copywin(wEmpty, wBattleField, 0, 0, y, x, y, x + UFOWIDTH - 1, 0);
}

/**
 * initialize gameover graphic
 */
static void gameOverInit() {
  // init game-over banner
  wGameOver = newpad(13, 31);
  wclear(wGameOver);
  wattrset(wGameOver, COLOR_PAIR(WHITE));
  waddstr(wGameOver, "                               ");
  waddstr(wGameOver, "  #####   ####  ##   ## ###### ");
  waddstr(wGameOver, " ##      ##  ## ####### ##     ");
  waddstr(wGameOver, " ## ###  ###### ## # ## #####  ");
  waddstr(wGameOver, " ##  ##  ##  ## ##   ## ##     ");
  waddstr(wGameOver, "  #####  ##  ## ##   ## ###### ");
  waddstr(wGameOver, "                               ");
  waddstr(wGameOver, "  ####  ##   ## ###### ######  ");
  waddstr(wGameOver, " ##  ## ##   ## ##     ##   ## ");
  waddstr(wGameOver, " ##  ##  ## ##  #####  ######  ");
  waddstr(wGameOver, " ##  ##  ## ##  ##     ##  ##  ");
  waddstr(wGameOver, "  ####    ###   ###### ##   ## ");
  waddstr(wGameOver, "                               ");
}

/**
 * display game over graphic
 */
void gameOverDisplay() {
  int x = (SCREENWIDTH / 2) - (31 / 2);
  int y = (SCREENHEIGHT / 2) - (13 / 2);
  copywin(wGameOver, wBattleField, 0, 0, y, x, y + 12, x + 30, 0);
  wrefresh(wBattleField);
}

/**
 * initialize title screen
 */
static void titleScreenInit() {
  wTitleScreen = newpad(SCREENHEIGHT, SCREENWIDTH);
  wclear(wTitleScreen);
}

/**
 * display title screen
 */
void titleScreenDisplay() {
  static int frame = 0;
  int x, y;
  int i;
  WINDOW *wTitleText;
  WINDOW *wAliensTitle;
  WINDOW *wHighscore;
  WINDOW *wStartText;
  char ufo[4][6] = {"<o o>", "<oo >", "<o o>", "< oo>"};
  char aliens[2][9][3 + 1] = {
      {",^,", "_O-", "-o-", "o=o", "<O>", "_x_", "*^*", "\\_/", "o o"},
      {".-.", "-O_", "/o\\", "o-o", "<o>", "-x-", "o^o", "/~\\", "oo "}};
  int score[3] = {200, 150, 100};
  int colors[9] = {RED, GREEN, BLUE, RED, GREEN, BLUE, RED, GREEN, BLUE};
  char buffer[12];
  char highscoreBuffer[25];
  static int alien_type = 0;
  static int fShowHighscore = 0;

  /* toggle between different alien types and highscores */
  frame = (frame + 1) % 180;
  if (frame == 0) {
    alien_type = 0;
    fShowHighscore = 0;
  } else if (frame == 30) {
    fShowHighscore = 1;
  } else if (frame == 60) {
    alien_type = 3;
    fShowHighscore = 0;
  } else if (frame == 90) {
    fShowHighscore = 1;
  } else if (frame == 120) {
    alien_type = 6;
    fShowHighscore = 0;
  } else if (frame == 150) {
    fShowHighscore = 1;
  }

  /* big title */
  wTitleText = newpad(4, 41);
  wclear(wTitleText);
  wattrset(wTitleText, COLOR_PAIR(YELLOW));
  waddstr(wTitleText, "        ____                 __          ");
  waddstr(wTitleText, "  ___  /  _/__ _  _____  ___/ /__ _______");
  waddstr(wTitleText, " / _ \\_/ // _ \\ |/ / _ `/ _  / -_) __(_-<");
  waddstr(wTitleText, "/_//_/___/_//_/___/\\_,_/\\_,_/\\__/_/ /___/");

  wAliensTitle = newpad(7, 11);
  wHighscore = newpad(12, 24);
  if (fShowHighscore == 0) {

    /* alien animation with score */

    wclear(wAliensTitle);
    snprintf(buffer, sizeof(buffer), "%s = 500", ufo[frame % 4]);
    wattrset(wAliensTitle, COLOR_PAIR(MAGENTA));
    waddstr(wAliensTitle, buffer); // print ufo

    for (i = alien_type; i < alien_type + 3; i++) {
      waddstr(wAliensTitle, "           ");
      snprintf(buffer, sizeof(buffer), "%s   = %d", aliens[frame % 2][i],
               score[i % 3]);
      wattrset(wAliensTitle, COLOR_PAIR(colors[i]));
      waddstr(wAliensTitle, buffer); // print aliens
    }

  } else {

    /* highscore */
    wclear(wHighscore);
    wattrset(wHighscore, COLOR_PAIR(WHITE));
    waddstr(wHighscore, "         TOP 10         ");
    waddstr(wHighscore, "                        ");
    for (i = 0; i < MAX_HIGHSCORE_ENTRIES; i++) {
      snprintf(highscoreBuffer, sizeof(highscoreBuffer), "%s.........%2.7d",
               highscore.normal[i].name, highscore.normal[i].score);
      waddstr(wHighscore, highscoreBuffer);
    }
  }

  /* info text */
  wStartText = newpad(1, 20);
  wclear(wStartText);
  wattrset(wStartText, COLOR_PAIR(RED));
  waddstr(wStartText, "Press SPACE to start");

  /* place title, aliens and text on wTitleScreen */
  x = (SCREENWIDTH / 2) - (41 / 2);
  y = 0;
  copywin(wTitleText, wTitleScreen, 0, 0, y, x, y + 3, x + 40, 0);

  if (fShowHighscore == 0) {
    x = (SCREENWIDTH / 2) - (24 / 2);
    y = 6;
    copywin(wEmpty, wTitleScreen, 0, 0, y, x, y + 11, x + 23, 0);

    x = (SCREENWIDTH / 2) - (11 / 2);
    y = 8;
    copywin(wAliensTitle, wTitleScreen, 0, 0, y, x, y + 6, x + 10, 0);
  } else {
    x = (SCREENWIDTH / 2) - (11 / 2);
    y = 8;
    copywin(wEmpty, wTitleScreen, 0, 0, y, x, y + 6, x + 10, 0);

    x = (SCREENWIDTH / 2) - (24 / 2);
    y = 6;
    copywin(wHighscore, wTitleScreen, 0, 0, y, x, y + 11, x + 23, 0);
  }

  x = (SCREENWIDTH / 2) - (20 / 2);
  y = SCREENHEIGHT - 2;
  copywin(wStartText, wTitleScreen, 0, 0, y, x, y, x + 19, 0);

  copywin(wTitleScreen, wBattleField, 0, 0, 0, 0, SCREENHEIGHT - 1,
          SCREENWIDTH - 1, 0);

  wrefresh(wBattleField);

  delwin(wTitleText);
  delwin(wAliensTitle);
  delwin(wHighscore);
  delwin(wStartText);
}

/**
 * clear title screen
 */
void titleScreenClear() { battleFieldClear(); }

/**
 * initialize scores
 */
void statusInit() {
  wStatus = newpad(1, 55);
  wclear(wStatus);
}

/**
 * display scores
 */
void statusDisplay(int level, int score, int lives) {
  int t, xOffset;
  char strStatus[55];
  // "Level: 01 Score: 0001450 Lives: /-\ /-\ /-\ /-\ /-\ "
  // "1234567890123456789012345678901234567890123456789012"

  xOffset = (SCREENWIDTH / 2) - 24;

  sprintf(strStatus, "Level: %2.2d Score: %2.7d Lives: ", level, score);

  wclear(wStatus);
  wattrset(wStatus, COLOR_PAIR(RED));
  waddstr(wStatus, strStatus);

  // show maximal five lives
  for (t = 1; ((t <= 5) && (t < lives)); t++) {
    waddstr(wStatus, "/-\\ ");
  }

  copywin(wStatus, wBattleField, 0, 0, SCREENHEIGHT - 1, xOffset,
          SCREENHEIGHT - 1, xOffset + 54, 0);
}

/**
 * initialize battlefield
 */
static void battleFieldInit() {
  wEmpty = newpad(SCREENHEIGHT, SCREENWIDTH);
  wclear(wEmpty);

  wBattleField = newwin(SCREENHEIGHT, SCREENWIDTH, 0, 0); // new window
  wclear(wBattleField);                                   // clear window
  mvwin(wBattleField, 0, 0);                              // move window
}

/**
 * clear battlefield
 */
void battleFieldClear() {
  copywin(wEmpty, wBattleField, 0, 0, 0, 0, SCREENHEIGHT - 1, SCREENWIDTH - 1,
          0);
}

/**
 * refresh screen so that modified graphic buffers get visible
 */
void refreshScreen() {
  redrawwin(wBattleField); // needed to display graphics properly at startup on
                           // some terminals
  wrefresh(wBattleField);
}

/**
 * do proper cleanup
 */
static void finish(int sig) {
  delwin(wBattleField);
  delwin(wEmpty);
  delwin(wScores);

  delwin(wPlayer);
  delwin(wPlayerMissile);
  delwin(wAliens);
  delwin(wAliensMissile);
  delwin(wBunkers);
  delwin(wGameOver);
  delwin(wUfo);
  delwin(wStatus);
  delwin(wTitleScreen);

  endwin(); // <curses.h> reset terminal into proper non-visual mode
  exit(0);
}

/**
 * initialize n_courses
 */
void graphicEngineInit() {
  (void)signal(SIGINT,
               finish); // <signal.h> on signal "SIGINT" call method "finish"
  (void)initscr();      // <curses.h> do initialization work
  keypad(stdscr, TRUE); // <curses.h> enable keypad for input
  (void)nonl(); // <curses.h> disable translation return/ newline for detection
                // of return key
  (void)cbreak(); // <curses.h> do not buffer typed characters, use at once
  (void)noecho(); // <curses.h> do not echo typed characters
  start_color();  // <curses.h> use colors
  init_pair(RED, COLOR_RED, COLOR_BLACK);       // <curses.h> define color-pair
  init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);   // <curses.h> define color-pair
  init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK); // <curses.h> define color-pair
  init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);     // <curses.h> define color-pair
  init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);     // <curses.h> define color-pair
  init_pair(MAGENTA, COLOR_MAGENTA,
            COLOR_BLACK);                     // <curses.h> define color-pair
  init_pair(WHITE, COLOR_WHITE, COLOR_BLACK); // <curses.h> define color-pair

  // timeout(0); 			// <curses.h> do not wait for input
  curs_set(0); // <curses.h> do not show annoying cursor jumping everywhere

  // initialize sprites
  battleFieldInit();
  playerInit();
  playerMissileInit();
  aliensInit();
  aliensMissileInit();
  bunkersInit();
  ufoInit();
  gameOverInit();
  statusInit();
  titleScreenInit();
}
