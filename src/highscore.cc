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

#include "highscore.h"
#include "decl.h"
#include "screen.h"

#include <cstdio>
#include <unistd.h>

#ifndef _WIN32
#include <fcntl.h>
#endif

// number of entries per scoring table
#define NUMHISCORES 10

// name of the file containing the scores
#define SCOREFNAME "toppler.hsc"

#pragma GCC diagnostic ignored "-Wunused-result"

// true, if we use the global highscore table, false if not
// if we use the global highscore we will create a lock file
static bool globalHighscore;

// the name of the highscore table we use the name because the
// file might change any time and so it's better to close and reopen
// every time we need access
static std::string highscoreName;

typedef struct {
    // number of points
    Uint32 points;

    // the name of the player, limited to that many characters
    // null terminated
    char name[SCORENAMELEN];

    // the tower reached, right now not really used, just stored
    // -1 = mission finished
    Sint16 tower;
} _scores;

// the currently loaded score table for the mission in
// missionname variable
static _scores scores[NUMHISCORES];

/* this is the name of the currenlty selected mission */
static std::string missionname;

static void savescores(FILE *f)
{
    unsigned char len;
    char mname[256];

    while (!feof(f))
    {
        // try to read mission
        if ((fread(&len, 1, 1, f) == 1) && (fread(mname, 1, len, f) == len))
        {
            // check if the current mission is the current mission in file
            mname[len] = 0;
            if (missionname == mname)
            {
                // this is necessary because some system can not switch
                // on the fly from reading to writing
                fseek(f, ftell(f), SEEK_SET);

                fwrite(scores, sizeof(_scores)*NUMHISCORES, 1, f);
                return;
            }
            else
            {
                // mission names don't match
                // skip this mission
                fseek(f, ftell(f) + sizeof(_scores)*NUMHISCORES, SEEK_SET);
            }
        }
        else
            // could not read, probably end of file
            break;

    }

    // when we get here we are at the end of file and have
    // not found the mission, so add an empty score table
    unsigned char tmp = missionname.size();

    fwrite(&tmp, 1, 1, f);
    fwrite(missionname.c_str(), 1, tmp, f);
    fwrite(scores, sizeof(_scores)*NUMHISCORES, 1, f);
}

static void loadscores(FILE *f)
{
    unsigned char len;
    char mname[256];

    while (f && !feof(f))
    {

        if ((fread(&len, 1, 1, f) == 1) && (fread(mname, 1, len, f) == len) &&
            (fread(scores, 1, sizeof(_scores) * NUMHISCORES, f) == sizeof(_scores) * NUMHISCORES))
        {
            mname[len] = 0;
            if (missionname == mname)
                // mission found and read
                return;
        }
    }

    // mission not found in the highscore, so create an empty one
    for (int t = 0; t < NUMHISCORES; t++) {
        scores[t].points = 0;
        scores[t].name[0] = 0;
    }
}

static bool hsc_lock(void)
{
#ifndef _WIN32

  if (globalHighscore)
  {
    // create lock for highscore table
    int lockfd;

    while ((lockfd = open(HISCOREDIR "/" SCOREFNAME ".lck", O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
      dcl_wait();
      scr_swap();
    }
    close(lockfd);
  }

#endif

  // load the table
  FILE *f = fopen(highscoreName.c_str(), "rb");
  loadscores(f);
  if (f) fclose(f);

  return true;
}

static void hsc_unlock(void)
{
#ifndef _WIN32

  // free the lock
  if (globalHighscore)
  {
      unlink(HISCOREDIR "/" SCOREFNAME ".lck");
  }

#endif
}

void hsc_init(void) {

    for (int t = 0; t < NUMHISCORES; t++)
    {
        scores[t].points = 0;
        scores[t].name[0] = 0;
        scores[t].tower = 0;
    }

#ifndef _WIN32

    /* asume we use local highscore table */
    globalHighscore = false;
    highscoreName = homedir();
    highscoreName += std::string("/.toppler/");
    highscoreName += SCOREFNAME;

    /* now check if we have access to a global highscore table */

#ifdef HISCOREDIR

    /* ok the dir is given, we need to be able to do 2 things: */

    /* 1. get read and write access to the file */

    std::string fname = HISCOREDIR;
    fname += '/';
    fname += SCOREFNAME;

    FILE * f = fopen(fname.c_str(), "r+");

    if (f) {

        fclose(f);

        /* 2. get write access to the directory to create the lock file
         * to check this we try to chreate a file with a random name
         */
        std::string fname2 = fname + std::to_string(rand());
        f = fopen(fname2.c_str(), "w+");
        if (f)
        {
            /* ok, we've got all the rights we need */
            fclose(f);
            unlink(fname2.c_str());
            highscoreName = fname;
            globalHighscore = true;
        }
        else
        {
            debugprintf(2, "could not open create the lock file, no write access to global hiscore directory\n", fname);
        }
    }
    else
    {
        debugprintf(2, "could not open global highscore file %s\n", fname);
    }

#else
    debugprintf(2, "no path for global highscore\n");
#endif

    /* no dir to the global highscore table -> not global highscore table */

#else // ifdef _WIN32

    /* for non unix systems we use only local highscore tables */
    globalHighscore = false;
    highscoreName = SCOREFNAME;

#endif

    if (globalHighscore)
        debugprintf(2, "using global highscore at %s\n", highscoreName.c_str());
    else
        debugprintf(2, "using local highscore at %s\n", highscoreName.c_str());
}

void hsc_select(const std::string & mission)
{
    missionname = mission;
    FILE *f = fopen(highscoreName.c_str(), "rb");
    loadscores(f);
    if (f) fclose(f);
}

Uint8 hsc_entries(void) { return NUMHISCORES; }

void hsc_entry(Uint8 nr, std::string & name, Uint32 & points, Uint8 & tower)
{
    if (nr < NUMHISCORES) {
        name = scores[nr].name;
        points = scores[nr].points;
        tower = scores[nr].tower;
    } else {
        name = "";
        points = 0;
        tower = 0;
    }
}

bool hsc_canEnter(Uint32 points)
{
    return points > scores[NUMHISCORES-1].points;
}

Uint8 hsc_enter(Uint32 points, Uint8 tower, const std::string & name)
{
    if (hsc_lock())
    {
        int t = NUMHISCORES;

        while ((t > 0) && (points > scores[t-1].points))
        {
            if (t < NUMHISCORES)
                scores[t] = scores[t-1];
            t--;
        }

        if (t < NUMHISCORES)
        {
            // fill in the entry
            for (size_t i = 0; i < std::min<size_t>(name.size()+1, SCORENAMELEN-1); i++)
                scores[t].name[i] = name[i];
            scores[t].name[SCORENAMELEN-1] = 0;
            scores[t].points = points;
            scores[t].tower = tower;

            FILE *f;

            if (globalHighscore)
            {
                f = fopen(highscoreName.c_str(), "r+b");
            } else
            {
                /* local highscore: this one might require creating the file */
                fclose(fopen(highscoreName.c_str(), "a+"));
                f = fopen(highscoreName.c_str(), "r+b");
            }

            savescores(f);
            fclose(f);

            hsc_unlock();

            return t;
        }

        hsc_unlock();
    }

    return 0xff;
}

