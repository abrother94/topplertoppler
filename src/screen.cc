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
#define _USE_MATH_DEFINES
#include "screen.h"

#include "archi.h"
#include "sprites.h"
#include "robots.h"
#include "stars.h"
#include "points.h"
#include "toppler.h"
#include "snowball.h"
#include "level.h"
#include "decl.h"
#include "keyb.h"
#include "configuration.h"

#include <algorithm>

static SDL_Surface *display = nullptr;
static SDL_Window *sdlWindow = nullptr;
static SDL_Renderer *sdlRenderer = nullptr;
static SDL_Texture *sdlTexture = nullptr;


static int color_ramp_radj = 3;
static int color_ramp_gadj = 5;
static int color_ramp_badj = 7;

static Uint8 *slicedata, *battlementdata, *crossdata;

static int slicestart;
static int battlementstart;

static Uint8 robotcount;

typedef struct {
  Uint8 count;            // how many pictures are in the animation of this robot
  unsigned short start;   // number of first robot
} robot_data;

robot_data * robots;      // array with robot data, robotcount contains size

static unsigned short ballst, boxst, snowballst, starst, crossst,
         fishst, subst, torb;
static int topplerstart;

static std::vector<std::tuple<int, int, SDL_Surface *>> circleShadowCache;
static std::vector<std::tuple<int, int, SDL_Surface *>> rectangularShadowCache;

static unsigned short  step, elevatorsprite, stick;

/* table used to calculate the distance of an object from the center of the
 tower that is at x degrees on the tower */
static int sintab[TOWER_ANGLES];

/* this table is used for the waves of the water */
static Sint8 waves[0x80];

/* this value added to the start of the animal sprites leads to
 the mirrored ones */
#define mirror          37

/* the state of the flashing boxes */
static int boxstate;

static struct {
  int xstart;          // x start position
  int width;           // width of door
  unsigned short s[3]; // the sprite index for the 3 layers of the door
  Uint8 *data[3];      // the data for the 3 layers of the door (pixel info for recoloring)
} doors[73];

#define MAXCHARNUM 256*256

static struct {
  unsigned short s;
  unsigned char width;
} fontchars[MAXCHARNUM];

/* bonus game scrolling layer */
typedef struct  {
  long xpos, ypos;    // position of the layer
  long xrepeat;       // width where the image repeats (a bit redundant to width, but theoretically you could have a gap)
  long width, height; // size of the layer
  int  num, den;      // speed
  Uint16 image;
} _scroll_layer;

/* # of scrolling layers in the bonus game */
static int num_scrolllayers;
/* tower layering depth and scrolling speed in the bonus game */
static int sl_tower_depth,
           sl_tower_num,
           sl_tower_den;
static _scroll_layer *scroll_layers;

Uint8 towerpal[2*256];
Uint8 crosspal[2*256];

Uint8 last_towercol_r, last_towercol_g, last_towercol_b;

void color_ramp1(int *c, int *adj, int min, int max) {
  *c = *c + *adj;
  if (*c > max - abs(*adj)) {
    *c = max;
    *adj = -(*adj);
  } else if (*c < min + abs(*adj)) {
    *c = min;
    *adj = -(*adj);
  }
}

void scr_color_ramp(int *r, int *g, int *b) {
  color_ramp1(r, &color_ramp_radj, 1, 255);
  color_ramp1(g, &color_ramp_gadj, 1, 255);
  color_ramp1(b, &color_ramp_badj, 1, 255);
}

void
scr_savedisplaybmp(char *fname)
{
  SDL_SaveBMP(display, fname);
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16 *)p = pixel;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
        } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
  }
}



Uint16 scr_loadsprites(spritecontainer *spr, archive::file * fi, int num, int w, int h, bool sprite, const Uint8 *pal, bool use_alpha) {
  Uint16 erg = 0;
  Uint8 b, a;
  SDL_Surface *z;
  Uint32 pixel;

  for (int t = 0; t < num; t++) {
    z = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    if (sprite & !use_alpha)
      SDL_SetColorKey(z, SDL_TRUE, SDL_MapRGB(z->format, 1, 1, 1));

    for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++) {
        b = fi->getbyte();
        pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]);
        if (sprite) {
          a = fi->getbyte();
          if (use_alpha) {
            pixel=SDL_MapRGBA(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2],a);
          } else {
            if (a<128)
              pixel=SDL_MapRGB(z->format,1,1,1);
            else
            {
              if ((pal[b*3+2] == 1) && (pal[b*3+1] == 1) && (pal[b*3] == 1))
                pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]+1);
              else
                /* ok, this is the case where we have a sprite and don't want
                 to use alpha blending, so we use normal sprites with key color
                 instead, this is much faster. So if the pixel is more than 50% transparent
                 make the whole pixel transparent by setting this pixel to the
                 key color. if the pixel is not supoosed to be transparent
                 we need to check if the pixel color is by accident the key color,
                 if so we alter is slightly */
                pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]);
            }
          }
        }
        putpixel(z,x,y,pixel);
      }

    if (t == 0)
      erg = spr->save(z);
    else
      spr->save(z);
  }

  return erg;
}


static Uint16 scr_gensprites(spritecontainer *spr, int num, int w, int h, bool sprite, bool use_alpha) {
  Uint16 erg = 0;
  SDL_Surface *z;

  for (int t = 0; t < num; t++) {
    z = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    if (sprite & !use_alpha)
      /* SDL_RLEACCEL is not allowed here, because we need to edit the data later
       on for the new colors */
      SDL_SetColorKey(z, SDL_TRUE, SDL_MapRGBA(z->format, 1, 1, 1, 0));

    if (t == 0)
      erg = spr->save(z);
    else
      spr->save(z);
  }

  return erg;
}

static void scr_regensprites(Uint8 *data, SDL_Surface * const target, int num, int w, int h, bool sprite, const Uint8 *pal, bool use_alpha, bool screenformat) {
  Uint8 a, b;
  Uint32 datapos = 0;
  SDL_Surface * z;
  Uint32 pixel;

  if (screenformat) {
    z = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  } else
    z = target;

  for (int t = 0; t < num; t++)
    for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++) {
        b = data[datapos++];
        pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]);
        if (sprite) {
          a = data[datapos++];
          if (use_alpha) {
            pixel=SDL_MapRGBA(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2],a);
          } else {
            if (a<128)
              pixel=SDL_MapRGB(z->format,1,1,1);
            else
            {
              if (((pal[b*3+2] == 1) && (pal[b*3+1] == 1)) || (pal[b*3] == 1))
                pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]+1);
              else
                /* ok, this is the case where we have a sprite and don't want
                 to use alpha blending, so we use normal sprites with key color
                 instead, this is much faster. So if the pixel is more than 50% transparent
                 make the whole pixel transparent by setting this pixel to the
                 key color. if the pixel is not supoosed to be transparent
                 we need to check if the pixel color is by accident the key color,
                 if so we alter is slightly */
                pixel=SDL_MapRGB(z->format,pal[b*3 + 0],pal[b*3 + 1],pal[b*3 + 2]);
            }
          }
        }
        putpixel(z,x,y,pixel);
      }
  if (screenformat) {
    SDL_Rect r;
    r.h = h;
    r.w = w;
    r.x = 0;
    r.y = 0;
    SDL_BlitSurface(z, &r, target, &r);

    SDL_FreeSurface(z);
  }
}

void scr_read_palette(archive::file * fi, Uint8 *pal) {
  Uint8 b;
  b = fi->getbyte();

  fi->read(pal, (Uint32)b*3+3);
}


/* loads all the graphics */
static void loadgraphics(Uint8 what) {

  unsigned char pal[3*256];
  int t;

  if (what == 0xff) {

    auto fi = dataarchive->open(grafdat);

    fi.read(towerpal, 2*256);

    slicedata = (Uint8*)malloc(SPR_SLICESPRITES * SPR_SLICEWID * SPR_SLICEHEI);
    fi.read(slicedata, SPR_SLICESPRITES * SPR_SLICEWID * SPR_SLICEHEI);

    battlementdata = (Uint8*)malloc(SPR_BATTLFRAMES * SPR_BATTLWID * SPR_BATTLHEI);
    fi.read(battlementdata, SPR_BATTLFRAMES * SPR_BATTLWID * SPR_BATTLHEI);

    slicestart = scr_gensprites(&restsprites, SPR_SLICESPRITES, SPR_SLICEWID, SPR_SLICEHEI, false, false);
    battlementstart = scr_gensprites(&restsprites, SPR_BATTLFRAMES, SPR_BATTLWID, SPR_BATTLHEI, false, false);

    for (t = -36; t < 37; t++) {

      doors[t+36].xstart = (Sint16)fi.getword();
      doors[t+36].width = fi.getword();

      for (int et = 0; et < 3; et++)
        if (doors[t+36].width != 0) {
          doors[t+36].s[et] = scr_gensprites(&restsprites, 1, doors[t+36].width, 16, false, false);
          doors[t+36].data[et] = (Uint8*)malloc(doors[t+36].width*16);
          fi.read(doors[t+36].data[et], doors[t+36].width*16);
        } else {
          doors[t+36].s[et] = 0;
          doors[t+36].data[et] = NULL;
        }
    }

    for (t = 0; t < 256; t++) {
      unsigned char c1, c2;

      c1 = fi.getbyte();
      c2 = fi.getbyte();

      pal[3*t] = c1;
      pal[3*t+1] = c2;
      pal[3*t+2] = c2;
    }

    step = scr_loadsprites(&restsprites, &fi, SPR_STEPFRAMES, SPR_STEPWID, SPR_STEPHEI, false, pal, false);
    elevatorsprite = scr_loadsprites(&restsprites, &fi, SPR_ELEVAFRAMES, SPR_ELEVAWID, SPR_ELEVAHEI, false, pal, false);
    stick = scr_loadsprites(&restsprites, &fi, 1, SPR_STICKWID, SPR_STICKHEI, false, pal, false);
  }

  {
    auto fi = dataarchive->open(topplerdat);

    scr_read_palette(&fi, pal);

    topplerstart = scr_loadsprites(&objectsprites, &fi, 74, SPR_HEROWID, SPR_HEROHEI, true, pal, config.use_alpha_sprites());
  }

  {
    auto fi = dataarchive->open(spritedat);

    scr_read_palette(&fi, pal);

    robotcount = fi.getbyte();

    robots = new robot_data[robotcount];

    for (t = 0; t < 8; t++) {
      robots[t].count = fi.getbyte();
      robots[t].start = scr_loadsprites(&objectsprites, &fi, robots[t].count, SPR_ROBOTWID, SPR_ROBOTHEI, true, pal, config.use_alpha_sprites());
    }

    scr_read_palette(&fi, pal);
    ballst = scr_loadsprites(&objectsprites, &fi, 2, SPR_ROBOTWID, SPR_ROBOTHEI, true, pal, config.use_alpha_sprites());

    scr_read_palette(&fi, pal);
    boxst = scr_loadsprites(&objectsprites, &fi, 16, SPR_BOXWID, SPR_BOXHEI, true, pal, config.use_alpha_sprites());

    scr_read_palette(&fi, pal);
    snowballst = scr_loadsprites(&objectsprites, &fi, 1, SPR_AMMOWID, SPR_AMMOHEI, true, pal, config.use_alpha_sprites());

    scr_read_palette(&fi, pal);
    starst = scr_loadsprites(&objectsprites, &fi, 16, SPR_STARWID, SPR_STARHEI, true, pal, config.use_alpha_sprites());
    sts_init(starst + 9, NUM_STARS);

    scr_read_palette(&fi, pal);
    fishst = scr_loadsprites(&objectsprites, &fi, 32*2, SPR_FISHWID, SPR_FISHHEI, true, pal, config.use_alpha_sprites());

    scr_read_palette(&fi, pal);
    subst = scr_loadsprites(&objectsprites, &fi, 31, SPR_SUBMWID, SPR_SUBMHEI, true, pal, config.use_alpha_sprites());

    scr_read_palette(&fi, pal);
    torb = scr_loadsprites(&objectsprites, &fi, 1, SPR_TORPWID, SPR_TORPHEI, true, pal, config.use_alpha_sprites());
  }

  {
    auto fi = dataarchive->open(crossdat);

    Uint8 numcol = fi.getbyte();

    for (t = 0; t < numcol + 1; t++) {
      crosspal[2*t] = fi.getbyte();
      fi.getbyte();
      crosspal[2*t+1] = fi.getbyte();
    }

    crossdata = (Uint8*)malloc(120*SPR_CROSSWID*SPR_CROSSHEI*2);
    fi.read(crossdata, 120*SPR_CROSSWID*SPR_CROSSHEI*2);

    crossst = scr_gensprites(&objectsprites, 120, SPR_CROSSWID, SPR_CROSSHEI, true, config.use_alpha_sprites());
  }
}

Uint8 scr_numrobots(void) { return robotcount; }


void scr_settowercolor(Uint8 r, Uint8 g, Uint8 b) {

  Uint8 pal[3*256];

  int t;
  int bw, gw, rw;

  for (t = 0; t < 256; t++) {
    rw = (int)r*towerpal[2*t+1] + (255-(int)r)*towerpal[2*t];
    gw = (int)g*towerpal[2*t+1] + (255-(int)g)*towerpal[2*t];
    bw = (int)b*towerpal[2*t+1] + (255-(int)b)*towerpal[2*t];

    rw /= 256;
    gw /= 256;
    bw /= 256;

    pal[3*t] = rw;
    pal[3*t+1] = gw;
    pal[3*t+2] = bw;
  }

  for (t = 0; t < SPR_SLICESPRITES; t++)
    scr_regensprites(slicedata + t*SPR_SLICEWID*SPR_SLICEHEI, restsprites.data(slicestart + t), 1, SPR_SLICEWID, SPR_SLICEHEI, false, pal, false, true);

  for (t = 0; t < SPR_BATTLFRAMES; t++)
    scr_regensprites(battlementdata + t*SPR_BATTLWID*SPR_BATTLHEI, restsprites.data(battlementstart + t), 1, SPR_BATTLWID, SPR_BATTLHEI, false, pal, false, true);

  for (t = -36; t < 37; t++)
    for (int et = 0; et < 3; et++)
      if (doors[t+36].width != 0)
        scr_regensprites(doors[t+36].data[et], restsprites.data(doors[t+36].s[et]), 1, doors[t+36].width, SPR_SLICEHEI, false, pal, false, true);

  last_towercol_r = r;
  last_towercol_g = g;
  last_towercol_b = b;
}

void resettowercolor(void) {
  scr_settowercolor(last_towercol_r, last_towercol_g, last_towercol_b);
}

void scr_setcrosscolor(Uint8 rk, Uint8 gk, Uint8 bk) {

  Uint8 pal[256*3];

  int t, r, g, b;

  for (t = 0; t < 256; t++) {
    r = g = b = crosspal[2*t];

    r += ((int)crosspal[2*t+1] * rk) / 256;
    g += ((int)crosspal[2*t+1] * gk) / 256;
    b += ((int)crosspal[2*t+1] * bk) / 256;

    if (r > 255)
      r = 255;
    if (g > 255)
      g = 255;
    if (b > 255)
      b = 255;

    pal[3*t+0] = r;
    pal[3*t+1] = g;
    pal[3*t+2] = b;
  }

  for (t = 0; t < 120; t++) {
    scr_regensprites(crossdata + t*SPR_CROSSWID*SPR_CROSSHEI*2,
                     objectsprites.data(crossst+t),
                     1, SPR_CROSSWID, SPR_CROSSHEI, true, pal, config.use_alpha_sprites(), false);
  }
}

static void loadfont(void) {

  unsigned char pal[256*3];
  Uint16 c;
  int fontheight;

  auto fi = dataarchive->open(fontdat);

  scr_read_palette(&fi, pal);

  fontheight = fi.getbyte();

  while (!fi.eof()) {
    c = fi.getword();

    if (!c) break;

    fontchars[c].width = fi.getbyte();
    fontchars[c].s = scr_loadsprites(&fontsprites, &fi, 1, fontchars[c].width, fontheight, true, pal, config.use_alpha_font());
  }
}

static void loadscroller(void) {

  auto fi = dataarchive->open(scrollerdat);

  Uint8 layers;
  Uint8 towerpos;
  Uint8 pal[3*256];

  layers = fi.getbyte();

  num_scrolllayers = layers;

  assert_msg(num_scrolllayers > 1, "Must have at least 2 scroll layers!");

  scroll_layers = new _scroll_layer[layers];
  assert_msg(scroll_layers, "Failed to alloc memory for bonus scroller!");

  towerpos = fi.getbyte();

  sl_tower_depth = towerpos;

  sl_tower_num = fi.getword();
  sl_tower_den = fi.getword();

  for (int l = 0; l < layers; l++) {

    scroll_layers[l].xpos = fi.getword();
    scroll_layers[l].ypos = fi.getword();
    scroll_layers[l].width = fi.getword();
    scroll_layers[l].height = fi.getword();
    scroll_layers[l].num = fi.getword();
    scroll_layers[l].den = fi.getword();
    scroll_layers[l].xrepeat = fi.getword();

    scr_read_palette(&fi, pal);

    scroll_layers[l].image = scr_loadsprites(&layersprites, &fi, 1,
	scroll_layers[l].width, scroll_layers[l].height,
	l != 0, pal, config.use_alpha_layers());
  }
}

static void load_sprites(Uint8 what) {
  if ((what == 0xff) || (what & RL_OBJECTS))
    loadgraphics(what);

  if (what & RL_FONT)
    loadfont();

  if (what & RL_SCROLLER)
    loadscroller();
}

static void free_memory(Uint8 what) {
  int t;

  if (what == 0xff) {
    free(slicedata);
    free(battlementdata);
  }

  if (what & RL_OBJECTS) {
    free(crossdata);
    delete [] robots;
  }

  if (what & RL_SCROLLER)
    delete [] scroll_layers;

  if (what == 0xff)
    for (t = -36; t < 37; t++)
      for (int et = 0; et < 3; et++)
        if (doors[t+36].data[et])
          free(doors[t+36].data[et]);
}

void scr_reload_sprites(Uint8 what) {
  free_memory(what);
  load_sprites(what);
  resettowercolor();
}

void scr_init(void) {

  scr_reinit();

  load_sprites(0xff);

  /* initialize sinus table */
  for (int i = 0; i < TOWER_ANGLES; i++)
    sintab[i] = int(sin(i * 2 * M_PI / TOWER_ANGLES) * (TOWER_RADIUS + SPR_STEPWID / 2) + 0.5);


  /* initialize wave table */
  for (int t = 0; t < 0x80; t++)
    waves[t] = (Sint8)(8 * (sin(t * 2.0 * M_PI / 0x7f)) +
      4 * (sin(t * 3.0 * M_PI / 0x7f+2)) +
      3 * (sin(t * 5.0 * M_PI / 0x7f+3)) + 0.5);

}

static void doneWindow()
{
  if (display)
  {
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    SDL_FreeSurface(display);
  }
}

static void setupDisplay(int width, int height)
{
  // calculate the new width
  int w = width * SCREENHEI / height;

  // make w even
  if (w % 2) w++;

  if (w > 1280) w = 1280;
  if (w < 640) w = SCREENWID;

  SDL_RenderSetLogicalSize(sdlRenderer, w, SCREENHEI);

  sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, SCREENHEI);
  display = SDL_CreateRGBSurface(0, w, SCREENHEI, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

  currentScreenWidth = w;
  sts_init(starst + 9, NUM_STARS);
}

void scr_reinit() {

  doneWindow();

  SDL_CreateWindowAndRenderer(SCREENWID, SCREENHEI, config.fullscreen() ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0, &sdlWindow, &sdlRenderer);
  SDL_SetWindowMinimumSize(sdlWindow, 640, 480);
  SDL_SetWindowTitle(sdlWindow, "Nebulus");
  SDL_SetWindowResizable(sdlWindow, SDL_TRUE);

  int width; int height;
  SDL_GetWindowSize(sdlWindow, &width, &height);

  setupDisplay(width, height);
}

void scr_resize_event(int width, int height)
{
  if (display)
  {
    SDL_FreeSurface(display);
    SDL_DestroyTexture(sdlTexture);
  }

  setupDisplay(width, height);
}

void scr_done(void) {
  free_memory(0xff);
  sts_done();

  doneWindow();
}

static void cleardesk(long height) {
  SDL_Rect r;
  height *= 4;

/* clear left side from top to water */
  r.w = (SCREENWID - SPR_SLICEWID) / 2;
  if (height < (SCREENHEI / 2))
    r.h = (SCREENHEI / 2) + height;
  else
    r.h = SCREENHEI;
  r.x = r.y = 0;
  SDL_FillRect(display, &r, 0);

/* clear right side from top to water */
  r.x = (SCREENWID - SPR_SLICEWID) / 2 + SPR_SLICEWID;
  SDL_FillRect(display, &r, 0);

/* clear middle row from top to battlement */
  int upend = (SCREENHEI / 2) - (lev_towerrows() * SPR_SLICEHEI - height + SPR_BATTLHEI);
  if (upend > 0) {
    r.x = (SCREENWID - SPR_SLICEWID) / 2;
    r.w = SPR_SLICEWID;
    r.h = upend;
    SDL_FillRect(display, &r, 0);
  }
}

void scr_darkenscreen(void) {

  if (!config.use_alpha_darkening())
    return;

  scr_putbar(0, 0, SCREENWID, SCREENHEI, 0, 0, 0, 128);
}


/*
 angle: 0 means column 0 is in front
        8 means column 1 ...

 height: 0 means row 0 is in the middle
         4 means row 1 ...
*/
static void puttower(long angle, long height, long towerheight, int shift = 0) {

  /* calculate the blit position of the lowest slice considering the current
   * vertical position
   */
  int slice = 0;
  int ypos = SCREENHEI / 2 - SPR_SLICEHEI + height;

  /* now go up until we go over the top of the screen or reach the
   * top of the tower
   */
  while ((ypos > -SPR_SLICEHEI) && (slice < towerheight)) {

    /* if we are over the bottom of the screen, draw the slice */
    if (ypos < SCREENHEI)
      scr_blit(restsprites.data(slicestart + (angle % SPR_SLICEANGLES)), (SCREENWID / 2) - (SPR_SLICEWID / 2) + shift, ypos);

    slice++;
    angle = (angle + (SPR_SLICEANGLES / 2)) % TOWER_ANGLES;
    ypos -= SPR_SLICEHEI;
  }

  int upend = (SCREENHEI / 2) - (towerheight * SPR_SLICEHEI - height);

  if (upend > -10 && config.use_shadows())
  {
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+0, 2*TOWER_RADIUS, 1, 0, 0, 0, 225);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+1, 2*TOWER_RADIUS, 1, 0, 0, 0, 200);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+2, 2*TOWER_RADIUS, 1, 0, 0, 0, 175);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+3, 2*TOWER_RADIUS, 1, 0, 0, 0, 150);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+4, 2*TOWER_RADIUS, 1, 0, 0, 0, 125);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+5, 2*TOWER_RADIUS, 1, 0, 0, 0, 100);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+6, 2*TOWER_RADIUS, 1, 0, 0, 0,  75);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+7, 2*TOWER_RADIUS, 1, 0, 0, 0,  50);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+8, 2*TOWER_RADIUS, 1, 0, 0, 0,  25);
      scr_putbar(SCREENWID/2-TOWER_RADIUS, upend+9, 2*TOWER_RADIUS, 1, 0, 0, 0,   0);
  }
}

static void putbattlement(long angle, long height) {

  /* calculate the lower border position of the battlement */
  int upend = (SCREENHEI / 2) - (lev_towerrows() * SPR_SLICEHEI - height);

  /* if it's below the top of the screen, then blit the battlement */
  if (upend > 0)
    scr_blit(restsprites.data((angle % SPR_BATTLFRAMES) + battlementstart),
             (SCREENWID / 2) - (SPR_BATTLWID / 2), upend - SPR_BATTLHEI);
}

static void putwater(long height) {

  static const char simple_waves[] = {
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4,
    4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
    3, 3 };

  static int wavetime = 0;

  height *= 4;

  if (height < (SCREENHEI / 2)) {

    switch(config.waves_type()) {

    case configuration::waves_expensive:
      {
        int source_line = (SCREENHEI / 2) + height - 1;

        Uint8 buffer[4] = {0,0,0,0};

        for (int y = 0; y < (SCREENHEI / 2) - height; y++) {

          Uint8 * target = (Uint8*)display->pixels + ((SCREENHEI/2) + height + y) * display->pitch;

          for (int x = 0; x < SCREENWID; x++) {
            Sint16 dx = waves[(x+y+12*wavetime) & 0x7f] + waves[(2*x-y+11*wavetime) & 0x7f];
            Sint16 dy = waves[(x-y+13*wavetime) & 0x7f] + waves[(2*x-3*y-14*wavetime) & 0x7f];

            dx = dx * y / (SCREENHEI/2);
            dy = dy * y / (SCREENHEI/2);

            if ((x+dx < 0) || (x+dx > SCREENWID) || (source_line+dy < 0))
              memcpy(target, &buffer, display->format->BytesPerPixel);
            else
              memcpy(target, (Uint8*)display->pixels +
                     (x+dx) * display->format->BytesPerPixel +
                     (source_line+dy) * display->pitch, display->format->BytesPerPixel);

            target += display->format->BytesPerPixel;
          }
          scr_putbar(0, (SCREENHEI/2) + height + y, SCREENWID, 1, 0, 0, y, 128);
          source_line --;
        }
      }
      break;
    case configuration::waves_simple:
      {
        int horizontal_shift;

        scr_putbar(0, (SCREENHEI / 2) + height, 10,  (SCREENHEI / 2) - height, 0, 0, 0, 255);
        scr_putbar(SCREENWID-10, (SCREENHEI / 2) + height, 10,  (SCREENHEI / 2) - height, 0, 0, 0, 255);

        for (int y = 0; y < (SCREENHEI / 2) - height; y++) {

          int target_line = (SCREENHEI / 2) + height + y;
          int source_line = (SCREENHEI / 2) + height - y - 1 - simple_waves[(wavetime * 4 + y * 2) & 0x7f];
          if (source_line < 0)
            source_line = 0;

          int z = simple_waves[(wavetime*5 + y) & 0x7f];
          if (abs(z - 4) > y) {
            if (z < 4)
              horizontal_shift = 4 - y;
            else
              horizontal_shift = 4 + y;
          } else {
            horizontal_shift = z;
          }

          SDL_Rect r1;
          SDL_Rect r2;

          r1.w = r2.w = SCREENWID;
          r1.h = r2.h = 1;

          r2.y = target_line;
          r1.y = source_line;

          if (horizontal_shift > 0) {
            r1.x = horizontal_shift;
            r2.x = 0;
          } else {
            r1.x = 0;
            r2.x = -horizontal_shift;
          }

          SDL_BlitSurface(display, &r1, display, &r2);
          scr_putbar(0, target_line, SCREENWID, 1, 0, 0, y, 128);

        }
      }
      break;
    case configuration::waves_nonreflecting:
      for (int y = 0; y < (SCREENHEI / 2) - height; y++)
        scr_putbar(0, SCREENHEI/2 + height + y, SCREENWID, 1, 0, 0, 30 + y/2, 255);
      break;
    }
  }

  wavetime++;
}

int scr_textlength(const std::string & s, int chars) {
  int len = 0;
  int pos = 0;
  mbstate_t state;
  memset (&state, '\0', sizeof (state));
  wchar_t tmp;

  while (s[pos] && (chars > 0)) {

    size_t nbytes = mbrtowc (&tmp, s.c_str()+pos, chars, &state);

    if (nbytes <= 0 || nbytes == static_cast<std::size_t>(-2) || nbytes == static_cast<std::size_t>(-1))
      return 0;

    if (tmp == ' ') {
      len += FONTMINWID;
    } else {
      if (fontchars[tmp & 0xffff].width != 0)
        len += fontchars[tmp & 0xffff].width + 3;
    }

    pos += nbytes;
    chars -= nbytes;
  }

  return len;
}

void scr_writetext_center(long y, const std::string & s) {
  scr_writetext ((SCREENWID - scr_textlength(s)) / 2, y, s);
}

void scr_writetext_broken_center(long y, const std::string & s) {

  // ok, we try to break the text into several lines, if the lines are longer then the
  // screenwidth

  int len = s.size();
  int start = 0;
  int end = len;

  while (start < len) {

    while (scr_textlength(s.substr(start, end-start+1)) > SCREENWID) {
      end--;
      while ((end > start) && (s[end] != ' ')) end--;

      if (end == start) {
        while ((end < len) && (s[end] != ' ')) end++;
        break;
      }
    }

    if (s[end] == ' ') end--;

    scr_writetext((SCREENWID - scr_textlength(s.substr(start, end-start+1))) / 2, y, s.substr(start), end-start+1);

    start = end+1;
    end = len;
    while ((start < len) && (s[start] == ' ')) start++;

    y += 40;

  }
}

void scr_putbar(int x, int y, int br, int h, Uint8 colr, Uint8 colg, Uint8 colb, Uint8 alpha) {

  if (alpha != 255) {

    SDL_Surface *s = SDL_CreateRGBSurface(0, br, h, 32,
                                          display->format->Rmask, display->format->Gmask,
                                          display->format->Bmask, display->format->Amask);

    SDL_Rect r;
    r.w = br;
    r.h = h;
    r.x = 0;
    r.y = 0;

    SDL_FillRect(s, &r, SDL_MapRGBA(s->format, colr, colg, colb, alpha));

    scr_blit(s, x, y);

    SDL_FreeSurface(s);

  } else {

    SDL_Rect r;
    r.w = br;
    r.h = h;
    r.x = x;
    r.y = y;
    SDL_FillRect(display, &r, SDL_MapRGBA(display->format, colr, colg, colb, 255));
  }
}

void
scr_putrect(int x, int y, int br, int h, Uint8 colr, Uint8 colg, Uint8 colb, Uint8 alpha)
{
  scr_putbar(x, y,      1     , h, colr, colg, colb, alpha);
  scr_putbar(x, y,      br    , 1, colr, colg, colb, alpha);
  scr_putbar(x + br, y, 1     , h, colr, colg, colb, alpha);
  scr_putbar(x, y + h , br + 1, 1, colr, colg, colb, alpha);
}

/* exchange active and inactive page */
void scr_swap(void) {
  if (!tt_has_focus)
  {
      scr_darkenscreen();
      SDL_UpdateTexture(sdlTexture, NULL, display->pixels, display->pitch);
      SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
      SDL_RenderPresent(sdlRenderer);
      wait_for_focus();
  }

  SDL_UpdateTexture(sdlTexture, NULL, display->pixels, display->pitch);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
  SDL_RenderPresent(sdlRenderer);
}

void scr_setclipping(int x, int y, int w, int h) {
  if (x < 0) SDL_SetClipRect(display, NULL);
  else {
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    SDL_SetClipRect(display, &r);
  }
}

void scr_blit(SDL_Surface * s, int x, int y) {
  SDL_Rect r;
  r.w = s->w;
  r.h = s->h;
  r.x = x;
  r.y = y;
  SDL_BlitSurface(s, NULL, display, &r);
}

void scr_blit_center(SDL_Surface * s) {
  SDL_Rect r;
  r.w = s->w;
  r.h = s->h;
  r.x = (display->w - s->w)/2;
  r.y = (display->h - s->h)/2;
  SDL_BlitSurface(s, NULL, display, &r);
}


/* draws the tower and the doors */
static void draw_tower(long vert, long angle) {

  puttower(angle, vert, lev_towerrows());

  int slice = 0;
  int ypos = SCREENHEI / 2 - SPR_SLICEHEI + vert;

  while (ypos > SCREENHEI) {
    slice++;
    ypos -= SPR_SLICEHEI;
  }

  while ((ypos > -SPR_SLICEHEI) && (slice < lev_towerrows())) {

    for (int col = 0; col < 16; col++) {

      int a = (col * 8 + angle + 36) % TOWER_ANGLES;

      if ((a > 72) || !doors[a].width)
        continue;

      if (lev_is_door(slice, col)) {
        if (lev_is_door_upperend(slice, col))
          scr_blit(restsprites.data(doors[a].s[2]), (SCREENWID / 2) + doors[a].xstart, ypos);
        else if (lev_is_door_upperend(slice - 1, col))
          scr_blit(restsprites.data(doors[a].s[1]), (SCREENWID / 2) + doors[a].xstart, ypos);
        else
          scr_blit(restsprites.data(doors[a].s[0]), (SCREENWID / 2) + doors[a].xstart, ypos);
      }
    }

    slice++;
    ypos -= SPR_SLICEHEI;
  }
}

static void draw_tower_editor(long vert, long angle, int state) {

  puttower(angle, vert, lev_towerrows());


  int slice = 0;
  int ypos = SCREENHEI / 2 - SPR_SLICEHEI + vert;

  while (ypos > SCREENHEI) {
    slice++;
    ypos -= SPR_SLICEHEI;
  }

  while ((ypos > -SPR_SLICEHEI) && (slice < lev_towerrows())) {

    for (int col = 0; col < 16; col++) {

      int a = (col * 8 + angle + 36) % TOWER_ANGLES;

      if ((a > 72) || !doors[a].width)
        continue;

      if (lev_is_door(slice, col)) {

        if (lev_is_targetdoor(slice, col) && (state & 1)) continue;

        if (lev_is_door_upperend(slice, col))
          scr_blit(restsprites.data(doors[a].s[2]), (SCREENWID / 2) + doors[a].xstart, ypos);
        else if (lev_is_door_upperend(slice - 1, col))
          scr_blit(restsprites.data(doors[a].s[1]), (SCREENWID / 2) + doors[a].xstart, ypos);
        else
          scr_blit(restsprites.data(doors[a].s[0]), (SCREENWID / 2) + doors[a].xstart, ypos);
      }
    }

    slice++;
    ypos -= SPR_SLICEHEI;
  }
}

static void putcircleshadow(int a, int h, int rad)
{
  int sw = 5;

  // first check, if we already calculated the required shadow
  for (const auto & [ca, crad, cs] : circleShadowCache)
      if (ca == a && crad == rad)
      {
          SDL_Rect r;
          r.w = cs->w;
          r.h = cs->h;
          r.x = SCREENWID/2-TOWER_RADIUS;
          r.y = h-rad-sw;

          SDL_BlitSurface(cs, nullptr, display, &r);

          return;
      }

  // well no entry in the cache create one
  auto sh = SDL_CreateRGBSurface(0, 2*TOWER_RADIUS, 2*rad+2*sw,32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  auto entry = std::make_tuple(a, rad, sh);

  // fill with transparenc
  SDL_FillRect(sh, nullptr, SDL_MapRGBA(sh->format, 0, 0, 0, 0));

  // draw the shadow into the surface
  for (int y = -rad-sw; y < rad+sw; y++)
      for (int x = -TOWER_RADIUS; x < TOWER_RADIUS; x++)
      {
          // find out position on the tower that we want to check
          float z = -sqrt(TOWER_RADIUS*TOWER_RADIUS-x*x);

          // calculate the position of the object
          float ox =  sin(a*2*M_PI/ TOWER_ANGLES) * (TOWER_RADIUS + SPR_STEPWID/2);
          float oz = -cos(a*2*M_PI/ TOWER_ANGLES) * (TOWER_RADIUS + SPR_STEPWID/2);

          // to simplyfy calculations
          // slightly rotate everything so that the light is
          // comming directly from the front
          float light_ang = (2 * M_PI / 16 /*steinzahl*/);

          float rox = cos(light_ang)*ox - sin(light_ang)*oz;
          float roz = sin(light_ang)*ox + cos(light_ang)*oz;
          float rx = cos(light_ang)*x - sin(light_ang)*z;

          // now we are in shadow, iff the x position is less than wi from
          // the position of the object
          int diff = (rox-rx)*(rox-rx)+y*y;

          if (roz < 0 && diff < (rad+sw)*(rad+sw))
          {
              float s = 0;

              if (diff < rad*rad)
                  s = 0;
              else
                  s = 1.0*(diff - rad*rad) / ((rad+sw)*(rad+sw)-rad*rad);

              // finally dim the shadows when going to the right because
              // we are supposed to be in shadow already
              if (x > TOWER_RADIUS-32)
                  s = 1.0 - ((1.0-s) * (0.5 + (TOWER_RADIUS-x)/64.0));

              Uint32 * target = (Uint32*)((Uint8*)sh->pixels + ((y+rad+sw)*sh->pitch) + ((x+TOWER_RADIUS) * sh->format->BytesPerPixel));
              *target = SDL_MapRGBA(sh->format, 0, 0, 0, 255-(100+155*s));
          }
      }

  SDL_Rect r;
  r.w = sh->w;
  r.h = sh->h;
  r.x = SCREENWID/2-TOWER_RADIUS;
  r.y = h-rad-sw;

  SDL_BlitSurface(sh, nullptr, display, &r);

  circleShadowCache.push_back(entry);
}


/* draws something of the environment */
static void putcaseshadow(unsigned char w, long a, long h, bool drawlevelrobotshadows) {

  int wi = 0;
  int type = 0;  // 0: rectangle, 1: sphere

  switch (w) {

  case TB_ELEV_BOTTOM:
  case TB_ELEV_TOP:
  case TB_ELEV_MIDDLE:
    wi = SPR_ELEVAWID;
    break;

  case TB_STEP:
  case TB_STEP_VANISHER:
  case TB_STEP_LSLIDER:
  case TB_STEP_RSLIDER:
    wi = SPR_STEPWID;
    break;

  case TB_STICK:
  case TB_STICK_BOTTOM:
  case TB_STICK_MIDDLE:
  case TB_STICK_DOOR:
  case TB_STICK_DOOR_TARGET:
  case TB_BOX:
    wi = SPR_STICKWID;
    break;

  case TB_ROBOT1:
  case TB_ROBOT2:
  case TB_ROBOT3:
    type = 1;
    wi = 15;
    break;
  case TB_ROBOT4:
  case TB_ROBOT5:
  case TB_ROBOT6:
  case TB_ROBOT7:
    type = 1;
    wi = 10;
    break;

  default:
    return;
  }

  if (type == 0)
  {
      int sw = 5;


      // first check, if we already calculated the required shadow
      for (const auto & [ca, cwi, cs] : rectangularShadowCache)
          if (ca == a && cwi == wi)
          {
              SDL_Rect r;
              r.w = cs->w;
              r.h = cs->h;
              r.x = SCREENWID/2-TOWER_RADIUS;
              r.y = h-sw;

              SDL_BlitSurface(cs, nullptr, display, &r);

              return;
          }

      // well no entry in the cache create one
      auto sh = SDL_CreateRGBSurface(0, 2*TOWER_RADIUS, SPR_SLICEHEI+2*sw,32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
      auto entry = std::make_tuple(a, wi, sh);

      // fill with transparenc
      SDL_FillRect(sh, nullptr, SDL_MapRGBA(sh->format, 0, 0, 0, 0));

      for (int y = -sw; y < SPR_SLICEHEI+sw; y++)
          for (int x = -TOWER_RADIUS; x < TOWER_RADIUS; x++)
          {
              // find out position on the tower that we want to check
              float z = -sqrt(TOWER_RADIUS*TOWER_RADIUS-x*x);

              // calculate the position of the object
              float ox =  sin(a*2*M_PI/ TOWER_ANGLES) * (TOWER_RADIUS + SPR_STEPWID/2);
              float oz = -cos(a*2*M_PI/ TOWER_ANGLES) * (TOWER_RADIUS + SPR_STEPWID/2);

              // to simplyfy calculations
              // slightly rotate everything so that the light is
              // comming directly from the front
              float light_ang = (2 * M_PI / 16 /*steinzahl*/);

              float rox = cos(light_ang)*ox - sin(light_ang)*oz;
              float roz = sin(light_ang)*ox + cos(light_ang)*oz;
              float rx = cos(light_ang)*x - sin(light_ang)*z;

              // now we are in shadow, iff the x position is less than wi from
              // the position of the object
              auto diff = abs(rox-rx);

              if ((roz < 0) && (diff < wi/2 + sw))
              {
                  float s = 0;

                  if (diff > wi/2-sw)
                      s = (1.0*diff-(wi/2-sw)) / (2*sw);

                  if (y < 0)
                      s = 1.0-((1.0-s)*(1.0-(-1.0*y/sw)));
                  if (y >= SPR_SLICEHEI)
                      s = 1.0-((1.0-s)*(1.0-(1.0*(y-(SPR_SLICEHEI-1))/sw)));

                  // finally dim the shadows when going to the right because
                  // we are supposed to be in shadow already
                  if (x > TOWER_RADIUS-32)
                      s = 1.0 - ((1.0-s) * (0.5 + (TOWER_RADIUS-x)/64.0));

                  Uint32 * target = (Uint32*)((Uint8*)sh->pixels + ((y+sw)*sh->pitch) + ((x+TOWER_RADIUS) * sh->format->BytesPerPixel));
                  *target = SDL_MapRGBA(sh->format, 0, 0, 0, 255-(100+155*s));
              }
          }

      SDL_Rect r;
      r.w = sh->w;
      r.h = sh->h;
      r.x = SCREENWID/2-TOWER_RADIUS;
      r.y = h-sw;

      SDL_BlitSurface(sh, nullptr, display, &r);

      rectangularShadowCache.push_back(entry);
  }
  else
  {
      if (drawlevelrobotshadows)
          putcircleshadow(a, h, wi);
  }
}


/* draws something of the environment */
static void putcase(unsigned char w, long x, long h, int depth) {
  long angle = 0;
  switch (w) {

  case TB_EMPTY:
    /* blank case */
    break;

  case TB_ELEV_BOTTOM:
  case TB_ELEV_TOP:
  case TB_ELEV_MIDDLE:
    scr_blit(restsprites.data((angle % SPR_ELEVAFRAMES) + elevatorsprite), x - (SPR_ELEVAWID / 2), h);
    if (config.use_shadows()) scr_putbar(x-(SPR_ELEVAWID/2), h, SPR_ELEVAWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;

  case TB_STEP:
  case TB_STEP_VANISHER:
  case TB_STEP_LSLIDER:
  case TB_STEP_RSLIDER:
    scr_blit(restsprites.data((angle % SPR_STEPFRAMES) + step), x - (SPR_STEPWID / 2), h);
    if (config.use_shadows()) scr_putbar(x-(SPR_STEPWID/2), h, SPR_STEPWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);

    break;

  case TB_STICK:
  case TB_STICK_BOTTOM:
  case TB_STICK_MIDDLE:
  case TB_STICK_DOOR:
  case TB_STICK_DOOR_TARGET:
    scr_blit(restsprites.data(stick), x - (SPR_STICKWID / 2), h);
    if (config.use_shadows()) scr_putbar(x-(SPR_STICKWID/2), h, SPR_STICKWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);

    break;

  case TB_BOX:
    scr_blit(objectsprites.data(boxst + boxstate), x - (SPR_BOXWID / 2), h);
    // no shading of the flashing box....

    break;
  }
}

static void drawdarkenedRobot(int nr, int x, int y, int depth)
{
    auto obj = objectsprites.data(nr);

    // create copy of obj
    // overdraw that one with a different surface that is black
    // and translucent to darken it and finally blit
    // that object
    auto obj2 = SDL_CreateRGBSurface(0, obj->w, obj->h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Surface *s = SDL_CreateRGBSurface(0, obj->w, obj->h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    SDL_SetSurfaceBlendMode(obj, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(obj, nullptr, obj2, nullptr);
    SDL_SetSurfaceBlendMode(obj, SDL_BLENDMODE_BLEND);

    SDL_Rect r;

    r.w = obj2->w;
    r.h = obj2->h;
    r.x = 0;
    r.y = 0;

    SDL_FillRect(s, &r, SDL_MapRGBA(s->format, 255-depth, 255-depth, 255-depth, depth));

    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_MOD);

    SDL_BlitSurface(s, nullptr, obj2, nullptr);

    r.x = x;
    r.y = y;

    SDL_BlitSurface(obj2, nullptr, display, &r);

    SDL_FreeSurface(s);
    SDL_FreeSurface(obj2);
}

static void putcase_editor(unsigned char w, long x, long h, int state, int depth) {
  long angle = 0;
  switch (w) {

  case TB_EMPTY:
    /* blank case */
    break;

  case TB_ELEV_BOTTOM:
    scr_blit(restsprites.data((angle % SPR_ELEVAFRAMES) + elevatorsprite), x - (SPR_ELEVAWID / 2), h - (state % 4));
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_ELEVAWID/2), h, SPR_ELEVAWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;
  case TB_STATION_MIDDLE:
    scr_blit(restsprites.data((angle % SPR_ELEVAFRAMES) + elevatorsprite), x - (SPR_ELEVAWID / 2), h - SPR_SLICEHEI/2 + abs(state % 16 - 8));
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_ELEVAWID/2), h, SPR_ELEVAWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;
  case TB_STATION_TOP:
    scr_blit(restsprites.data((angle % SPR_ELEVAFRAMES) + elevatorsprite), x - (SPR_ELEVAWID / 2), h + (state % 4));
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_ELEVAWID/2), h, SPR_ELEVAWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;
  case TB_STEP:
    scr_blit(restsprites.data(((angle % SPR_STEPFRAMES) + step)), x - (SPR_STEPWID / 2), h);
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_STEPWID/2), h, SPR_STEPWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;
  case TB_STEP_VANISHER:
    if (state & 1)
    {
      scr_blit(restsprites.data(((angle % SPR_STEPFRAMES) + step)), x - (SPR_STEPWID / 2), h);
      if (config.use_alpha_sprites()) scr_putbar(x-(SPR_STEPWID/2), h, SPR_STEPWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    }
    break;
  case TB_STEP_LSLIDER:
    scr_blit(restsprites.data(((angle % SPR_STEPFRAMES) + step)), x - (SPR_STEPWID / 2) + state % 4, h);
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_STEPWID/2), h, SPR_STEPWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;

  case TB_STEP_RSLIDER:
    scr_blit(restsprites.data(((angle % SPR_STEPFRAMES) + step)), x - (SPR_STEPWID / 2) - state % 4, h);
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_STEPWID/2), h, SPR_STEPWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;

  case TB_STICK:
    scr_blit(restsprites.data(stick), x - (SPR_STICKWID / 2), h);
    if (config.use_alpha_sprites()) scr_putbar(x-(SPR_STICKWID/2), h, SPR_STICKWID, TOWER_SLICE_HEIGHT, 0, 0, 0, depth);
    break;

  case TB_BOX:
    scr_blit(objectsprites.data(boxst + boxstate % 16), x - (SPR_BOXWID / 2), h);
    break;

  case TB_ROBOT1:
    drawdarkenedRobot(ballst + 1, x - (SPR_ROBOTWID / 2), h - SPR_ROBOTHEI/2, depth);
    break;
  case TB_ROBOT2:
    drawdarkenedRobot(ballst, x - (SPR_ROBOTWID / 2) + state % 16 / 2, h - SPR_ROBOTHEI/2, depth);
    break;
  case TB_ROBOT3:
    drawdarkenedRobot(ballst, x - (SPR_ROBOTWID / 2), h - SPR_ROBOTHEI/2, depth);
    break;
  case TB_ROBOT4:
    drawdarkenedRobot(robots[lev_robotnr()].start + state % robots[lev_robotnr()].count, x - (SPR_ROBOTWID / 2), h - SPR_ROBOTHEI/2 + abs(state % 16 - 8), depth);
    break;
  case TB_ROBOT5:
    drawdarkenedRobot(robots[lev_robotnr()].start + state % robots[lev_robotnr()].count, x - (SPR_ROBOTWID / 2), h - SPR_ROBOTHEI + abs(state % 16 - 8) * 2, depth);
    break;
  case TB_ROBOT6:
    drawdarkenedRobot(robots[lev_robotnr()].start + state % robots[lev_robotnr()].count, x - (SPR_ROBOTWID / 2) + abs(state % 16 - 8), h - SPR_SLICEHEI/2, depth);
    break;
  case TB_ROBOT7:
    drawdarkenedRobot(robots[lev_robotnr()].start + state % robots[lev_robotnr()].count, x - (SPR_ROBOTWID / 2) + 2 * abs(state % 16 - 8), h - SPR_SLICEHEI/2, depth);
    break;
  }
}


/* draws a robot */
static void putrobot(int t, int m, long x, long h, int depth)
{
  int nr;

  if (h > (SCREENHEI + SPR_ROBOTHEI)) return;

  switch (t) {

    case OBJ_KIND_JUMPBALL:
      nr = ballst;
      break;

    case OBJ_KIND_FREEZEBALL:
    case OBJ_KIND_FREEZEBALL_FROZEN:
    case OBJ_KIND_FREEZEBALL_FALLING:
      nr = ballst + 1;
      break;

    case OBJ_KIND_DISAPPEAR:
      nr = starst + m * 2;
      break;

    case OBJ_KIND_APPEAR:
      nr = starst - m * 2 + 16;
      break;

    case OBJ_KIND_ROBOT_VERT:
    case OBJ_KIND_ROBOT_HORIZ:
      nr = robots[lev_robotnr()].start + (m % robots[lev_robotnr()].count);
      break;

    default:
      nr = 40;
      break;
  }

  drawdarkenedRobot(nr, x + (SCREENWID / 2) - (SPR_ROBOTWID / 2), h - SPR_ROBOTHEI, depth);
}

void scr_writetext(long x, long y, const std::string & s, int maxchars) {

  mbstate_t state;
  memset (&state, '\0', sizeof (state));
  wchar_t tmp;

  int t = 0;

  if (maxchars == -1)
    maxchars = s.size();

  while (s[t] && (maxchars > 0)) {

    size_t nbytes = mbrtowc (&tmp, s.c_str()+t, maxchars, &state);

    if (nbytes >= (size_t) -2) {
      return;
    }

    if (tmp == ' ') {
      x += FONTMINWID;
      t += nbytes;
      maxchars -= nbytes;
      continue;
    }

    if (fontchars[tmp & 0xffff].width != 0) {
      scr_blit(fontsprites.data(fontchars[tmp & 0xffff].s), x, y-20);
      x += fontchars[tmp & 0xffff].width + 3;
    }

    t += nbytes;
    maxchars -= nbytes;
  }
}

void scr_writeformattext(long x, long y, const std::string & s) {

  mbstate_t state;
  memset (&state, '\0', sizeof (state));
  wchar_t tmp;

  int len = s.size();

  int origx = x;
  int t = 0;
  Uint8 towerblock = 0;
  while (t < len) {

    size_t nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);

    if (nbytes >= (size_t) -2) {
      return;
    }

    t += nbytes;

    switch(tmp) {
    case ' ':
      x += FONTMINWID;
      break;
    case '~':

      nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
      if (nbytes >= (size_t) -2) {
        return;
      }
      t += nbytes;

      switch(tmp) {
      case 't':

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x = (tmp - '0') * 100;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x += (tmp - '0') * 10;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x += (tmp - '0');

        break;
      case 'T':
        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x = origx + (tmp - '0') * 100;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x += (tmp - '0') * 10;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        x += (tmp - '0');

        break;
      case 'b':

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        towerblock = conv_char2towercode(tmp);
        putcase(towerblock, x+16, y, 255);
        x += 32;
        break;
      case 'e':
        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) {
          return;
        }
        t += nbytes;

        towerblock = conv_char2towercode(tmp);
        putcase_editor(towerblock, x+16, y, boxstate, 255);
        x += 32;
        break;
      default:
        assert_msg(0, "Wrong command in formatted text.");
      }
      break;
    default:
      if (fontchars[tmp & 0xffff].width != 0) {
        scr_blit(fontsprites.data(fontchars[tmp & 0xffff].s), x, y-20);
        x += fontchars[tmp & 0xffff].width + 3;
      }
    }
  }
}

long scr_formattextlength(long x, const std::string & s) {
  mbstate_t state;
  memset (&state, '\0', sizeof (state));
  wchar_t tmp;

  int len = s.size();

  int origx = x;
  int t = 0;
  while (t < len) {

    size_t nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
    if (nbytes >= (size_t) -2) return 0;
    t += nbytes;

    switch(tmp) {
    case ' ':
      x += FONTMINWID;
      break;
    case '~':

      nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
      if (nbytes >= (size_t) -2) return 0;
      t += nbytes;

      switch(tmp) {
      case 't':

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x = (tmp - '0') * 100;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += (tmp - '0') * 10;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += (tmp - '0');

        break;
      case 'T':
        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x = origx + (tmp - '0') * 100;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += (tmp - '0') * 10;

        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += (tmp - '0');

        break;
      case 'b':
        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += 32;
        break;
      case 'e':
        nbytes = mbrtowc (&tmp, s.c_str()+t, len-t, &state);
        if (nbytes >= (size_t) -2) return 0;
        t += nbytes;
        x += 32;
        break;
      default:
        assert_msg(0, "Wrong command in formatted text.");
      }
      break;
    default:
      if (fontchars[tmp & 0xffff].width != 0) {
        x += fontchars[tmp & 0xffff].width + 3;
      }
    }
  }

  return (x-origx);
}

/* draws something of the tower */

/* vert is the vertical position of the tower
 * a is the angle on the tower to be drawn: 0 front, 32 right, ...
 * angle is the angle of the tower: 0 column 0 in front, 8, column 1, ...
 * hs, he are start and ending rows to be drawn
 */
static void putthings(long vert, long a, long angle) {

  /* ok, at first lets check if there is a column right at the
   angle to be drawn */
  if (((a - angle) & 0x7) == 0) {

    /* yes there is one, find out wich one */
    int col = ((a - angle) / TOWER_STEPS_PER_COLUMN) & (TOWER_COLUMNS - 1);

    /* calc the x pos where the thing has to be drawn */
    int x = sintab[a % TOWER_ANGLES] + (SCREENWID/2);

    int depth = clamp(400-2*(sintab[(a+TOWER_ANGLES/4) % TOWER_ANGLES]+TOWER_RADIUS+SPR_STEPWID/2+1), 0, 205);

    int slice = 0;
    int ypos = SCREENHEI / 2 - SPR_SLICEHEI + vert;

    while ((ypos > -SPR_SLICEHEI) && (slice < lev_towerrows())) {

      /* if we are over the bottom of the screen, draw the slice */
      if (ypos < SCREENHEI)
        putcase(lev_tower(slice, col), x, ypos, depth);

      slice++;
      ypos -= SPR_SLICEHEI;
    }
  }

  /* and now check for robots to be drawn */
  for (int rob = 0; rob < 4; rob++) {

    /* if the the current robot is active and not the cross */
    if (rob_kind(rob) != OBJ_KIND_NOTHING && rob_kind(rob) != OBJ_KIND_CROSS) {

      /* ok calc the angle the robots needs to be drawn at */
      int rob_a = (rob_angle(rob) - 4 + angle) & (TOWER_ANGLES - 1);

      int depth = clamp(400-2*(sintab[(rob_a+TOWER_ANGLES/4) % TOWER_ANGLES]+TOWER_RADIUS+SPR_STEPWID/2+1), 0, 205);

      /* check if the robot is "inside" the current column */
      if (rob_a == a)
        putrobot(rob_kind(rob), rob_time(rob),
                 sintab[rob_a], SCREENHEI / 2 + vert - rob_vertical(rob) * 4, depth);
    }
  }
}

static void putshadow(long vert, long a, long angle, bool drawlevelrobotshadows) {

  /* ok, at first lets check if there is a column right at the
   angle to be drawn */
  if (((a - angle) & 0x7) == 0) {

    /* yes there is one, find out wich one */
    int col = ((a - angle) / TOWER_STEPS_PER_COLUMN) & (TOWER_COLUMNS - 1);

    int slice = 0;
    int ypos = SCREENHEI / 2 - SPR_SLICEHEI + vert;

    while ((ypos > -SPR_SLICEHEI) && (slice < lev_towerrows())) {

      /* if we are over the bottom of the screen, draw the slice */
      if (ypos < SCREENHEI)
      {
        putcaseshadow(lev_tower(slice, col), a % TOWER_ANGLES, ypos, drawlevelrobotshadows);
      }

      slice++;
      ypos -= SPR_SLICEHEI;
    }
  }




  /* and now check for robots to be drawn */
  // this only draws robots that are active in the game
  // the robots of the level editor are done with putcaseshadow above
  for (int rob = 0; rob < 4; rob++) {

    /* if the the current robot is active and not the cross */
    if (rob_kind(rob) != OBJ_KIND_NOTHING && rob_kind(rob) != OBJ_KIND_CROSS && rob_kind(rob) != OBJ_KIND_APPEAR && rob_kind(rob) != OBJ_KIND_DISAPPEAR) {

      /* ok calc the angle the robots needs to be drawn at */
      int rob_a = (rob_angle(rob) - 4 + angle) & (TOWER_ANGLES - 1);

      /* check if the robot is "inside" the current column */
      if (rob_a == a)
      {
          int wi = 15;

          if (rob_kind(rob) == OBJ_KIND_ROBOT_VERT || rob_kind(rob) == OBJ_KIND_ROBOT_HORIZ)
              wi = 10;

          putcircleshadow(rob_a,SCREENHEI / 2 + vert - rob_vertical(rob) * 4 - SPR_ROBOTHEI/2, wi);
      }
    }
  }
}

static void putthings_editor(long vert, long a, long angle, int state) {

  /* ok, at first lets check if there is a column right at the
   angle to be drawn */
  if (((a - angle) & 0x7) == 0) {

    /* yes there is one, find out wich one */
    int col = ((a - angle) / TOWER_STEPS_PER_COLUMN) & (TOWER_COLUMNS - 1);

    /* calc the x pos where the thing has to be drawn */
    int x = sintab[a % TOWER_ANGLES] + (SCREENWID/2);

    int depth = clamp(400-2*(sintab[(a+TOWER_ANGLES/4) % TOWER_ANGLES]+TOWER_RADIUS+SPR_STEPWID/2+1), 0, 205);

    int slice = 0;
    int ypos = SCREENHEI / 2 - SPR_SLICEHEI + vert;

    while ((ypos > -SPR_SLICEHEI) && (slice < lev_towerrows())) {

      /* if we are over the bottom of the screen, draw the slice */
      if (ypos < SCREENHEI)
        putcase_editor(lev_tower(slice, col), x, ypos, state, depth);

      slice++;
      ypos -= SPR_SLICEHEI;
    }
  }
}

/* draws everything behind the tower */
static void draw_behind(long vert, long angle)
{
  for (int a = 0; a < 16; a ++) {
/* angle 48 to 31 */
    putthings(vert, 48 - a, angle);
/* amgle 80 to 95 */
    putthings(vert, 80 + a, angle);
  }
}


static void draw_behind_editor(long vert, long angle, int state)
{
  for (int a = 0; a < 16; a ++) {
    putthings_editor(vert, 48 - a, angle, state);
    putthings_editor(vert, 80 + a, angle, state);
  }
}

/* draws everything in front of the tower */
static void draw_before(long  vert, long angle)
{
  for (int a = 0; a < 32; a ++) {
    putthings(vert, 32 - a, angle);
    putthings(vert, 96 + a, angle);
  }
  putthings(vert, 0, angle);
}

static void draw_shadows(long vert, long angle, bool drawlevelrobotshadows)
{
  for (int a = 0; a < 32; a ++) {
    putshadow(vert, 32 - a, angle, drawlevelrobotshadows);
    putshadow(vert, 96 + a, angle, drawlevelrobotshadows);
  }
  putshadow(vert, 0, angle, drawlevelrobotshadows);

}


static void draw_before_editor(long  vert, long angle, int state)
{
  for (int a = 0; a < 32; a ++) {
    putthings_editor(vert, 32 - a, angle, state);
    putthings_editor(vert, 96 + a, angle, state);
  }
  putthings_editor(vert, 0, angle, state);
}

/* draws the cross that flies over the screen */
static void putcross(long vert)
{
  long i, y;

  for (int t = 0; t < 4; t++) {
    if (rob_kind(t) == OBJ_KIND_CROSS) {
      // we need to figure out the scaling factor such that the
      // factor is 5 when rob_angle is close to 60 and (SCREENWID/2) / 60 when rob_angle is close to 0 or 120
      int64_t j = rob_angle(t) - 60;
      if (j < 0) j = -j;
      j *= (SCREENWID - 600);
      j /= 6*12;
      j += 500;
      i = ((rob_angle(t) - 60) * j) / 100;
      y = (vert - rob_vertical(t)) * 4 + (SCREENHEI / 2) - SPR_CROSSHEI;
      if (y > -SPR_CROSSHEI && y < SCREENHEI)
        scr_blit(objectsprites.data(crossst + labs(rob_time(t)) % 120), i + (SCREENWID - SPR_CROSSWID) / 2, y);
      return;
    }
  }
}

/* draws the points, time and lifes left */
static void draw_data(int time, screenflag flags)
{
  char s[256];
  int t;
  int y = config.status_top() ? 5 : SCREENHEI - FONTHEI;

  if (time > 0) {
    snprintf(s, 256, "%u", time);
    scr_writetext_center(y, s);
  }

  snprintf(s, 256, "%u", pts_points());
  scr_writetext(5L, y, s);

  *s = '\0';
  if (pts_lifes() < 4)
    for (t = 0; t < pts_lifes(); t++)
      snprintf(s + strlen(s), 256-strlen(s), "%c", fonttoppler);
  else snprintf(s, 256, "%ix%c", pts_lifes(), fonttoppler);
  scr_writetext(SCREENWID - scr_textlength(s) - 5, y, s);

  y = config.status_top() ? SCREENHEI - FONTHEI : 5;
  switch (flags) {
    case SF_REC:  if (!(boxstate & 8)) scr_writetext_center(y, _("REC")); break;
    case SF_DEMO: scr_writetext_center(y, _("DEMO")); break;
    case SF_NONE:
    default:      break;
  }
}

void scr_drawall(long vert,
                 long angle,
                 long time,
                 bool svisible,
                 int subshape,
                 int substart,
                 screenflag flags
                ) {

  cleardesk(vert);

  sts_blink();
  sts_draw();
  draw_behind(vert * 4, angle);
  draw_tower(vert * 4, angle);

  if (config.use_shadows()) draw_shadows(vert*4, angle, false);

  draw_before(vert * 4, angle);

  if (snb_exists())
    scr_blit(objectsprites.data(snowballst),
             sintab[(snb_anglepos() + angle) % TOWER_ANGLES] + (SCREENWID / 2) - (SPR_HEROWID - SPR_AMMOWID),
             ((vert - snb_verticalpos()) * 4) + (SCREENHEI / 2) - SPR_AMMOHEI);

  if (top_visible()) {

      if (config.use_shadows())
          switch (top_shape())
          {
              // special cases for door enter and leave
              case 0x14:
              case 0x19:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2, 11);
                  break;

              case 0x15:
              case 0x18:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2, 7);
                  break;

              case 0x16:
              case 0x17:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2, 4);
                  break;

              // special cases for drowning
              case 31:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+6, 15);
                  break;
              case 32:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+11, 15);
                  break;
              case 33:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+17, 15);
                  break;
              case 34:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+23, 15);
                  break;
              case 35:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+28, 15);
                  break;
              case 36:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2+34, 15);
                  break;

              default:
                  putcircleshadow(0, (vert - top_verticalpos()) * 4 + (SCREENHEI / 2)-SPR_HEROHEI/2, 15);
                  break;
          }

      scr_blit(objectsprites.data(topplerstart + top_shape() +
                              ((top_look_left()) ?  mirror : 0)),
               (SCREENWID / 2) - (SPR_HEROWID / 2),
               (vert - top_verticalpos()) * 4 + (SCREENHEI / 2) - SPR_HEROHEI);

    if (top_onelevator())
    {
      // draw shadow of elevator platform

      if (config.use_shadows()) putcaseshadow(TB_ELEV_BOTTOM, 0, vert - top_verticalpos() + (SCREENHEI / 2), false);

      scr_blit(restsprites.data((angle % SPR_ELEVAFRAMES) + elevatorsprite), (SCREENWID / 2) - (SPR_ELEVAWID / 2),
               vert - top_verticalpos() + (SCREENHEI / 2));
    }
  }

  if (svisible) {
    scr_blit(objectsprites.data(subst + subshape),
             (SCREENWID / 2) - 70,
             (SCREENHEI / 2) + 12 - substart + 16);

  }

  putcross(vert);

  putbattlement(angle, vert * 4);

  putwater(vert);

  draw_data(time, flags);

  if (dcl_wait_overflow())
    scr_putbar(0, 0, 5, 5, 255, 0, 0, 255);

  boxstate = (boxstate + 1) & 0xf;
}

void scr_drawedit(long vpos, long apos, bool showtime) {

  long t;
  static long vert = 0, angle = 0;

  if (vpos != vert) {
    if (vpos > vert) {
      if (vpos > vert + 0x8)
        vert += 4;
      else
        vert += 1;
    } else {
      if (vpos < vert - 0x8)
        vert -= 4;
      else
        vert -= 1;
    }
  }

  apos &= 0x7f;

  t = (apos - angle) & (TOWER_ANGLES - 1);

  if (t != 0) {
    if (t < 0x3f) {
      if (t > 0x8)
        angle += 4;
      else
        angle += 1;
    } else {
      if (t < 0x7f-0x8)
        angle -= 4;
      else
        angle -= 1;
    }
    angle &= 0x7f;
  }

  cleardesk(vert);

  draw_behind_editor(vert * 4, angle, boxstate);
  draw_tower_editor(vert * 4, angle, boxstate);

  if (config.use_shadows()) draw_shadows(vert*4, angle, true);

  draw_before_editor(vert * 4, angle, boxstate);

  putbattlement(angle, vert * 4);

  putwater(vert);

  if (boxstate & 1) {
    scr_putrect((SCREENWID / 2) - (32 / 2), (SCREENHEI / 2) - 16, 32, 16,
                boxstate * 0xf, boxstate *0xf, boxstate *0xf, 128);
  }

  if (showtime) {
      char s[20];
      snprintf(s, 20, "%u", lev_towertime());
      scr_writetext_center(5, s);
  }

  boxstate = (boxstate + 1) & 0x1f;
}

static void put_scrollerlayer(long horiz, int layer) {
  horiz += scroll_layers[layer].xpos;
  horiz %= scroll_layers[layer].xrepeat;
  horiz = -horiz;

  while (horiz < SCREENWID)
  {
    scr_blit(layersprites.data(scroll_layers[layer].image), horiz, scroll_layers[layer].ypos);
    horiz += scroll_layers[layer].xrepeat;
  }
}

void scr_draw_bonus1(long horiz, long towerpos) {
  int l;

  if (config.use_full_scroller())
    for (l = 0; (l < num_scrolllayers) && (l < sl_tower_depth); l++)
      put_scrollerlayer(scroll_layers[l].num*horiz/scroll_layers[l].den, l);
  else
    put_scrollerlayer(scroll_layers[0].num*horiz/scroll_layers[0].den, 0);

  puttower(0, SCREENHEI/2, SCREENHEI, sl_tower_num*towerpos/sl_tower_den);
}

void scr_draw_bonus2(long horiz) {
  int l;

  if (config.use_full_scroller())
    for (l = sl_tower_depth; l < num_scrolllayers; l++)
      put_scrollerlayer(scroll_layers[l].num*horiz/scroll_layers[l].den, l);
  else
    put_scrollerlayer(scroll_layers[num_scrolllayers-1].num*horiz/scroll_layers[num_scrolllayers-1].den, num_scrolllayers-1);

  draw_data(-1, SF_NONE);
}

void scr_draw_submarine(long vert, long x, long number) {
  scr_blit(objectsprites.data(subst+number), x, vert);
}

void scr_draw_fish(long vert, long x, long number) {
  scr_blit(objectsprites.data(fishst+number), x, vert);
}

void scr_draw_torpedo(long vert, long x) {
  scr_blit(objectsprites.data(torb), x, vert);
}
