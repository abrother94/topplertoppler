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

#include "decl.h"
#include "soundsys.h"
#include "archi.h"

ttsounds::ttsounds(void)
{
  useSound = false;
  debugprintf(9, "ttsounds::ttsounds\n");
  title = 0;
}

ttsounds::~ttsounds(void)
{
  closesound();

  for (auto & s : sounds)
      if (s.sound)
          Mix_FreeChunk(s.sound);

  debugprintf(9, "ttsounds::~ttsounds\n");
}

void ttsounds::addsound(const char *fname, int id, int vol, int loops)
{
  auto f = dataarchive->open(fname);

  ttsnddat d;

  d.play = false;
  d.id_num = id;
  d.channel = -1;
  d.volume = vol;
  d.loops = loops;
  d.sound = Mix_LoadWAV_RW(f.rwOps(), 1);

  sounds.push_back(d);

  return;
}

void ttsounds::play(void)
{
  if (!useSound) return;

  for (auto & s : sounds)
      if (s.play)
      {
          s.channel = Mix_PlayChannel(-1, s.sound, s.loops);
          Mix_Volume(s.channel, s.volume);
          s.play = false;
      }
  debugprintf(9,"ttsounds::play()\n");
}

void ttsounds::stop(void)
{
  for (size_t t = 0; t < sounds.size(); t++) stopsound(t);
}

void ttsounds::stopsound(size_t snd)
{
  if (useSound) {
    if (snd < sounds.size()) {
      if (sounds[snd].channel != -1)
      {
        Mix_HaltChannel(sounds[snd].channel);
        sounds[snd].channel = -1;
      }
      sounds[snd].play = false;
    }
  }
  debugprintf(9,"ttsounds::stopsound(%i)\n", snd);
}

void ttsounds::startsound(size_t snd)
{
  if (!useSound) return;

  if (snd < sounds.size()) sounds[snd].play = true;

  debugprintf(9,"ttsounds::startsound(%i)\n", snd);
}

void ttsounds::setsoundvol(size_t snd, int vol)
{
  if (useSound) {
    if (snd < sounds.size()) {
      if (sounds[snd].channel != -1) {

        Mix_Volume(sounds[snd].channel, vol);

      }
      sounds[snd].volume = vol;
    }

    debugprintf(9,"ttsounds::setsoundvol(%i, %i)\n", snd, vol);
  }
}

ttsounds * ttsounds::instance(void)
{
    if (!inst)
        inst = new ttsounds();

    return inst;
}

class ttsounds *ttsounds::inst = 0;

void ttsounds::opensound(void)
{
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        debugprintf(0, "Couldn't init the sound system, muting.\n");
        return;
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        debugprintf(0, "Could not open audio, muting.\n");
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    useSound = true;
}

void ttsounds::closesound(void)
{
    if (!useSound) return;

    while (Mix_Playing(-1)) dcl_wait();

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    useSound = false;
}

void ttsounds::playmusic(const std::string & fname)
{
    if (!useSound) return;

    std::string f;

    if (get_data_file_path(fname, f)) {
        title = Mix_LoadMUS(f.c_str());
        Mix_PlayMusic(title, -1);
        musicVolume = MIX_MAX_VOLUME;
    }
}

void ttsounds::stopmusic(void)
{
    if (!useSound) return;

    if (title)
    {
        Mix_FadeOutMusic(1000);

        while (Mix_FadingMusic() != MIX_NO_FADING)
            dcl_wait();
    }
}

void ttsounds::fadeToVol(int vol)
{
    if (!title) return;

    while (musicVolume != vol)
    {
        if (musicVolume > vol) {
            musicVolume -= 4;
            if (musicVolume < vol)
                musicVolume = vol;
        } else {
            musicVolume += 4;
            if (musicVolume > vol)
                musicVolume = vol;
        }

        Mix_VolumeMusic(musicVolume);
        dcl_wait();
    }
}

