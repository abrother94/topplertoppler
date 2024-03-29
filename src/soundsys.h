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

#ifndef SOUNDSYS_H
#define SOUNDSYS_H

#include <SDL_mixer.h>

#include <vector>
#include <string>

struct ttsnddat {
  bool play;   //is this block goind to get played next time?
  int id_num;  //unique ID # of this sound
  int channel; //sound channel
  int volume;  //sound volume
  int loops;   //how many times to loop this sound?
  Mix_Chunk *sound; //sound data
};

class ttsounds {
public:
  ~ttsounds(void);

  void addsound(const char *fname, int id, int vol, int loops);

  void play(void); //play all active sounds
  void stop(void); //stop all sounds

  void stopsound(size_t snd); //stop the sound from playing
  void startsound(size_t snd); //the sound will play in the next update
  void setsoundvol(size_t snd, int vol); //set sound volume

  void playmusic(const std::string & file); //start playing a background music
  void stopmusic(void);            // stop playing the background music
  void fadeToVol(int vol);

  /* tries to open and initialize the sound device */
  void opensound(void);
  /* closes the sound device */
  void closesound(void);

  /* singleton function, use this function to access the one and only
   * instance of this class
   */
  static ttsounds * instance(void);

private:
  ttsounds(void);

  /* this var is only true, if we the user wants sound, and wa
   * can init it
   */
  bool useSound;

  std::vector<ttsnddat> sounds;

  Mix_Music * title;
  int musicVolume;

  static class ttsounds *inst;

};

#endif
