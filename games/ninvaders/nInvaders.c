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

#include "nInvaders.h"
#include "aliens.h"
#include "highscore.h"
#include "player.h"
#include "ufo.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define FPS 50

int lives;
long score;
int status; // status handled in timer

// todo: let's try to not having to declare these "public"
int weite;
int level;
int skill_level;

#define GAME_LOOP 1
#define GAME_NEXTLEVEL 2
#define GAME_PAUSED 3
#define GAME_OVER 4
#define GAME_EXIT 5
#define GAME_HIGHSCORE 6

/**
 * initialize level: reset attributes of most units
 */
static void initLevel() {
  playerReset();
  aliensReset();
  ufoReset();
  bunkersReset();
  render();
  drawscore();
}

/**
 * evaluate command line parameters
 */
static void evaluateCommandLine(int argc, char **argv) {

  // -l : set skill level
  if (argc == 3 && strcmp(argv[1], "-l") == 0) {
    if (argv[2][0] >= '0' && argv[2][0] <= '9') {
      skill_level = argv[2][0] - 48;
    } else {
      argc = 2;
    }
  }

  // -gpl : show GNU GPL
  if (argc == 2 && strcmp(argv[1], "-gpl") == 0) {
    showGplShort();
    exit(0);
  }

  // wrong command line: show usage
  if (argc == 2 || (argc == 3 && strcmp(argv[1], "-l") != 0)) {
    showVersion();
    showUsage();
    exit(1);
  }
}

static void finish(int sig) {
  endwin();
  showGplShort();

  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "============================================================"
                  "=============\n");
  fprintf(stderr, "\n");

  fprintf(stderr, "Final score: %7.7ld, Final level: %2.2d\nFinal rating... ",
          score, level);
  if (lives > 0)
    fprintf(stderr, "Quitter\n\n");
  else if (score < 5000)
    fprintf(stderr, "Alien Fodder\n\n");
  else if (score < 7500)
    fprintf(stderr, "Easy Target\n\n");
  else if (score < 10000)
    fprintf(stderr, "Barely Mediocre\n\n");
  else if (score < 12500)
    fprintf(stderr, "Shows Promise\n\n");
  else if (score < 15000)
    fprintf(stderr, "Alien Blaster\n\n");
  else if (score < 20000)
    fprintf(stderr, "Earth Defender\n\n");
  else if (score > 19999)
    fprintf(stderr, "Supreme Protector\n\n");

  showVersion();
  exit(0);
}

void drawscore() { statusDisplay(level, score, lives); }

/**
 * reads input from keyboard and do action
 */
void readInput() {
  int ch;
  static int lastmove;

  ch = getch(); // get key pressed

  switch (status) {

  case GAME_PAUSED:

    if (ch == 'p') {
      status = GAME_LOOP;
    }
    break;

  case GAME_HIGHSCORE:

    if (ch == ' ') {
      titleScreenClear();
      level = 0; // reset level
      score = 0; // reset score
      lives = 3; // restore lives
      status = GAME_NEXTLEVEL;
    } else if (ch == 'q') { // quit game
      status = GAME_EXIT;
    } else if (ch == 'a') { // DEBUG: add highscore entry
      addEntry("CHEATER!", 6666, 1);
    }
    break;

  case GAME_OVER:
    break; // don't do anything

  default:

    if (ch == 'l' || ch == KEY_RIGHT) { // move player right
      if (lastmove == 'l') {
        playerTurboOn(); // enable Turbo
      } else {
        playerTurboOff(); // disable Turbo
      }
      playerMoveRight(); // move player
      lastmove = 'l';    // remember last move for turbo mode
    } else if (ch == 'h' || ch == KEY_LEFT) { // move player left
      if (lastmove == 'h') {
        playerTurboOn(); // enable Turbo
      } else {
        playerTurboOff(); // disable Turbo
      }
      playerMoveLeft();                  // move player
      lastmove = 'h';                    // remember last move for turbo mode
    } else if (ch == 'k' || ch == ' ') { // shoot missile
      playerLaunchMissile();
    } else if (ch == 'p') { // pause game until 'p' pressed again
      // set status to game paused
      status = GAME_PAUSED;
    } else if (ch == 'W') { // cheat: goto next level
      status = GAME_NEXTLEVEL;
    } else if (ch == 'L') { // cheat: one more live
      lives++;
      drawscore();
    } else if (ch == 'q') { // quit game
      status = GAME_EXIT;
    } else { // disable turbo mode if key is not kept pressed
      lastmove = ' ';
    }

  } // switch
}

/**
 * timer
 * this method is executed every 1 / FPS seconds
 */
void handleTimer() {
  static int aliens_move_counter = 0;
  static int aliens_shot_counter = 0;
  static int player_shot_counter = 0;
  static int ufo_move_counter = 0;
  static int title_animation_counter = 0;
  static int game_over_counter = 0;

  switch (status) {

  case GAME_NEXTLEVEL: // go to next level

    level++; // increase level

    initLevel(); // initialize level

    aliens_move_counter = 0;
    aliens_shot_counter = 0;
    player_shot_counter = 0;
    ufo_move_counter = 0;

    weite = (shipnum + (skill_level * 10) - (level * 5) + 5) / 10;

    if (weite < 0) {
      weite = 0;
    }

    // change status and start game!
    status = GAME_LOOP;

  case GAME_LOOP: // do game handling

    // move aliens
    if (aliens_move_counter == 0 && aliensMove() == 1) {
      // aliens reached player
      lives = 0;
      status = GAME_OVER;
    }

    // move player missile
    if (player_shot_counter == 0 && playerMoveMissile() == 1) {
      // no aliens left
      status = GAME_NEXTLEVEL;
    }

    // move aliens' missiles
    if (aliens_shot_counter == 0 && aliensMissileMove() == 1) {
      // player was hit
      lives--;              // player looses one life
      drawscore();          // draw score
      playerExplode();      // display some explosion graphics
      if (lives == 0) {     // if no lives left ...
        status = GAME_OVER; // ... exit game
      }
    }

    // move ufo
    if (ufo_move_counter == 0 && ufoShowUfo() == 1) {
      ufoMoveLeft(); // move it one position to the left
    }

    if (aliens_shot_counter++ >= 5) {
      aliens_shot_counter = 0;
    } // speed of alien shot
    if (player_shot_counter++ >= 1) {
      player_shot_counter = 0;
    } // speed of player shot
    if (aliens_move_counter++ >= weite) {
      aliens_move_counter = 0;
    } // speed of aliend
    if (ufo_move_counter++ >= 3) {
      ufo_move_counter = 0;
    } // speed of ufo

    refreshScreen();
    break;

  case GAME_PAUSED: // game is paused
    break;

  case GAME_OVER: // game over
    if (game_over_counter == 100) {
      battleFieldClear();
      status = GAME_HIGHSCORE;
      game_over_counter = 0;
    } else {
      gameOverDisplay();
      game_over_counter++;
    }
    break;

  case GAME_EXIT: // exit game
    finish(0);
    break;

  case GAME_HIGHSCORE: // display highscore
    if (title_animation_counter == 0) {
      titleScreenDisplay();
    }

    if (title_animation_counter++ >= 6) {
      title_animation_counter = 0;
    } // speed of animation
    break;
  }
}

/**
 * set up timer
 */
void setUpTimer() {
  struct itimerval myTimer;
  struct sigaction myAction;
  myTimer.it_value.tv_sec = 0;
  myTimer.it_value.tv_usec = 1000000 / FPS;
  myTimer.it_interval.tv_sec = 0;
  myTimer.it_interval.tv_usec = 1000000 / FPS;
  setitimer(ITIMER_REAL, &myTimer, NULL);

  myAction.sa_handler = &handleTimer;
  myAction.sa_flags = SA_RESTART;
  sigaction(SIGALRM, &myAction, NULL);
}

int main(int argc, char **argv) {
  weite = 0;
  score = 0;
  lives = 3;
  level = 0;
  skill_level = 1;

  evaluateCommandLine(argc, argv); // evaluate command line parameters
  graphicEngineInit();             // initialize graphic engine

  // set up timer/ game handling
  setUpTimer();

  // load highscore and set start status
  highscore = readHighScore();
  status = GAME_HIGHSCORE;

  // read keyboard input
  do {
    // do movements and key-checking
    readInput();
  } while (0 == 0);

  // save highscore
  writeHighScore(highscore);

  return 0;
}

void doScoring(int alienType) {
  int points[4] = {500, 200, 150, 100}; // 0: ufo, 1:red, 2:green, 3:blue

  score += points[alienType]; // every alien type does different scoring points

  // every 6000 pts player gets a new live
  if (score % 6000 == 0) {
    lives++;
  }

  drawscore(); // display score
}
