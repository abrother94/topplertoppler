/* Tower Toppler - Nebulus
 * Copyright (C) 2000-2012  Andreas R�ver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "bonus.h"

#include "keyb.h"
#include "screen.h"
#include "menu.h"
#include "decl.h"
#include "points.h"
#include "level.h"
#include "sound.h"
#include "menusys.h"

#define fishcnt         16
#define gametime        500
#define scrollerspeed   2

/* position, status and movement direction
 * of all the fishes
 */
static struct {
   Sint32 x;
   Sint32 y;
   Sint32 state;
   Sint32 ydir;
} fish[fishcnt];

/* position of the submarine and the torpedo,
 * the state is taken from time
 */
static Sint32 torpedox, torpedoy, subposx, subposy;

/* current game time
 */
static Sint32 curtime;

/* current xposition, this is ongoing from tower to
 * tower so that you continue where you've left of in
 * the last bonus game
 */
static Uint32 xpos = 0;

/* this function displays everything of the bonus game */
static void show() {

  /* lets first calc the position of the tower on the screen
   * tower pos 0 places the tower smack in the middle
   * the tower moves with 4 per time tick to the left (negative)
   * after halve the game time we toggle to the new tower
   */
  Sint32 towerpos;

  if (curtime < gametime/2)
    // old tower just moving to the left
    towerpos = -(4*curtime);
  else
    // new tower when curtime == gametime the towerpos must be zero
    towerpos = -(4*curtime) + 4*gametime;

  /* draw the background layers */
  scr_draw_bonus1(xpos, towerpos);

  /* if the torpedo is visible, draw it */
  if (torpedox != -1)
    scr_draw_torpedo(torpedoy, torpedox);

  /* output the submarine */
  scr_draw_submarine(subposy - 20, subposx, curtime % 9);

  /* and the fishes */
  for (int b = 0; b < fishcnt; b++)
    if (fish[b].x >= -SPR_FISHWID)
      scr_draw_fish(fish[b].y, fish[b].x, fish[b].state);

  /* and finally the forground layers of the scroller */
  scr_draw_bonus2(xpos);
}

/* callback proc for menu drawing the current state and dim that picture */
static void bonus_background_proc() {
  show();
  scr_darkenscreen();
}

static bool escape() {

  set_men_bgproc(bonus_background_proc);
  return men_game();
}

static void bns_pause() {

  set_men_bgproc(bonus_background_proc);
  men_info(_("Pause"), -1, 1);
}

void bns_restart() { xpos = 0; }

bool bns_game() {

  /* game local x position, moved since the start of
   * this bonus game
   */
  Uint32 xpos_ofs = 0;

  /* when will the next fish appear */
  Uint32 nextfish = 30;

  /* when automatic is switched on the submarine will move
   * automatically to the tower base
   */
  bool automatic = false;

  /* the newtowercol is true, the towercolor has already been switched
   * to the color of the tower we're going to arrive at
   */
  bool newtowercol = false;

  Uint8 b;

  subposx = SUBM_TARGET_X;
  subposy = SUBM_TARGET_Y;

  /* no fished and no torpedo visible at game start */
  for (b = 0; b < fishcnt; b++)
    fish[b].x = -(SPR_FISHWID+1);
  torpedox = -1;

  /* restart timer */
  curtime = 0;

  key_readkey();

  do {

    /* move torpedo */
    if (torpedox >= 0) {
      torpedox += 8;
      if (torpedox > (640+SPR_TORPWID))
        torpedox = -1;
      for (b = 0; b < fishcnt; b++) {
        if (fish[b].x > 0 && fish[b].state >= 32) {
          if ((torpedox + SPR_TORPWID > fish[b].x) && (torpedox < fish[b].x + SPR_FISHWID) &&
              (torpedoy + SPR_TORPHEI > fish[b].y) && (torpedoy < fish[b].y + SPR_FISHHEI)) {
            torpedox = -1;
            fish[b].state -= 32;
            ttsounds::instance()->stopsound(SND_TORPEDO);
          }
        }
      }
    }

    /* move submarine */
    if (!automatic) {
      if (key_keypressed(fire_key)) {
        if (torpedox == -1) {
          torpedox = subposx + TORPEDO_OFS_X;
          torpedoy = subposy + TORPEDO_OFS_Y;
          ttsounds::instance()->startsound(SND_TORPEDO);
        }
      }

      if ((key_keystat() & down_key) != 0) {
        if (subposy < SUBM_MAX_Y)
          subposy += 4;
      } else {
        if ((key_keystat() & up_key) != 0) {
          if (subposy > SUBM_MIN_Y)
            subposy -= 4;
        }
      }

      if ((key_keystat() & left_key) != 0) {
        if (subposx > SUBM_MIN_X)
          subposx -= 8;
      } else {
        if ((key_keystat() & right_key) != 0) {
          if (subposx < SUBM_MAX_X)
            subposx += 4;
        }
      }
    } else {
      if (subposx > SUBM_TARGET_X)
        subposx -= 8;
      else if (subposx < SUBM_TARGET_X)
        subposx += 4;

      if (subposy > SUBM_TARGET_Y)
        subposy -= 4;
    }

    /* escape or pausekey pressed */
    if (key_keypressed(break_key))
      if (escape()) {
        return false;
      }

    if (key_keypressed(pause_key))
      bns_pause();

    key_readkey();

    /* move the fish */
    for (b = 0; b < fishcnt; b++) {
      if (fish[b].x >= -SPR_FISHWID) {
        fish[b].x -= 8;
        fish[b].y += fish[b].ydir;
        if (fish[b].y > 300 || fish[b].y < 80)
          fish[b].ydir = -fish[b].ydir;

        if (fish[b].state >= 32)
          fish[b].state = ((fish[b].state + 1) & 31) + 32;
        else
          fish[b].state = (fish[b].state + 1) & 31;

        if ((fish[b].state < 32) &&
            (fish[b].x > subposx - 40) &&
            (fish[b].x < subposx + 120) &&
            (fish[b].y > subposy - 40) &&
            (fish[b].y < subposy + 40)) {
          pts_add(50);
          ttsounds::instance()->startsound(SND_HIT);
          fish[b].x = - (SPR_FISHWID + 1);
        }
      }
    }

    /* nexfish handling */
    if (nextfish > 0)
      nextfish--;
    else {
      for (b = 0; b < fishcnt; b++) {
        if (fish[b].x < -SPR_FISHWID) {
          fish[b].x = SCREENWID;
          fish[b].y = rand() / (RAND_MAX / 140) + 120;
          fish[b].state = 32;
          do {
            fish[b].ydir = rand() / (RAND_MAX / 10) - 5;
          } while (fish[b].ydir == 0);
          nextfish = rand() / (RAND_MAX / 20) + 5;
          break;
        }
      }
    }

    /* change towercolor in the middle of the game */
    if ((curtime > gametime/2) && !newtowercol) {
      scr_settowercolor(lev_towercol_red(), lev_towercol_green(), lev_towercol_blue());
      newtowercol = true;
    }

    /* end of game, switch to automatic, stop scrolling */
    if (curtime == gametime) {
      automatic = true;
      if ((subposx == SUBM_TARGET_X) && (subposy == SUBM_TARGET_Y)) break;
    } else {
      xpos +=4;
      xpos_ofs += 4;
      curtime++;
    }

    if (!((curtime + 20) & 0x3f)) ttsounds::instance()->startsound(SND_SONAR);

    /* display screen and wait */
    show();
    scr_swap();
    ttsounds::instance()->play();
    dcl_wait();

  } while (true);

  return true;
}
