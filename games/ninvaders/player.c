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

#include "player.h"
#include "aliens.h"
#include "nInvaders.h"
#include "ufo.h"

typedef struct Player Player;

struct Player {
  int posX;         // horizontal position of player
  int posY;         // vertical position of player
  int speed;        // 0: no movement; 1: one per turn; etc.
  int missileFired; // 0: missile not running; 1: missile running
  int missileX;     // horizontal position of missile
  int missileY;     // vertical position of missile
};

Player player;

/**
 * initialize player attributes
 */
void playerReset() {
  // if missile was fired clear that position
  if (player.missileFired == 1) {
    playerMissileClear(player.missileX, player.missileY);
  }

  playerClear(player.posX, player.posY); // clear old position of player

  player.posY = PLAYERPOSY; // set vertical Position
  player.posX = 0;          // set horizontal Position
  player.speed = 1;
  player.missileFired = 0;
  player.missileX = 0;
  player.missileY = 0;

  playerDisplay(player.posX, player.posY); // display new position of player
}

/**
 * move player horizontally to position newPosX
 */
static void playerMove(int newPosX) {
  playerClear(player.posX, player.posY);   // clear sprite
  player.posX = newPosX;                   // make movement
  playerDisplay(player.posX, player.posY); // display sprite
}

/**
 * move player left
 */
void playerMoveLeft() {
  // check if space between player and border of screen is big enough
  if (player.posX > 0 + player.speed) {
    // desired movement is possible
    playerMove(player.posX - player.speed);
  } else {
    // space too small, move to left-most position
    playerMove(0);
  }
}

/**
 * move player right
 */
void playerMoveRight() {
  // check if space between player and border of screen is big enough
  if (player.posX < SCREENWIDTH - PLAYERWIDTH - player.speed) {
    // desired movement is possible
    playerMove(player.posX + player.speed);
  } else {
    // space too small, move to right-most position
    playerMove(SCREENWIDTH - PLAYERWIDTH);
  }
}

/**
 * toggle turbo mode on (if key is kept pressed)
 */
void playerTurboOn() { player.speed = 2; }

/**
 * toggle turbo mode off (if key is kept pressed)
 */
void playerTurboOff() { player.speed = 1; }

/**
 * player was hit by an alien shot
 */
int playerHitCheck(int hostileShotX, int hostileShotY) {
  int fPlayerWasHit = 0;

  // if shot reached vertikal position of player
  if (hostileShotY == PLAYERPOSY) {
    // if shot hits player
    if (hostileShotX >= player.posX &&
        hostileShotX <= player.posX + PLAYERWIDTH - 1) {
      fPlayerWasHit = 1; // set return value
    }
  }

  return fPlayerWasHit; // return if player was hit
}

/**
 * Launch Missile
 */
void playerLaunchMissile() {
  // only launch missile if no other is on its way
  if (player.missileFired == 0) {
    player.missileFired = 1; // missile is on its way
    player.missileX =
        player.posX + PLAYERWIDTH / 2; // launched from the middle of player...
    player.missileY = PLAYERPOSY;      // ...at same horizontal position
  }
}

/**
 * Reload Missile
 */
static void playerReloadMissile() {
  player.missileFired = 0; // no player missile flying
}

/**
 * move player missile and do alien/bunker hit testing
 * return value - 0: still some aliens left, 1: no aliens left
 */
int playerMoveMissile() {
  int fNoAliensLeft = 0; // return value
  int alienTypeHit = 0;

  // only do movements if there is a missile to move
  if (player.missileFired == 1) {

    playerMissileClear(player.missileX,
                       player.missileY); // clear old missile position
    playerDisplay(player.posX,
                  player.posY); // todo: check if this can be removed later if
                                // missiled is fired,
                                // the middle of player is cleared
    player.missileY--;          // move missile

    // if missile out of battlefield
    if (player.missileY < 0) {
      playerReloadMissile(); // reload missile
    } else {
      playerMissileDisplay(player.missileX,
                           player.missileY); // display missile at new position
    }

    // if missile hits an alien (TODO)
    alienTypeHit = aliensHitCheck(player.missileX, player.missileY);
    if (alienTypeHit != 0) {

      doScoring(alienTypeHit); // increase score
      playerReloadMissile();   // reload missile

      aliensClear(aliens.posX, aliens.posY, aliens.right,
                  aliens.bottom); // clear old posiiton of aliens

      render();
      if (shipnum == 0) {
        fNoAliensLeft = 1;
      }

      // speed of aliens
      weite = (shipnum + (skill_level * 10) - (level * 5) + 5) / 10;
      if (weite < 0) {
        weite = 0;
      }

      playerMissileClear(player.missileX,
                         player.missileY); // clear old missile position
      aliensDisplay(aliens.posX, aliens.posY, aliens.right,
                    aliens.bottom); // display aliens
    }

    // if missile hits a bunker
    if (bunkersHitCheck(player.missileX, player.missileY) == 1) {
      bunkersClearElement(player.missileX,
                          player.missileY); // clear element of bunker
      playerReloadMissile();                // reload player missile
    }

    // if missile hits ufo
    if (ufoHitCheck(player.missileX, player.missileY) == 1) {
      doScoring(UFO_ALIEN_TYPE);
      playerReloadMissile();
    }
  }

  return fNoAliensLeft;
}

/**
 * let player explode
 */
void playerExplode() {
  playerExplosionDisplay(player.posX, player.posY);
  playerDisplay(player.posX, player.posY);
}
