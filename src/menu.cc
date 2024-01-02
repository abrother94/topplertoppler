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

#include "menu.h"

#include "game.h"
#include "points.h"
#include "bonus.h"
#include "sprites.h"
#include "archi.h"
#include "screen.h"
#include "keyb.h"
#include "decl.h"
#include "level.h"
#include "sound.h"
#include "leveledit.h"
#include "stars.h"
#include "robots.h"
#include "configuration.h"
#include "highscore.h"
#include "menusys.h"

#include <SDL_endian.h>

#include <stdlib.h>
#include <string.h>

#define NUMHISCORES 10
#define HISCORES_PER_PAGE 5

static unsigned short menupicture, titledata;
static unsigned char currentmission = 0;

static void men_reload_sprites(Uint8 what) {
  Uint8 pal[3*256];

  if (what & 1) {
    auto fi = dataarchive->open(menudat);

    scr_read_palette(&fi, pal);
    menupicture = scr_loadsprites(&restsprites, &fi, 1, 1280, 480, false, pal, 0);
  }

  if (what & 2) {
    auto fi = dataarchive->open(titledat);

    scr_read_palette(&fi, pal);
    titledata = scr_loadsprites(&fontsprites, &fi, 1, SPR_TITLEWID, SPR_TITLEHEI, true, pal, config.use_alpha_font());
  }
}

#ifdef GAME_DEBUG_KEYS
static const char *debug_menu_extralife(_menusystem *ms) {
  if (ms) lives_add();
  return _("Extra Life");
}

static const char *debug_menu_extrascore(_menusystem *ms) {
  if (ms) pts_add(200);
  return _("+200 Points");
}
#endif /* GAME_DEBUG_KEYS */

static std::string
men_main_background_proc(_menusystem *ms)
{
  if (ms) {
    scr_blit_center(restsprites.data(menupicture));
    scr_blit(fontsprites.data(titledata), (SCREENWID - fontsprites.data(titledata)->w) / 2, 20);
  }
  return "";
}

#define REDEFINEREC 5
static int times_called = 0;
static std::string redefine_menu_up(_menusystem *ms) {
  static char buf[50];
  const std::string code[REDEFINEREC] = {_("Up"), _("Down"), _("Left"), _("Right"), _("Fire")};
  const char *keystr;
  static int blink, times_called;
  const ttkey key[REDEFINEREC] = {up_key, down_key, left_key, right_key, fire_key};
  const char *redef_fmt = "%s:  %s";
  buf[0] = '\0';
  if (ms) {
    switch (ms->mstate) {
    default:
      if (key_sdlkey2conv(ms->key, false) == fire_key) {
        ms->mstate = 1;
        ms->opt_steal_control = ms->hilited;
        ms->key = SDLK_UNKNOWN;
      }
      break;
    case 1:
      if (ms->key != SDLK_UNKNOWN) {
        key_redefine(key[ms->hilited % REDEFINEREC], ms->key);
        config.requestsave();
        ms->mstate = 2;
        ms->opt_steal_control = ms->hilited;
        blink = 0;
      } else blink++;
      break;
    case 2:
      ms->mstate = 0;
      ms->opt_steal_control = -1;
      break;
    }
    if ((blink & 4) || (ms->mstate != 1))
      keystr = SDL_GetKeyName(key_conv2sdlkey(key[ms->hilited % REDEFINEREC], true));
    else keystr = "";
    snprintf(buf, 50, redef_fmt, code[ms->hilited % REDEFINEREC].c_str(), keystr);
  } else {
    keystr = SDL_GetKeyName(key_conv2sdlkey(key[times_called], true));
    snprintf(buf, 50, redef_fmt, code[times_called].c_str(), keystr);
    times_called = (times_called + 1) % REDEFINEREC;
  }
  return buf;
}

static std::string game_options_menu_password(_menusystem *prevmenu) {

  if (prevmenu) {
    std::string pwd = config.curr_password();
    while (!men_input(pwd, PASSWORD_LEN, -1, -1, PASSWORD_CHARS)) ;
    config.curr_password(pwd);
    /* FIXME: change -1, -1 to correct position; Need to fix menu system
     first... */
  }
  return _("Password: ") + config.curr_password();
}

static std::string game_options_menu_statustop(_menusystem *prevmenu) {
  std::string txt = _("Status on top") + " ";

  if (prevmenu) {
    config.status_top(!config.status_top());
  }

  if (config.status_top())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string game_options_menu_lives(_menusystem *prevmenu) {
  std::string buf = _("Lives: ");

  if (prevmenu) {
    switch (key_sdlkey2conv(prevmenu->key, false)) {
    case right_key:
      config.start_lives(config.start_lives() + 1);
      if (config.start_lives() > 3) config.start_lives(3);
      break;
    case left_key:
      config.start_lives(config.start_lives() - 1);
      if (config.start_lives() < 1) config.start_lives(1);
      break;
    default: return "";
    }
  }

  for (int i = 0; i < config.start_lives(); i++)
    buf += fonttoppler;

  return buf;
}

static std::string game_options_menu_speed(_menusystem *prevmenu)
{
  // Changing game_speed during a game has no effect until a
  // a new game is started.
  std::string buf = _("Game Speed:") + " ";
  if (prevmenu) {
    switch (key_sdlkey2conv(prevmenu->key, false)) {
    case right_key:
      config.game_speed(config.game_speed() + 1);
      if (config.game_speed() > MAX_GAME_SPEED) config.game_speed(MAX_GAME_SPEED);
      break;
    case left_key:
      config.game_speed(config.game_speed() - 1);
      if (config.game_speed() < 0) config.game_speed(0);
      break;
    case fire_key:
      config.game_speed((config.game_speed() + 1) % (MAX_GAME_SPEED+1));
      break;
    default: return "";
    }
  }

  buf += std::to_string(config.game_speed());
  return buf;
}

static std::string game_options_bonus(_menusystem *ms)
{
  std::string txt = _("Bonus") + " ";

  if (ms)
    config.nobonus(!config.nobonus());

  if (config.nobonus())
      txt += fontemptytagbox;
  else
      txt += fonttagedtagbox;

  return txt;
}


static std::string men_game_options_menu(_menusystem *prevmenu) {
  static auto s = _("Game Options");
  if (prevmenu) {
    _menusystem *ms = new_menu_system(s, NULL, 0, fontsprites.data(titledata)->h+30);

    ms = add_menu_option(ms, "", game_options_menu_password, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, "", game_options_menu_lives, SDLK_UNKNOWN, (menuoptflags)((int)MOF_PASSKEYS|(int)MOF_LEFT));
    ms = add_menu_option(ms, "", game_options_menu_statustop);
    ms = add_menu_option(ms, "", game_options_menu_speed, SDLK_UNKNOWN, (menuoptflags)((int)MOF_PASSKEYS|(int)MOF_LEFT));
    ms = add_menu_option(ms, "", game_options_bonus);

    ms = add_menu_option(ms, "", NULL);
    ms = add_menu_option(ms, _("Back"), NULL);

    ms = run_menu_system(ms, prevmenu);

    free_menu_system(ms);
  }
  return s;
}

static std::string run_redefine_menu(_menusystem *prevmenu) {
  if (prevmenu) {
    _menusystem *ms = new_menu_system(_("Redefine Keys"), NULL, 0, fontsprites.data(titledata)->h+30);

    times_called = 0;

    ms = add_menu_option(ms, "", redefine_menu_up, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, "", redefine_menu_up, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, "", redefine_menu_up, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, "", redefine_menu_up, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, "", redefine_menu_up, SDLK_UNKNOWN, MOF_LEFT);
    ms = add_menu_option(ms, _("Back"), NULL);

    ms = run_menu_system(ms, prevmenu);

    free_menu_system(ms);
  }
  return _("Redefine Keys");
}

static std::string men_options_windowed(_menusystem *ms)
{
  std::string txt = _("Fullscreen") + " ";

  if (ms) {
    config.fullscreen(!config.fullscreen());
    scr_reinit();
    SDL_ShowCursor(config.fullscreen() ? 0 : 1);
  }

  if (config.fullscreen())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string men_options_sounds(_menusystem *ms)
{
  std::string txt = _("Sounds") + " ";

  if (ms) {
    if (config.nosound()) {
      config.nosound(false);
      snd_init();
      if (!config.nomusic())
        snd_playTitle();
    } else {
      if (!config.nomusic())
        snd_stopTitle();
      snd_done();
      config.nosound(true);
    }
  }

  if (config.nosound())
      txt += fontemptytagbox;
  else
      txt += fonttagedtagbox;

  return txt;
}

static std::string men_options_music(_menusystem *ms)
{
  std::string txt = _("Music") + " ";

  if (ms) {
    if (config.nomusic()) {
      config.nomusic(false);
      snd_playTitle();
    } else {
      snd_stopTitle();
      config.nomusic(true);
    }
  }

  if (config.nomusic())
      txt += fontemptytagbox;
  else
      txt += fonttagedtagbox;

  return txt;
}


static void reload_font_graphics(void)
{
  fontsprites.freedata();

  scr_reload_sprites(RL_FONT);
  men_reload_sprites(2);
}

static void reload_robot_graphics(void)
{
  objectsprites.freedata();
  scr_reload_sprites(RL_OBJECTS);
}

static void reload_layer_graphics(void)
{
  layersprites.freedata();
  scr_reload_sprites(RL_SCROLLER);
}

static std::string men_alpha_font(_menusystem *ms)
{
  std::string txt = _("Font alpha") + " ";

  if (ms) {
    config.use_alpha_font(!config.use_alpha_font());
    reload_font_graphics();
  }

  if (config.use_alpha_font())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string men_alpha_sprites(_menusystem *ms)
{
  std::string txt = _("Sprites alpha") + " ";

  if (ms) {
    config.use_alpha_sprites(!config.use_alpha_sprites());
    reload_robot_graphics();
  }

  if (config.use_alpha_sprites())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string men_alpha_layer(_menusystem *ms)
{
  std::string txt = _("Scroller alpha") + " ";

  if (ms) {
    config.use_alpha_layers(!config.use_alpha_layers());
    reload_layer_graphics();
  }

  if (config.use_alpha_layers())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string men_alpha_menu(_menusystem *ms)
{
  std::string txt = _("Shadowing") + " ";

  if (ms) {
    config.use_alpha_darkening(!config.use_alpha_darkening());
  }

  if (config.use_alpha_darkening())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}

static std::string men_waves_menu(_menusystem *ms)
{
  if (ms) {
    switch (key_sdlkey2conv(ms->key, false)) {
    case fire_key:
      config.waves_type((config.waves_type() + 1) % configuration::num_waves);
      break;
    case right_key:
      config.waves_type(config.waves_type() + 1);
      if (config.waves_type() >= configuration::num_waves) config.waves_type(configuration::num_waves - 1);
      break;
    case left_key:
      config.waves_type(config.waves_type() - 1);
      if (config.waves_type() < 0) config.waves_type(0);
      break;
    default: return "";
    }
  }
  switch(config.waves_type()) {
  case configuration::waves_nonreflecting: return _("Nonreflecting waves");
  case configuration::waves_simple: return _("Simple waves");
  case configuration::waves_expensive: return _("Expensive waves");
  default: return _("Error");
  }
}

static std::string men_full_scroller(_menusystem *ms)
{
  if (ms) {
    config.use_full_scroller(!config.use_full_scroller());
  }
  if (config.use_full_scroller()) return _("Complete Scroller");
  else return _("2 layers Scoller");
}

static std::string men_shadows(_menusystem *ms)
{
  std::string txt = _("Shadows") + " ";

  if (ms) {
    config.use_shadows(!config.use_shadows());
  }

  if (config.use_shadows())
      txt += fonttagedtagbox;
  else
      txt += fontemptytagbox;

  return txt;
}


static std::string men_alpha_options(_menusystem *mainmenu) {
  static auto s = _("Alpha Options");
  if (mainmenu) {

    _menusystem *ms = new_menu_system(s, NULL, 0, fontsprites.data(titledata)->h+30);

    if (!ms) return "";

    ms = add_menu_option(ms, "", men_alpha_font, SDLK_UNKNOWN, MOF_RIGHT);
    ms = add_menu_option(ms, "", men_alpha_sprites, SDLK_UNKNOWN, MOF_RIGHT);
    ms = add_menu_option(ms, "", men_alpha_layer, SDLK_UNKNOWN, MOF_RIGHT);
    ms = add_menu_option(ms, "", men_alpha_menu, SDLK_UNKNOWN, MOF_RIGHT);

    ms = add_menu_option(ms, "", NULL);
    ms = add_menu_option(ms, _("Back"), NULL);

    ms = run_menu_system(ms, mainmenu);

    free_menu_system(ms);
  }
  return s;
}

static std::string men_options_graphic(_menusystem *mainmenu) {
  static auto s = _("Graphics");
  if (mainmenu) {

    _menusystem *ms = new_menu_system(s, NULL, 0, fontsprites.data(titledata)->h+30);

    if (!ms) return "";

    ms = add_menu_option(ms, "", men_options_windowed);
    ms = add_menu_option(ms, "", men_alpha_options);
    ms = add_menu_option(ms, "", men_waves_menu, SDLK_UNKNOWN, MOF_PASSKEYS);
    ms = add_menu_option(ms, "", men_full_scroller, SDLK_UNKNOWN);
    ms = add_menu_option(ms, "", men_shadows, SDLK_UNKNOWN);

    ms = add_menu_option(ms, "", NULL);
    ms = add_menu_option(ms, _("Back"), NULL);

    ms = run_menu_system(ms, mainmenu);

    free_menu_system(ms);
  }
  return s;
}

static std::string men_options(_menusystem *mainmenu) {
  static auto s = _("Options");
  if (mainmenu) {

    _menusystem *ms = new_menu_system(s, NULL, 0, fontsprites.data(titledata)->h+30);

    if (!ms) return "";

    ms = add_menu_option(ms, "", men_game_options_menu);
    ms = add_menu_option(ms, "", run_redefine_menu);
    ms = add_menu_option(ms, "", men_options_graphic);
    ms = add_menu_option(ms, "", men_options_sounds);
    ms = add_menu_option(ms, "", men_options_music);

    ms = add_menu_option(ms, "", NULL);
    ms = add_menu_option(ms, _("Back"), NULL);

    ms = run_menu_system(ms, mainmenu);

    free_menu_system(ms);
  }
  return s;
}

static int hiscores_timer = 0;
static int hiscores_pager = 0;
static int hiscores_state = 0;
static int hiscores_xpos = SCREENWID;
static int hiscores_hilited = -1;
static int hiscores_maxlen_pos = 0;
static int hiscores_maxlen_points = 0;
static int hiscores_maxlen_name = 0;
static int hiscores_maxlen = 0;

static void get_hiscores_string(int p, std::string & pos, std::string & points, std::string & name)
{
  Uint32 pt;
  Uint8 tw;
  std::string buf3;

  hsc_entry(p, buf3, pt, tw);

  pos = std::to_string(p)+".";
  points = std::to_string(pt);
  name = buf3;
}

static void
calc_hiscores_maxlen(int *max_pos, int * max_points, int *max_name)
{
  for (int x = 0; x < hsc_entries(); x++) {
    std::string a, b, c;
    int clen;

    get_hiscores_string(x, a, b, c);

    clen = scr_textlength(a);
    if (clen > *max_pos) *max_pos = clen;

    clen = scr_textlength(b);
    if (clen < 64) clen = 64;
    if (clen > *max_points) *max_points = clen;

    clen = scr_textlength(c);
    if (clen > *max_name) *max_name = clen;
  }
}

static std::string men_hiscores_background_proc(_menusystem *ms)
{
  static int blink_r = 120, blink_g = 200, blink_b = 40;
  static int next_page = 0;

  if (ms) {

    scr_blit_center(restsprites.data(menupicture));
    scr_blit(fontsprites.data(titledata), (SCREENWID - fontsprites.data(titledata)->w) / 2, 20);

    switch (hiscores_state) {
    case 0: /* bring the scores in */
      if (hiscores_xpos > ((SCREENWID - hiscores_maxlen) / 2)) {
        hiscores_xpos -= 10;
        break;
      } else hiscores_state = 1;
      [[fallthrough]];
    case 1: /* hold the scores on screen */
      if (hiscores_timer < 100) {
        hiscores_timer++;
        break;
      } else {
        bool filled_page = false;
        bool firstpage = (hiscores_pager == 0);
        int pager = (hiscores_pager + 1) % (NUMHISCORES / HISCORES_PER_PAGE);
        for (int tmp = 0; tmp < HISCORES_PER_PAGE; tmp++) {
          //          int cs = tmp + (pager * HISCORES_PER_PAGE);
          //          if (scores[cs].points || strlen(scores[cs].name)) {
          filled_page = true;
          break;
          //          }
        }
        if (!filled_page && firstpage) {
          hiscores_timer = 0;
          break;
        } else {
          hiscores_state = 2;
          next_page = pager;
        }
      }
      [[fallthrough]];
    case 2: /* move the scores out */
      if (hiscores_xpos > -(hiscores_maxlen + 40)) {
        hiscores_timer = 0;
        hiscores_xpos -= 10;
        break;
      } else {
        hiscores_state = 0;
        hiscores_xpos = SCREENWID;
        hiscores_pager = next_page;
      }
    default: break;
    }
    for (int t = 0; t < HISCORES_PER_PAGE; t++) {
      int cs = t + (hiscores_pager * HISCORES_PER_PAGE);
      int ypos = (t*(FONTHEI+1)) + fontsprites.data(titledata)->h + FONTHEI*2;
      std::string pos, points, name;
      get_hiscores_string(cs, pos, points, name);
      if (cs == hiscores_hilited) {
        int clen = hiscores_maxlen_pos + hiscores_maxlen_points + hiscores_maxlen_name + 20 * 2 + 20;
        scr_putbar(hiscores_xpos - 5, ypos - 3,
                   clen, FONTHEI + 3, blink_r, blink_g, blink_b, (config.use_alpha_darkening())?128:255);
      }
      scr_writetext(hiscores_xpos + hiscores_maxlen_pos - scr_textlength(pos), ypos, pos);
      scr_writetext(hiscores_xpos + hiscores_maxlen_pos + 20 + hiscores_maxlen_points - scr_textlength(points), ypos, points);
      scr_writetext(hiscores_xpos + hiscores_maxlen_pos + 20 + 20 + hiscores_maxlen_points, ypos, name);
    }
    scr_color_ramp(&blink_r, &blink_g, &blink_b);
  }
  return _("HighScores");
}

static void show_scores(bool back = true, int mark = -1) {
  std::string buf = _("Scores for") + " " + lev_missionname(currentmission);
  _menusystem *ms = new_menu_system(buf, men_hiscores_background_proc, 0, fontsprites.data(titledata)->h + 30);

  if (!ms) return;

  hsc_select(lev_missionname(currentmission));

  hiscores_timer = 0;
  if ((mark >= 0) && (mark < NUMHISCORES))
    hiscores_pager = (mark / HISCORES_PER_PAGE);
  else
    hiscores_pager = 0;
  hiscores_state = 0;
  calc_hiscores_maxlen(&hiscores_maxlen_pos, &hiscores_maxlen_points, &hiscores_maxlen_name);
  hiscores_maxlen = hiscores_maxlen_pos + hiscores_maxlen_points + hiscores_maxlen_name + 20;
  hiscores_xpos = SCREENWID;
  hiscores_hilited = mark;

  /* fake options; the empty lines are used by the background proc */
  for (int tmpz = 0; tmpz < HISCORES_PER_PAGE; tmpz++) ms = add_menu_option(ms, "", NULL);

  if (back)
    ms = add_menu_option(ms, _("Back"), NULL);
  else
    ms = add_menu_option(ms, _("OK"), NULL);

  ms = run_menu_system(ms, 0);

  free_menu_system(ms);
}

static void
congrats_background_proc(void)
{
  scr_blit_center(restsprites.data(menupicture));
  scr_blit(fontsprites.data(titledata), (SCREENWID - fontsprites.data(titledata)->w) / 2, 20);

  /* you can use up to 4 lines of text here, but please check
   * if the text fits onto the screen
   */
  auto text = _("Congratulations! You are\n"
                "probably good enough to\n"
                "enter the highscore table!");

  int ypos = 210;

  for (auto c : text)
    if (c == '\n') ypos -= 40;

  std::string line;

  for (auto c : text)
  {
    if (c == '\n') {
      scr_writetext_center(ypos, line);
      line = "";
      ypos += 40;
    } else {
      line += c;
    }
  }

  if (!line.empty())
      scr_writetext_center(ypos, line);

  scr_writetext_center(270, _("Please enter your name"));
}

/* highscores, after the game
 * pt = points,
 * twr = tower reached, -1 = mission finished
 */
static void men_highscore(unsigned long pt, int twr) {

  Uint8 pos = 0xff;

#ifndef GAME_DEBUG_KEYS

  hsc_select(lev_missionname(currentmission));

  /* check, if there is a chance at all to get into the list,
   * if not we don't need to lock the highscoretable
   */
  if (hsc_canEnter(pt)) {

    set_men_bgproc(congrats_background_proc);

    std::string name;

#ifndef _WIN32
    /* copy the login name into the name entered into the highscore table */
    name = getenv("LOGNAME");
#endif

    while (!men_input(name, SCORENAMELEN-1)) ;

    pos = hsc_enter(pt, twr, name);
  }

#endif /* GAME_DEBUG_KEYS */

  show_scores(false, pos);
}

static void
main_game_loop()
{
  unsigned char tower;
  Uint8 anglepos;
  Uint16 resttime;
  std::vector<Uint16> dummydemo;
  int gameresult;

  if (!lev_loadmission(currentmission)) {
    if (!men_yn(_("This mission contains\n"
                "unknown building blocks.\n"
                "You probably need a new\n"
                "version of Tower Toppler.\n"
                "Do you want to continue?"), false, men_main_background_proc))
      return;
  }

  tower = lev_tower_passwd_entry(config.curr_password());

  gam_newgame();
  bns_restart();

  do {
    ttsounds::instance()->startsound(SND_WATER);
    do {
      gam_loadtower(tower);
      scr_settowercolor(lev_towercol_red(), lev_towercol_green(), lev_towercol_blue());
      ttsounds::instance()->setsoundvol(SND_WATER, 128);
      gam_arrival();
      gameresult = gam_towergame(anglepos, resttime, dummydemo, 0);
    } while ((gameresult == GAME_DIED) && pts_lifesleft());

    if (gameresult == GAME_FINISHED) {
      gam_pick_up(anglepos, resttime);

      ttsounds::instance()->stopsound(SND_WATER);
      tower++;

      if (tower < lev_towercount()) {

        // load next tower, because its colors will be needed for bonus game
        gam_loadtower(tower);

        if (!config.nobonus())
          if (!bns_game())
            gameresult = GAME_ABORTED;
      }
    } else {
      ttsounds::instance()->stopsound(SND_WATER);
    }
  } while (pts_lifesleft() && (tower < lev_towercount()) && (gameresult != GAME_ABORTED));

  if (gameresult != GAME_ABORTED)
    men_highscore(pts_points(), (tower >= lev_towercount()) ? tower : -1);
}

#ifdef HUNT_THE_FISH
static const char *
men_main_bonusgame_proc(_menusystem *ms)
{
  if (ms) {
    gam_newgame();
    scr_settowercolor(rand() % 256, rand() % 256, rand() % 256);
    lev_set_towercol(rand() % 256, rand() % 256, rand() % 256);
    bns_game();
  }
  return _("Hunt the Fish");
}
#endif /* HUNT_THE_FISH */

static std::string
men_main_startgame_proc(_menusystem *ms)
{
  if (ms) {
    int missioncount = lev_missionnumber();
    switch (key_sdlkey2conv(ms->key, false)) {
    case fire_key:
      dcl_update_speed(config.game_speed());
      snd_musicVolume(MIX_MAX_VOLUME/4);
      main_game_loop();
      snd_musicVolume(MIX_MAX_VOLUME);
      dcl_update_speed(MENU_DCLSPEED);
      break;
    case right_key: currentmission = (currentmission + 1) % missioncount; break;
    case left_key: currentmission = (currentmission + missioncount - 1) % missioncount; break;
    default: return "";
    }
  }

  return std::string("") + fontptrleft + " " + _("Start:") + " " + _(lev_missionname(currentmission)) + " " + fontptrright;
}

static std::string
men_main_highscore_proc(_menusystem *ms)
{
  if (ms) {
    show_scores();
  }
  return _("Highscores");
}

static std::string
men_main_leveleditor_proc(_menusystem *ms)
{
  if (ms) {
    le_edit();
    (void)key_sdlkey();
  }
  return _("Level Editor");
}

static std::string
men_main_timer_proc(_menusystem *ms)
{
  if (ms) {
    Uint16 num_demos = 0;
    Uint8 demos[256];
    Uint16 miss = rand() % lev_missionnumber();
    Uint8 num_towers;

    Uint8 anglepos;
    Uint16 resttime;

    for (int tmpm = 0; (tmpm < lev_missionnumber()) && (num_demos == 0); tmpm++) {
      Uint16 tmiss = (miss + tmpm) % lev_missionnumber();

      if (lev_loadmission(tmiss)) {
        num_towers = lev_towercount();

        for (Uint8 idx = 0; (idx < num_towers) && (num_demos < 256); idx++) {
          lev_selecttower(idx);
          auto demo = lev_get_towerdemo();
          if (!demo.empty()) demos[num_demos++] = idx;
        }
      }
    }

    if (num_demos < 1) return "";

    lev_selecttower(demos[rand() % num_demos]);
    auto demo = lev_get_towerdemo();

    dcl_update_speed(config.game_speed());
    gam_newgame();
    ttsounds::instance()->startsound(SND_WATER);
    scr_settowercolor(lev_towercol_red(), lev_towercol_green(), lev_towercol_blue());
    ttsounds::instance()->setsoundvol(SND_WATER, 128);
    rob_initialize();
    (void)gam_towergame(anglepos, resttime, demo, 0);
    ttsounds::instance()->stopsound(SND_WATER);
    dcl_update_speed(MENU_DCLSPEED);
  }
  return "";
}

static std::string men_game_return2game(_menusystem *tms)
{
  if (tms) {
    tms->exitmenu = true;
    tms->mstate = 0;
  }
  return _("Return to Game");
}

static std::string men_game_leavegame(_menusystem *tms)
{
  if (tms) {
    tms->exitmenu = true;
    tms->mstate = 1;
  }
  return _("Quit Game");
}



#ifdef GAME_DEBUG_KEYS
void run_debug_menu(void) {
  _menusystem *ms = new_menu_system(_("DEBUG MENU"), NULL, 0, SCREENHEI / 5);

  ms = add_menu_option(ms, "", debug_menu_extralife);
  ms = add_menu_option(ms, "", debug_menu_extrascore);
  ms = add_menu_option(ms, "", NULL);
  ms = add_menu_option(ms, _("Back to Game"), NULL);

  ms = run_menu_system(ms, 0);

  free_menu_system(ms);
}
#endif

void men_init(void) {
  men_reload_sprites(3);
}

void men_main() {
  _menusystem *ms;

  ms = new_menu_system("", men_main_background_proc, 0, fontsprites.data(titledata)->h + 30);

  if (!ms) return;

  ms = set_menu_system_timeproc(ms, 200, men_main_timer_proc);

  ms = add_menu_option(ms, "", men_main_startgame_proc, SDLK_s, MOF_PASSKEYS);
  ms = add_menu_option(ms, "", NULL);
  ms = add_menu_option(ms, "", men_main_highscore_proc, SDLK_h);
  ms = add_menu_option(ms, "", men_options, SDLK_o);
  ms = add_menu_option(ms, "", men_main_leveleditor_proc, SDLK_e);
#ifdef HUNT_THE_FISH
  ms = add_menu_option(ms, "", men_main_bonusgame_proc);
#endif
  ms = add_menu_option(ms, "", NULL);
  ms = add_menu_option(ms, _("Quit"), NULL, SDLK_q);

  ms->wraparound = true;

  ms = run_menu_system(ms, 0);

  free_menu_system(ms);
}

bool men_game() {
  _menusystem *ms;
  bool do_quit;
  int  speed = dcl_update_speed(MENU_DCLSPEED);

  ms = new_menu_system("", NULL, 0, fontsprites.data(titledata)->h + 30);

  if (!ms) return 0;

  ms = add_menu_option(ms, "", men_game_return2game);
  ms = add_menu_option(ms, "", men_options, SDLK_o);
  ms = add_menu_option(ms, "", NULL);
  ms = add_menu_option(ms, "", men_game_leavegame);

  ms->wraparound = true;

  ms = run_menu_system(ms, 0);

  do_quit = ms->mstate != 0;

  free_menu_system(ms);

  dcl_update_speed(speed);

  return do_quit;
}

void men_done(void) {
}

