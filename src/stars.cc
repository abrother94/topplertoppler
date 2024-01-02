/* Tower Toppler - Nebulus
 * Copyright (C) 2000-2012  Andreas Röver
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

#include "stars.h"

#include "decl.h"
#include "sprites.h"
#include "screen.h"

#include <SDL.h>

#define starstep 5

typedef struct {
  long x, y;
  int state;
  int size;
} _star;

// sprite id of the first, the biggest star
static long star_spr_nr;

// the stars
static std::vector<_star> stars;

void sts_draw(void)
{
    for (auto & s : stars)
        scr_blit(objectsprites.data(star_spr_nr + s.size - ((s.state != 0) ? 1 : 0)), s.x, s.y);
}

void sts_init(int sn, int nstar)
{
  assert_msg(nstar > 1, "sts_init with too few stars!");

  stars.resize(nstar);

  for (auto & s : stars) {
    s.x = rand() / (RAND_MAX / SCREENWID) - SPR_STARWID;
    s.y = rand() / (RAND_MAX / SCREENHEI) - SPR_STARHEI;
    s.state = 0;
    s.size = rand() / (RAND_MAX / 7);
  }

  star_spr_nr = sn;
}

void sts_done(void)
{
}

void sts_blink(void)
{
    for (auto & s : stars)
        if (s.state > 0)
            s.state = (s.state + 1) % 4;
        else if (!(rand() & 0xff))
            s.state++;
}

void sts_move(long x, long y)
{
  for (auto & s : stars)
  {
    s.x += starstep * x;
    s.y += y;
    if (s.x > SCREENWID) {
      s.x = rand() / (RAND_MAX / starstep) - SPR_STARWID;
      s.y = rand() / (RAND_MAX / SCREENHEI);
    } else if (s.x < -SPR_STARWID) {
      s.x = SCREENWID - rand() / (RAND_MAX / starstep);
      s.y = rand() / (RAND_MAX / SCREENHEI);
    }
    if (s.y > SCREENHEI) {
      s.y = -SPR_STARHEI;
      s.x = rand() / (RAND_MAX / (SCREENWID + SPR_STARWID)) - SPR_STARWID;
    } else if (s.y < -SPR_STARHEI) {
      s.y = SCREENHEI;
      s.x = rand() / (RAND_MAX / (SCREENWID + SPR_STARWID)) - SPR_STARWID;
    }
  }
}

