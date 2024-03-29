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

#include "decl.h"
#include "configuration.h"

#include <SDL.h>

#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifndef _WIN32
#include <pwd.h>
#endif

#if ENABLE_NLS == 1
#include <libintl.h>
#endif

static bool wait_overflow = false;
/* Not read from config file */
int  curr_scr_update_speed = MENU_DCLSPEED;

int dcl_update_speed(int spd) {
    int tmp = curr_scr_update_speed;
    curr_scr_update_speed = spd;
    return tmp;
}

void dcl_wait(void) {
  static Uint32 last;

  if (SDL_GetTicks() >= last + (Uint32)(55-(curr_scr_update_speed*5))) {
    wait_overflow = true;
    last = SDL_GetTicks();
    return;
  }

  wait_overflow = false;
  while ((SDL_GetTicks() - last) < (Uint32)(55-(curr_scr_update_speed*5)) ) SDL_Delay(2);
  last = SDL_GetTicks();
}

bool dcl_wait_overflow(void) { return wait_overflow; }

static int current_debuglevel;

void dcl_setdebuglevel(int level) {
  current_debuglevel = level;
}


void debugprintf(int lvl, const char *fmt, ...) {
    if (lvl <= current_debuglevel) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

/* returns true, if file exists, this is not the
 optimal way to do this. it would be better to open
 the dir the file is supposed to be in and look there
 but this is not really portable so this
 */
bool dcl_fileexists(const std::string & n) {

  FILE *f = fopen(n.c_str(), "r");

  if (f) {
    fclose(f);
    return true;
  } else
    return false;
}

std::string homedir()
{

#ifndef _WIN32

  return getenv("HOME");

#else

  return "./";

#endif

}

/* checks if home/.toppler exists and creates it, if not */
static void checkdir(void) {

#ifndef _WIN32

  std::string n = homedir() + "/.toppler";

  DIR *d = opendir(n.c_str());

  if (!d) {
    mkdir(n.c_str(), S_IRWXU);
  }

  closedir(d);
#endif

}

FILE *open_data_file(const std::string & name) {


#ifndef _WIN32
  FILE *f = NULL;
  // look into actual directory
  if (dcl_fileexists(name))
    return fopen(name.c_str(), "rb");

  // look into the data dir
  std::string n = std::string(TOP_DATADIR) + "/" + name;

  if (dcl_fileexists(n))
    f = fopen(n.c_str(), "rb");

  return f;

#else

  if (dcl_fileexists(name))
    return fopen(name.c_str(), "rb");

  return NULL;

#endif
}

bool get_data_file_path(const std::string & name, std::string & f) {

#ifndef _WIN32
  // look into actual directory
  if (dcl_fileexists(name)) {
    f = name;
    return true;
  }

  // look into the data dir
  std::string n = std::string(TOP_DATADIR) + "/" + name;

  if (dcl_fileexists(n)) {
    f = n;
    return true;
  }

  return false;

#else

  if (dcl_fileexists(name)) {
    f = name;
    return true;
  }

  return false;

#endif
}

std::string local_file_name(const std::string & name)
{
#ifdef CREATOR
    return name;
#endif

#ifndef _WIN32
  return homedir() + "/.toppler/" + name;
#else
  return name;
#endif
}

FILE *open_local_config_file(const std::string & name)
{
  checkdir();

  std::string n = local_file_name(name);

  if (dcl_fileexists(n))
    return fopen(n.c_str(), "r+");

  return nullptr;
}

FILE *create_local_config_file(const std::string & name)
{
  checkdir();

  std::string n = local_file_name(name);
  return fopen(n.c_str(), "wb+");
}

/* used for tower and mission saving */

FILE *open_local_data_file(const std::string & name)
{
  checkdir();

  std::string n = local_file_name(name);
  return fopen(n.c_str(), "rb");
}

FILE *create_local_data_file(const std::string & name)
{
  checkdir();

  std::string n = local_file_name(name);
  return fopen(n.c_str(), "wb+");
}

std::vector<std::string> alpha_scandir(const std::string & path, std::function<bool(const std::string & f)> filter)
{
    std::vector<std::string> entries;

    DIR * dir = ::opendir(path.c_str());
    if (dir == NULL)
        // throw std::runtime_error("Can't open directory: " + path);
        return entries;
    for (struct dirent * i = ::readdir(dir); i != NULL; i = ::readdir(dir))
    {
        std::string n = i->d_name;
        if (filter(n))
            entries.push_back(path + "/" + i->d_name);
    }
    ::closedir(dir);
    std::sort(entries.begin(), entries.end());
    return entries;
}

#ifdef _WIN32

static int
utf8_mbtowc (void * conv, wchar_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = s[0];

  if (c < 0x80) {
    *pwc = c;
    return 1;
  } else if (c < 0xc2) {
    return -1;
  } else if (c < 0xe0) {
    if (n < 2)
      return -2;
    if (!((s[1] ^ 0x80) < 0x40))
      return -1;
    *pwc = ((wchar_t) (c & 0x1f) << 6)
           | (wchar_t) (s[1] ^ 0x80);
    return 2;
  } else if (c < 0xf0) {
    if (n < 3)
      return -2;
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (c >= 0xe1 || s[1] >= 0xa0)))
      return -1;
    *pwc = ((wchar_t) (c & 0x0f) << 12)
           | ((wchar_t) (s[1] ^ 0x80) << 6)
           | (wchar_t) (s[2] ^ 0x80);
    return 3;
  } else if (c < 0xf8 && sizeof(wchar_t)*8 >= 32) {
    if (n < 4)
      return -2;
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40
          && (c >= 0xf1 || s[1] >= 0x90)))
      return -1;
    *pwc = ((wchar_t) (c & 0x07) << 18)
           | ((wchar_t) (s[1] ^ 0x80) << 12)
           | ((wchar_t) (s[2] ^ 0x80) << 6)
           | (wchar_t) (s[3] ^ 0x80);
    return 4;
  } else if (c < 0xfc && sizeof(wchar_t)*8 >= 32) {
    if (n < 5)
      return -2;
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
          && (c >= 0xf9 || s[1] >= 0x88)))
      return -1;
    *pwc = ((wchar_t) (c & 0x03) << 24)
           | ((wchar_t) (s[1] ^ 0x80) << 18)
           | ((wchar_t) (s[2] ^ 0x80) << 12)
           | ((wchar_t) (s[3] ^ 0x80) << 6)
           | (wchar_t) (s[4] ^ 0x80);
    return 5;
  } else if (c < 0xfe && sizeof(wchar_t)*8 >= 32) {
    if (n < 6)
      return -2;
    if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
          && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
          && (s[5] ^ 0x80) < 0x40
          && (c >= 0xfd || s[1] >= 0x84)))
      return -1;
    *pwc = ((wchar_t) (c & 0x01) << 30)
           | ((wchar_t) (s[1] ^ 0x80) << 24)
           | ((wchar_t) (s[2] ^ 0x80) << 18)
           | ((wchar_t) (s[3] ^ 0x80) << 12)
           | ((wchar_t) (s[4] ^ 0x80) << 6)
           | (wchar_t) (s[5] ^ 0x80);
    return 6;
  } else
    return -1;
}

size_t mbrtowc (wchar_t * out, const char *s, int n, mbstate_t * st) {
  return utf8_mbtowc(0, out, (const unsigned char *)s, n);
}

#endif

std::string _(const std::string & in) {
#if ENABLE_NLS == 1
    return gettext(in.c_str());
#else
    return in;
#endif
}
std::string N_(const std::string & in) { return in; }

int currentScreenWidth = 640;

