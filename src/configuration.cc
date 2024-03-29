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

#include "configuration.h"

#include "decl.h"
#include "keyb.h"

#include <algorithm>

static bool str2bool(const std::string & s)
{
    if (s == "yes" || s == "true")
        return true;
    else
        return (std::atoi(s.c_str()) != 0);
}

void configuration::parse(const std::string & inf)
{
    FILE * in = fopen(inf.c_str(), "r");

    if (!in) return;

    char line[201], param[201];

    while (!feof(in))
        if (fscanf(in, "%200s%*[\t ]%200s\n", line, param) == 2)
            for (auto & t : data)
                if (strstr(line, t.cnf_name.c_str()))
                {
                    switch (t.cnf_typ)
                    {
                        case CT_BOOL:
                            *(bool *)t.cnf_var = str2bool(param);
                            break;
                        case CT_STRING:
                            {
                                std::string tmp = param;
                                *(std::string *)t.cnf_var = std::string(tmp).substr(1, tmp.size()-2);
                            }
                            break;
                        case CT_INT:
                            *(int *)t.cnf_var = atoi(param);
                            break;
                        case CT_KEY:
                            if (atoi(param) > 0)
                                key_redefine((ttkey)t.maxlen, (SDL_Keycode)atoi(param));
                            break;
                        default:
                            assert_msg(0, "Unknown config data type.");
                            break;
                    }
                    break;
                }

    fclose(in);
}

void configuration::register_entry(const std::string & cnf_name, cnf_type  cnf_typ, void *cnf_var, long maxlen)
{
    config_data t;

    t.cnf_name = cnf_name;
    t.cnf_typ = cnf_typ;
    t.cnf_var = cnf_var;
    t.maxlen = maxlen;

    data.push_back(t);
}

#define CNF_BOOL(a,b) register_entry(a, CT_BOOL, b, 0)
#define CNF_CHAR(a,b) register_entry(a, CT_STRING, b, 0)
#define CNF_INT(a,b) register_entry(a, CT_INT, b, 0)
#define CNF_KEY(a,b) register_entry(a, CT_KEY, NULL, b)

configuration::configuration(const std::string & glob, const std::string & local)
{
    i_fullscreen = false;
    i_nosound  = false;
    i_use_water = true;
    i_use_alpha_sprites = true;
    i_use_alpha_layers = true;
    i_use_alpha_font = true;
    i_use_alpha_darkening = true;
    i_use_full_scroller = true;
    i_shadows = true;
    i_waves_type = waves_expensive;
    i_status_top = true;  /* is status line top or bottom of screen? */
    i_editor_towerpagesize = -1;
    i_editor_towerstarthei = -5;
    i_start_lives = 3;
    i_debug_level = 0;
    i_game_speed = DEFAULT_GAME_SPEED;
    i_nobonus = false;

    need_save = local.empty();

    CNF_BOOL( "fullscreen",          &i_fullscreen );
    CNF_BOOL( "nosound",             &i_nosound );
    CNF_BOOL( "nomusic",             &i_nomusic );
    CNF_CHAR( "editor_towername",    &i_editor_towername);
    CNF_BOOL( "use_alpha_sprites",   &i_use_alpha_sprites );
    CNF_BOOL( "use_alpha_font",      &i_use_alpha_font );
    CNF_BOOL( "use_alpha_layers",    &i_use_alpha_layers );
    CNF_BOOL( "use_alpha_darkening", &i_use_alpha_darkening );
    CNF_BOOL( "use_full_scroller",   &i_use_full_scroller );
    CNF_BOOL( "use_shadows",         &i_shadows );
    CNF_BOOL( "status_top",          &i_status_top );
    CNF_INT(  "editor_pagesize",     &i_editor_towerpagesize );
    CNF_INT(  "editor_towerstarthei",&i_editor_towerstarthei );
    CNF_INT(  "waves_type",          &i_waves_type );
    CNF_KEY(  "key_fire",             fire_key );
    CNF_KEY(  "key_up",               up_key );
    CNF_KEY(  "key_down",             down_key );
    CNF_KEY(  "key_left",             left_key );
    CNF_KEY(  "key_right",            right_key );
    CNF_CHAR( "password",            &i_curr_password);
    CNF_INT(  "start_lives",         &i_start_lives );
    CNF_INT(  "game_speed",          &i_game_speed);
    CNF_BOOL( "nobonus",             &i_nobonus);

    parse(glob);
    parse(local);

    fname = local;

    i_start_lives = clamp(i_start_lives, 1, 3);
    i_game_speed = clamp(i_game_speed, 0, MAX_GAME_SPEED);
}

configuration::~configuration(void)
{
    FILE * f = nullptr;

    if (need_save)
    {
        if (fname.empty())
            f = create_local_config_file(".toppler.rc");
        else
            f = fopen(fname.c_str(), "w");

        if (f)
        {
            for (auto & t : data)
            {
                fprintf(f, "%s: ", t.cnf_name.c_str());

                switch (t.cnf_typ)
                {
                    case CT_BOOL:
                        fprintf(f, "%s", (*(bool *)t.cnf_var)?("yes"):("no"));
                        break;
                    case CT_STRING:
                        fprintf(f, "\"%s\"", ((std::string*)(t.cnf_var))->c_str());
                        break;
                    case CT_INT:
                        fprintf(f, "%i", *(int *)t.cnf_var);
                        break;
                    case CT_KEY:
                        fprintf(f, "%i", (int)key_conv2sdlkey((ttkey)t.maxlen, true));
                        break;
                    default:
                        assert_msg(0, "Unknown config data type.");
                        break;
                }

                fprintf(f, "\n");
            }
            fclose(f);
        }
    }
}

void configuration::debug_level(int l)
{
    need_save = true;
    i_debug_level = l;
    dcl_setdebuglevel(l);
}

configuration config(local_file_name(".toppler.rc"), local_file_name(".toppler.rc"));
