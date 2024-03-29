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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <vector>
#include <string>

// TODO remove once everything is converted
#define TOWERNAMELEN 19
#define PASSWORD_LEN 5

/* this module contains a class for configuration file
 * handling loading and saving is handled
 */

class configuration {

public:

  configuration(const std::string & glob, const std::string & local);
  ~configuration();

  bool fullscreen() const { return i_fullscreen; }
  void fullscreen(bool on) { need_save = true; i_fullscreen = on; }

  bool nosound() const { return i_nosound; }
  void nosound(bool on) { need_save = true; i_nosound = on; }

  bool nomusic() const { return i_nomusic; }
  void nomusic(bool on) { need_save = true; i_nomusic = on; }

  bool use_water() const { return i_use_water; }
  void use_water(bool on) { need_save = true; i_use_water = on; }

  const std::string & editor_towername() const { return i_editor_towername; }
  void editor_towername(const std::string & name) { need_save = true; i_editor_towername = name; }

  bool use_alpha_sprites() const { return i_use_alpha_sprites; }
  void use_alpha_sprites(bool on) { need_save = true; i_use_alpha_sprites = on; }

  bool use_alpha_layers() const { return i_use_alpha_layers; }
  void use_alpha_layers(bool on) { need_save = true; i_use_alpha_layers = on; }

  bool use_alpha_font() const { return i_use_alpha_font; }
  void use_alpha_font(bool on) { need_save = true; i_use_alpha_font = on; }

  bool use_alpha_darkening() const { return i_use_alpha_darkening; }
  void use_alpha_darkening(bool on) { need_save = true; i_use_alpha_darkening = on; }

  bool use_full_scroller() const { return i_use_full_scroller; }
  void use_full_scroller(bool on) { need_save = true; i_use_full_scroller = on; }

  bool use_shadows() const { return i_shadows; }
  void use_shadows(bool on) { need_save = true; i_shadows = on; }


  /* the different types of waves used in waves_type */
  enum {
    waves_nonreflecting,
    waves_simple,
    waves_expensive,
    num_waves
  };

  int  waves_type() const { return i_waves_type; }
  void waves_type(int type) { need_save = true; i_waves_type = type; }

  bool status_top() const { return i_status_top; }
  void status_top(bool on) { need_save = true; i_status_top = on; }

  int  editor_towerpagesize() const { return i_editor_towerpagesize; }
  void editor_towerpagesize(int sz) { need_save = true; i_editor_towerpagesize = sz; }

  int  editor_towerstarthei() const { return i_editor_towerstarthei; }
  void editor_towerstarthei(int sz) { need_save = true; i_editor_towerstarthei = sz; }

  int  start_lives() const { return i_start_lives; }
  void start_lives(int lv) { need_save = true; i_start_lives = lv; }

  const std::string & curr_password() const { return i_curr_password; }
  void curr_password(const std::string & pwd) { need_save = true; i_curr_password = pwd; }

  int  debug_level() const { return i_debug_level; }
  void debug_level(int l);

  int  game_speed() const { return i_game_speed; }
  void game_speed(int spd) { need_save = true; i_game_speed = spd; }

  int  nobonus() const { return i_nobonus; }
  void nobonus(bool on) { need_save = true; i_nobonus = on; }

  void requestsave() { need_save = true; }

private:

  std::string fname;

  typedef enum {
      CT_BOOL,
      CT_STRING,
      CT_INT,
      CT_KEY
  } cnf_type;

  void parse(const std::string & in);
  void register_entry(const std::string & cnf_name, cnf_type  cnf_typ, void *cnf_var, long maxlen);

  typedef struct config_data {
    std::string cnf_name;
    cnf_type  cnf_typ;
    void     *cnf_var;
    long      maxlen;
  } config_data;

  std::vector<config_data> data;

  bool i_fullscreen;
  bool i_nosound;
  bool i_nomusic;
  bool i_use_water;
  std::string i_editor_towername;
  bool i_use_alpha_sprites;
  bool i_use_alpha_layers;
  bool i_use_alpha_font;
  bool i_use_alpha_darkening;
  bool i_use_full_scroller;
  bool i_shadows;
  int  i_waves_type;
  bool i_status_top;
  int  i_editor_towerpagesize;
  int  i_editor_towerstarthei;
  int  i_start_lives;
  std::string i_curr_password;
  int  i_debug_level;
  int  i_game_speed;
  int  i_nobonus;

  bool need_save;

};

extern configuration config;

#endif
