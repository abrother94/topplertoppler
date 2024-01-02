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

#ifndef CREATOR

#include "level.h"
#include "points.h"
#include "archi.h"
#include "configuration.h"
#include "screen.h"

#endif

#include "decl.h"

#ifdef _WIN32
#include <direct.h>
#endif

#define TOWERWID 16

/* tower block flags */
#define TBF_NONE     0x0000
#define TBF_EMPTY    0x0001 /* block is not solid */
#define TBF_PLATFORM 0x0002 /* block is a platform */
#define TBF_STATION  0x0004 /* block is a lift station */
#define TBF_DEADLY   0x0008 /* block is deadly */
#define TBF_ROBOT    0x0010 /* block is a robot */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

static const struct _tblockdata {
    const char *nam; /* name */
    char       ch;   /* representation in saved tower file */
    Uint16     tf = 0;   /* flags; TBF_foo */
} towerblockdata[NUM_TBLOCKS] = {
    { "space",            ' ', TBF_EMPTY },
    { "lift top stop",    'v', TBF_EMPTY|TBF_STATION },
    { "lift middle stop", '+', TBF_EMPTY|TBF_STATION },
    { "lift bottom stop",  0,  TBF_EMPTY|TBF_STATION },
    { "robot 1",          '1', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 2",          '2', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 3",          '3', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 4",          '4', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 5",          '5', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 6",          '6', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "robot 7",          '7', TBF_EMPTY|TBF_DEADLY|TBF_ROBOT },
    { "stick",            '!', /*TBF_PLATFORM*/ },
    { "step",             '-', TBF_PLATFORM },
    { "vanisher step",    '.', TBF_PLATFORM },
    { "slider > step",    '>', TBF_PLATFORM },
    { "slider < step",    '<', TBF_PLATFORM },
    { "box",              'b', TBF_NONE },
    { "door",             '#', TBF_EMPTY },
    { "target door",      'T', TBF_EMPTY },
    { "stick top",         0,  TBF_STATION/*|TBF_PLATFORM*/ },
    { "stick middle",      0,  TBF_STATION/*|TBF_PLATFORM*/ },
    { "stick bottom",      0,  TBF_STATION/*|TBF_PLATFORM*/ },
    { "lift top",          0,  TBF_STATION|TBF_PLATFORM },
    { "lift middle",       0,  TBF_STATION|TBF_PLATFORM },
    { "lift bottom",      '^', TBF_STATION|TBF_PLATFORM },
    { "stick at door",     0,  },
    { "stick at target",   0,  },
    { "lift at door",      0 , TBF_STATION|TBF_PLATFORM },
    { "lift at target",    0 , TBF_STATION|TBF_PLATFORM },
};

/* Sections in the data files; do not change the order,
 * and always add new ones to the end, so that we keep
 * compatibility with old towers/missions.
 * (the loader silently ignores unrecognized sections)
 */
enum towersection {
    TSS_END,
    TSS_TOWERNAME,
    TSS_TOWERTIME,
    TSS_TOWERCOLOR,
    TSS_TOWERDATA,
    TSS_DEMO,
    TSS_ROBOT
};

static const char tss_string_name[] = "name";
static const char tss_string_time[] = "time";
static const char tss_string_color[] = "color";
static const char tss_string_data[] = "data";
static const char tss_string_demo[] = "demo";
static const char tss_string_robot[] = "robot";

static std::vector<Uint8> mission;
static Uint8 towerheight;
static Uint8 towerrobot;
// TODO this would be nicer as a vector of arrays or something like this
// as right now we are limited to a hight of 256 layers
static Uint8 tower[256][TOWERWID];
static std::string towername;
static Uint8 towernumber;
static Uint8 towercolor_red, towercolor_green, towercolor_blue;
static Uint16 towertime;
static std::vector<Uint16> towerdemo;

typedef struct mission_node {
  std::string name;
  std::string fname;
  bool archive;       // is the mission inside the archive, or not
  Uint8 prio;         // the lower prio, the further in front the mission will be in the list
} mission_node;

std::vector<mission_node> missions;

#ifndef CREATOR

static bool missionfiles (const std::string & file)
{
  if (file.size() < 4) return false;
  return file.substr(file.size()-4) == ".ttm";
}

#endif

Uint8 conv_char2towercode(char ch) {
  if (ch)
    for (int x = 0; x < NUM_TBLOCKS; x++)
      // we can do that because we use only chars below 128
      if (ch == towerblockdata[x].ch) return x;
  return TB_EMPTY;
}

char conv_towercode2char(Uint8 code) {
  if ((code < NUM_TBLOCKS) && (towerblockdata[code].ch))
      return towerblockdata[code].ch;
  return towerblockdata[TB_EMPTY].ch;
}


#ifndef CREATOR

static void add_mission(std::string fname, bool archive = false) {

  char mname[30];
  Uint8 prio;

  if (archive) {

    auto f = dataarchive->open(fname);

    unsigned char mnamelength;
    f.read(&mnamelength, 1);

    if (mnamelength > 29) mnamelength = 29;

    f.read(mname, mnamelength);
    mname[mnamelength] = 0;
    f.read(&prio, 1);

  } else {

    FILE * f = fopen(fname.c_str(), "rb");

    if (!f) return;

    unsigned char mnamelength;
    size_t read = fread(&mnamelength, 1, 1, f);
    assert_msg(read == 1, "could not read data");

    if (mnamelength > 29) mnamelength = 29;

    read = fread(mname, 1, mnamelength, f);
    assert_msg(read == mnamelength, "could not read data");
    mname[mnamelength] = 0;
    read = fread(&prio, 1, 1, f);
    assert_msg(read == 1, "could not read data");
    fclose(f);
  }

  /* first check if the mission is already there */
  /* no two missions with the same name */
  for (auto & m : missions)
      if (m.name == mname)
          return;

  auto i = std::find_if(missions.begin(), missions.end(), [prio](auto & m) { return m.prio > prio; });

  mission_node n;
  n.name = mname;
  n.fname = fname;
  n.prio = prio;
  n.archive = archive;

  missions.insert(i, n);
}

void lev_findmissions() {

  missions.clear();

  /* first check inside the archive */

  for (auto f : dataarchive->filelist())
      if (missionfiles(f.name))
          add_mission(f.name, true);

#ifdef _WIN32
  std::string pathname;
  {
    char n[100];
    _getcwd(n, 100);
    pathname = n;
    pathname = pathname + "\\";
  }

#else
  std::string pathname = "./";
#endif

  for (auto & e : alpha_scandir(pathname, missionfiles))
      add_mission(e);

#ifndef _WIN32

  for (auto & e : alpha_scandir(homedir() + "/.toppler/", missionfiles))
      add_mission(e);

  for (auto & e : alpha_scandir(std::string(TOP_DATADIR) + "/", missionfiles))
      add_mission(e);

#endif
}

#endif

void lev_done() {
}


Uint16 lev_missionnumber() {
  return missions.size();
}

const std::string & lev_missionname(Uint16 num) {
  return missions[num].name;
}

bool lev_loadmission(Uint16 num) {

  auto & m = missions[num];

  if (m.archive) {

    auto f = dataarchive->open(m.fname);
    Uint32 fsize = f.size();

    mission.resize(fsize);
    f.read(mission.data(), fsize);

  } else {

    FILE * in = fopen(m.fname.c_str(), "rb");

    /* find out file size */
    fseek(in, 0, SEEK_END);
    unsigned long fsize = ftell(in);

    /* get enough memory and load the whole file into memory */
    mission.resize(fsize);
    fseek(in, 0, SEEK_SET);
    size_t read = fread(mission.data(), 1, fsize, in);
    assert_msg(read == fsize, "could not read data");

    fclose(in);
  }

  for (int t = 0; t < lev_towercount(); t++) {
    lev_selecttower(t);
    for (int r = 0; r < towerheight; r++)
      for (int c = 0; c < TOWERWID; c++)
        if (tower[r][c] >= NUM_TBLOCKS)
          return false;
  }

  return true;
}

Uint8 lev_towercount(void) {
  return mission[mission[0] + 2];
}

void lev_selecttower(Uint8 number) {

  Uint32 towerstart;

  towernumber = number;
  towerrobot = number;
  Uint8 section;
  Uint32 section_len;

  lev_clear_towerdemo();

  // find start of towerdata in mission
  {
    Uint32 idxpos = 0;

    idxpos += mission[mission[0] + 3];
    idxpos += long(mission[mission[0] + 4]) << 8;
    idxpos += long(mission[mission[0] + 5]) << 16;
    idxpos += long(mission[mission[0] + 6]) << 24;

    towerstart = mission[idxpos + 4 * number];
    towerstart += long(mission[idxpos + 4 * number + 1]) << 8;
    towerstart += long(mission[idxpos + 4 * number + 2]) << 16;
    towerstart += long(mission[idxpos + 4 * number + 3]) << 24;
  }

  do {
    section = mission[towerstart++];
    section_len = mission[towerstart++];
    section_len += Uint32(mission[towerstart++]) << 8;
    section_len += Uint32(mission[towerstart++]) << 16;
    section_len += Uint32(mission[towerstart++]) << 24;
    switch ((towersection)section) {
    case TSS_TOWERNAME:
      towername.resize(section_len);
      memmove(towername.data(), &mission[towerstart], section_len);
      break;
    case TSS_TOWERTIME:
      towertime = mission[towerstart] + (int(mission[towerstart + 1]) << 8);
      break;
    case TSS_TOWERCOLOR:
      towercolor_red = mission[towerstart];
      towercolor_green = mission[towerstart + 1];
      towercolor_blue = mission[towerstart + 2];
      break;
    case TSS_TOWERDATA:
      {
        towerheight = mission[towerstart];

        Uint32 bitstart = towerstart + 1;
        Uint32 bytestart = bitstart + 2 * towerheight;
        Uint16 wpos = 0;
        Uint16 bpos = 0;

        lev_clear_tower();

        for (Uint8 row = 0; row < towerheight; row++) {
          for (Uint8 col = 0; col < TOWERWID; col++) {
            if ((mission[bitstart + (bpos >> 3)] << (bpos & 7)) & 0x80)
              tower[row][col] = mission[bytestart + wpos++];
            else
              tower[row][col] = 0;
            bpos++;
          }
        }
        break;
      }
    case TSS_DEMO:
      {
        // get tower demo
        std::vector<Uint16> demo;
        Uint16 tmpbuf_len = mission[towerstart];
        tmpbuf_len += Uint16(mission[towerstart+1]) << 8;
        Uint16 ofs = 2;

        if (tmpbuf_len) {
          Uint16 idx = 0;
          while (idx < tmpbuf_len)
          {
            Uint8 run = mission[towerstart + ofs++];
            Uint16 data = mission[towerstart + ofs++];
            data += Uint16(mission[towerstart + ofs++]) << 8;

            while (run) {
              demo.push_back(data);
              idx++;
              run--;
            }
          }
        }

        lev_set_towerdemo(std::move(demo));
        break;
      }
    case TSS_ROBOT:
      towerrobot = mission[towerstart];
#ifndef CREATOR
      towerrobot %= scr_numrobots();
#endif
      break;
    case TSS_END:
    default:      break;
    }
    towerstart += section_len;
  } while ((towersection)section != TSS_END);
}

// TODO make buf a vector
static std::string gen_passwd(int pwlen, const std::string & allowed, int buflen, char *buf)
{
  int alen = allowed.size();

  if (pwlen > PASSWORD_LEN) pwlen = PASSWORD_LEN;

  int len = buflen;
  if (len < (pwlen*5)) len = pwlen*5;

  char passwd[PASSWORD_LEN + 1];
  memset(passwd, 0, PASSWORD_LEN+1);

  for (int i = 0; i < len; i++)
  {
    passwd[i % pwlen] += buf[i % buflen];
    if (passwd[i % pwlen] > alen) passwd[(i+1) % pwlen]++;
  }

  for (int i = 0; i < pwlen; i++)
    passwd[i] = allowed[abs(passwd[i]) % alen];

  return passwd;
}

std::string lev_get_passwd(void) {
    return gen_passwd(PASSWORD_LEN, PASSWORD_CHARS, 256*TOWERWID, (char *)tower);
}

bool lev_show_passwd(int levnum) {
  return ((levnum > 0) &&
          (levnum < lev_towercount()) &&
          ((levnum % 3) == 0));
}

int lev_tower_passwd_entry(const std::string & passwd)
{
    for (int i = 0; i < lev_towercount(); i++)
    {
        lev_selecttower(i);
        if (passwd == lev_get_passwd()) return i;
    }
    return 0;
}

void lev_clear_tower(void)
{
    memset(&tower, TB_EMPTY, 256*TOWERWID);
}

void lev_set_towercol(Uint8 r, Uint8 g, Uint8 b) {
  towercolor_red = r;
  towercolor_green = g;
  towercolor_blue = b;
}

Uint8 lev_towercol_red() {
  return towercolor_red;
}

Uint8 lev_towercol_green() {
  return towercolor_green;
}

Uint8 lev_towercol_blue() {
  return towercolor_blue;
}

Uint8 lev_tower(Uint16 row, Uint8 column) {
  return tower[row][column];
}

Uint8 lev_set_tower(Uint16 row, Uint8 column, Uint8 block) {
    Uint8 tmp = tower[row][column];
    tower[row][column] = block;
    return tmp;
}

Uint8 lev_towerrows(void) {
  return towerheight;
}

const std::string & lev_towername(void) {
  return towername;
}

// TODO demobuffer should be vector
void lev_set_towerdemo(std::vector<Uint16> && demo)
{
    towerdemo = std::move(demo);
}

void lev_clear_towerdemo()
{
    towerdemo.clear();
}

// TODO use vector
const std::vector<Uint16> & lev_get_towerdemo() {
    return towerdemo;
}

void lev_set_towername(const std::string & str)
{
    towername = str;
}

Uint8 lev_towernr(void) {
  return towernumber;
}

bool lev_lasttower(void) {
  return (towernumber+1) == lev_towercount();
}

Uint8 lev_robotnr(void) {
  return towerrobot;
}

void lev_set_robotnr(Uint8 robot) {
  towerrobot = robot;
}

Uint16 lev_towertime(void) {
  return towertime;
}

void lev_set_towertime(Uint16 time) {
  towertime = time;
}

void lev_removelayer(Uint8 layer) {
  while (layer < towerheight) {
    for (Uint8 c = 0; c < TOWERWID; c++)
      tower[layer][c] = tower[layer + 1][c];
    layer++;
  }

  towerheight--;
}

/* empties a cell in the tower */
void lev_clear(int row, int col) {
  tower[row][col] = TB_EMPTY;
}

/* if the given position contains a vanishing step, remove it */
void lev_removevanishstep(int row, int col) {
  if (tower[row][col] == TB_STEP_VANISHER)
    tower[row][col] = TB_EMPTY;
}

/********** everything for doors ********/

/* returns true, if the given position is the upper end of a door
 (a door is always 3 layers) */
bool lev_is_door_upperend(int row, int col) {
  return lev_is_door(row, col) &&
    lev_is_door(row + 1, col) &&
    lev_is_door(row + 2, col);
}

/* returns true if the given position contains a door */
bool lev_is_door(int row, int col) {
  return (tower[row][col] == TB_DOOR ||
          tower[row][col] == TB_DOOR_TARGET ||
          tower[row][col] == TB_STICK_DOOR);
}

/* returns true, if the given fiels contains a target door */
bool lev_is_targetdoor(int row, int col) {
  return tower[row][col] == TB_DOOR_TARGET;
}

/**************** everything for elevators ******************/

bool lev_is_station(int row, int col) {
  return ((towerblockdata[tower[row][col]].tf & TBF_STATION) != 0);
}
bool lev_is_up_station(int row, int col) {
  return ((tower[row][col] == TB_ELEV_BOTTOM) ||
          (tower[row][col] == TB_ELEV_MIDDLE));
}
bool lev_is_down_station(int row, int col) {
  return ((tower[row][col] == TB_ELEV_TOP) ||
          (tower[row][col] == TB_ELEV_MIDDLE));
}
bool lev_is_bottom_station(int row, int col) {
   return (tower[row][col] == TB_ELEV_BOTTOM);
}

bool lev_is_platform(int row, int col) {
  return ((towerblockdata[tower[row][col]].tf & TBF_PLATFORM) != 0);
}
bool lev_is_stick(int row, int col) {
  return ((tower[row][col] == TB_STICK) ||
          (tower[row][col] == TB_STICK_TOP) ||
          (tower[row][col] == TB_STICK_MIDDLE) ||
          (tower[row][col] == TB_STICK_BOTTOM) ||
          (tower[row][col] == TB_STICK_DOOR) ||
          (tower[row][col] == TB_STICK_DOOR_TARGET));
}

bool lev_is_elevator(int row, int col) {
  return ((tower[row][col] == TB_STICK_BOTTOM) ||
          (tower[row][col] == TB_STICK_MIDDLE) ||
          (tower[row][col] == TB_STICK_TOP) ||
          (tower[row][col] == TB_ELEV_BOTTOM) ||
          (tower[row][col] == TB_ELEV_MIDDLE) ||
          (tower[row][col] == TB_ELEV_TOP));
}

void lev_platform2stick(int row, int col) {
  if (tower[row][col] == TB_ELEV_TOP) tower[row][col] = TB_STICK_TOP;
  else if (tower[row][col] == TB_ELEV_MIDDLE) tower[row][col] = TB_STICK_MIDDLE;
  else if (tower[row][col] == TB_ELEV_BOTTOM) tower[row][col] = TB_STICK_BOTTOM;
  else if (tower[row][col] == TB_STEP) tower[row][col] = TB_STICK;
}
void lev_stick2platform(int row, int col) {
  if (tower[row][col] == TB_STICK_TOP) tower[row][col] = TB_ELEV_TOP;
  else if (tower[row][col] == TB_STICK_MIDDLE) tower[row][col] = TB_ELEV_MIDDLE;
  else if (tower[row][col] == TB_STICK_BOTTOM) tower[row][col] = TB_ELEV_BOTTOM;
  else if (tower[row][col] == TB_STICK_DOOR) tower[row][col] = TB_ELEV_DOOR;
  else if (tower[row][col] == TB_STICK_DOOR_TARGET) tower[row][col] = TB_ELEV_DOOR_TARGET;
  else if (tower[row][col] == TB_STICK) tower[row][col] = TB_STEP;
}
void lev_stick2empty(int row, int col) {
  if (tower[row][col] == TB_STICK_TOP) tower[row][col] = TB_STATION_TOP;
  else if (tower[row][col] == TB_STICK_MIDDLE) tower[row][col] = TB_STATION_MIDDLE;
  else if (tower[row][col] == TB_STICK_BOTTOM) tower[row][col] = TB_STATION_BOTTOM;
  else if (tower[row][col] == TB_STICK_DOOR_TARGET) tower[row][col] = TB_DOOR_TARGET;
  else if (tower[row][col] == TB_STICK_DOOR) tower[row][col] = TB_DOOR;
  else if (tower[row][col] == TB_STICK) tower[row][col] = TB_EMPTY;
}
void lev_empty2stick(int row, int col) {
  if (tower[row][col] == TB_STATION_TOP) tower[row][col] = TB_STICK_TOP;
  else if (tower[row][col] == TB_STATION_MIDDLE) tower[row][col] = TB_STICK_MIDDLE;
  else if (tower[row][col] == TB_STATION_BOTTOM) tower[row][col] = TB_STICK_BOTTOM;
  else if (tower[row][col] == TB_DOOR) tower[row][col] = TB_STICK_DOOR;
  else if (tower[row][col] == TB_DOOR_TARGET) tower[row][col] = TB_STICK_DOOR_TARGET;
  else if (tower[row][col] == TB_EMPTY) tower[row][col] = TB_STICK;
}
void lev_platform2empty(int row, int col) {
  if (tower[row][col] == TB_ELEV_TOP) tower[row][col] = TB_STATION_TOP;
  else if (tower[row][col] == TB_ELEV_MIDDLE) tower[row][col] = TB_STATION_MIDDLE;
  else if (tower[row][col] == TB_ELEV_BOTTOM) tower[row][col] = TB_STATION_BOTTOM;
  else if (tower[row][col] == TB_ELEV_DOOR_TARGET) tower[row][col] = TB_DOOR_TARGET;
  else if (tower[row][col] == TB_ELEV_DOOR) tower[row][col] = TB_DOOR;
  else if (tower[row][col] == TB_STEP) tower[row][col] = TB_EMPTY;
}

/* misc questions */
bool lev_is_empty(int row, int col) {
  return ((towerblockdata[tower[row][col]].tf & TBF_EMPTY));
}

bool lev_is_box(int row, int col) {
  return tower[row][col] == TB_BOX;
}

int lev_is_sliding(int row, int col) {
    return ((tower[row][col] == TB_STEP_LSLIDER) ? 1 :
            (tower[row][col] == TB_STEP_RSLIDER) ? -1 :
            0);
}

bool lev_is_robot(int row, int col) {
  return ((towerblockdata[tower[row][col]].tf & TBF_ROBOT) != 0);
}

#ifndef CREATOR

static bool inside_cyclic_intervall(int x, int start, int end, int cycle) {

  while (x < start) x += cycle;
  while (x >= end) x -= cycle;

  return (x >= start) && (x < end);
}


/* returns true, if the given figure can be at the given position
 without colliding with fixed objects of the tower */
bool lev_testfigure(long angle, long vert, long back,
                    long fore, long typ, long height, long width) {
  long hinten, vorn, y, x = 0, k, t;

  hinten = ((angle + back) >> 3) & 0xf;
  vorn = (((angle + fore) >> 3) + 1) & 0xf;

  y = vert / 4;
  vert &= 3;

  switch (typ) {

    case 0:  /* toppler */
      x = (vert == 3) ? 3 : 2;
      break;

    case 1:  /* robot */
      x = (vert == 0) ? 1 : 2;
      break;

    case 2:  /* snowball */
      x = (vert == 0) ? 0 : 1;
      break;
  }

  do {
    k = x;
    do {
      if (lev_is_platform(k + y, hinten)) {
        return false;
      } else if (lev_is_stick(k + y, hinten)) {
        t = hinten * 8 + height;
        if (inside_cyclic_intervall(angle, t, t+width, 0x80))
          return false;
      } else if (lev_is_box(k + y, hinten)) {
        t = hinten * 8 + height;
        if (inside_cyclic_intervall(angle, t, t+width, 0x80)) {
          if (typ == 2) {
            // the snowball removes the box
            lev_clear(k + y, hinten);
            pts_add(50);
          }
          return false;
        }
      }
      k--;
    } while (k != -1);
    hinten = (hinten + 1) & 0xf;
  } while (hinten != vorn);

  return true;
}

#endif

unsigned char lev_putplatform(int row, int col) {
  unsigned char erg = tower[row][col];

  tower[row][col] = TB_ELEV_BOTTOM;

  return erg;
}

void lev_restore(int row, int col, unsigned char bg) {
  tower[row][col] = bg;
}


/* load and save a tower */
bool lev_loadtower(const std::string & fname) {
  FILE *in = open_local_data_file(fname);
  char line[200];

  if (in == NULL) return false;

  lev_clear_tower();
  lev_clear_towerdemo();
  towertime = 0;
  towerheight = 0;
  towerrobot = 0;

  while (true) {

    if (feof(in)) break;

    char * read;

    /* look for next section start */
    read = fgets(line, 200, in);

    if (read == nullptr && feof(in)) break;

    assert_msg(read != nullptr, "could not read data");
    if (line[0] != '[')
      continue;

    if (strncmp(&line[1], tss_string_name, strlen(tss_string_name)) == 0) {
      char tmp[TOWERNAMELEN+1];
      towername.clear();

      read = fgets(tmp, TOWERNAMELEN+1, in);
      assert_msg(read != nullptr, "could not read data");
      /* remove not allowed characters */
      {
        int inp = 0;
        while(tmp[inp])
        {
          if ((tmp[inp] >= 32) && (tmp[inp] < 127))
            towername += tmp[inp];
          inp++;
        }
      }
    } else if (strncmp(&line[1], tss_string_color, strlen(tss_string_color)) == 0) {
      read = fgets(line, 200, in);
      assert_msg(read != nullptr, "could not read data");
      sscanf(line, "%hhu, %hhu, %hhu\n", &towercolor_red, &towercolor_green, &towercolor_blue);
    } else if (strncmp(&line[1], tss_string_time, strlen(tss_string_time)) == 0) {
      read = fgets(line, 200, in);
      assert_msg(read != nullptr, "could not read data");
      sscanf(line, "%hu\n", &towertime);
    } else if (strncmp(&line[1], tss_string_data, strlen(tss_string_data)) == 0) {

      read = fgets(line, 200, in);
      assert_msg(read != nullptr, "could not read data");
      sscanf(line, "%hhu\n", &towerheight);

      for (int row = towerheight - 1; row >= 0; row--) {

        read = fgets(line, 200, in);
        assert_msg(read != nullptr, "could not read data");

        for (int col = 0; col < TOWERWID; col++)
          tower[row][col] = conv_char2towercode(line[col]);
      }
    } else if (strncmp(&line[1], tss_string_demo, strlen(tss_string_demo)) == 0) {
      if (fgets(line, 200, in)) {
          int len;
          sscanf(line, "%i\n", &len);

          if (len > 0) {
              towerdemo.resize(len);

              for (int idx = 0; idx < len; idx++) {
                  read = fgets(line, 200, in);
                  assert_msg(read != nullptr, "could not read data");
                  Uint16 tmp;
                  sscanf(line, "%hu\n", &tmp);
                  towerdemo[idx] = tmp;
              }
          } else towerdemo.clear();
      }
    } else if (strncmp(&line[1], tss_string_robot, strlen(tss_string_robot)) == 0) {
      read = fgets(line, 200, in);
      assert_msg(read != nullptr, "could not read data");
      {
        int i;
        sscanf(line, "%u\n", &i);
#ifdef CREATOR
        towerrobot = i & 0xFF;
#else
        towerrobot = (i & 0xFF) % scr_numrobots();
#endif
      }
    }
  }

  fclose(in);
  return true;
}

#ifndef CREATOR

bool lev_savetower(const std::string & fname) {
  FILE *out = create_local_data_file(fname);

  if (out == NULL) return false;

  fprintf(out, "[%s]\n", tss_string_name);
  fprintf(out, "%s\n", towername.c_str());

  fprintf(out, "[%s]\n", tss_string_color);
  fprintf(out, "%hhu, %hhu, %hhu\n", towercolor_red, towercolor_green, towercolor_blue);

  fprintf(out, "[%s]\n", tss_string_time);
  fprintf(out, "%hu\n", towertime);

  fprintf(out, "[%s]\n", tss_string_robot);
  fprintf(out, "%hhu\n", towerrobot);

  fprintf(out, "[%s]\n", tss_string_data);
  fprintf(out, "%hhu\n", towerheight);
  for (int row = towerheight - 1; row >= 0; row--) {
    char line[TOWERWID+2];

    for (int col = 0; col < TOWERWID; col++)
      line[col] = conv_towercode2char(tower[row][col]);

    line[TOWERWID] = '|';
    line[TOWERWID+1] = 0;
    fprintf(out, "%s\n", line);
  }

  fprintf(out, "[%s]\n", tss_string_demo);
  fprintf(out, "%zu\n", towerdemo.size());
  for (auto d : towerdemo)
      fprintf(out, "%hu\n", d);

  fclose(out);

  return true;
}

#endif

/* insert and delete one row */
void lev_insertrow(int position) {
  if ((towerheight < 255) && (position < towerheight)) {
    int k = towerheight - 1;
    while (k >= position) {
      for (int i = 0; i < TOWERWID; i++)
        tower[k + 1][i] = tower[k][i];
      k--;
    }
    for (int i = 0; i < TOWERWID; i++)
      tower[position][i] = 0;
    towerheight++;
    return;
  }
  if (towerheight == 0) {
    for (int i = 0; i < TOWERWID; i++)
      tower[0][i] = 0;
    towerheight = 1;
  }
}

void lev_deleterow(int position) {
  if ((position < towerheight) && (position >= 0)) {
    int k = position + 1;
    while (k < towerheight) {
      for (int i = 0; i < TOWERWID; i++)
        tower[k - 1][i] = tower[k][i];
      k++;
    }
    towerheight--;
  }
}

void lev_new(Uint8 hei) {
  towerheight = hei;
  lev_clear_tower();
}

void lev_putspace(int row, int col) {

  // always delete the whole door
  if (lev_is_door(row, col)) {
    int r = row - 1;
    while (lev_is_door(r, col)) {
      tower[r][col] = TB_EMPTY;
      r--;
    }
    r = row + 1;
    while (lev_is_door(r, col)) {
      tower[r][col] = TB_EMPTY;
      r++;
    }
  }
  tower[row][col] = TB_EMPTY;
}
void lev_putrobot1(int row, int col) { tower[row][col] = TB_ROBOT1; }
void lev_putrobot2(int row, int col) { tower[row][col] = TB_ROBOT2; }
void lev_putrobot3(int row, int col) { tower[row][col] = TB_ROBOT3; }
void lev_putrobot4(int row, int col) { tower[row][col] = TB_ROBOT4; }
void lev_putrobot5(int row, int col) { tower[row][col] = TB_ROBOT5; }
void lev_putrobot6(int row, int col) { tower[row][col] = TB_ROBOT6; }
void lev_putrobot7(int row, int col) { tower[row][col] = TB_ROBOT7; }
void lev_putstep(int row, int col) { tower[row][col] = TB_STEP; }
void lev_putvanishingstep(int row, int col) { tower[row][col] = TB_STEP_VANISHER; }
void lev_putslidingstep_left(int row, int col) { tower[row][col] = TB_STEP_LSLIDER; }
void lev_putslidingstep_right(int row, int col) { tower[row][col] = TB_STEP_RSLIDER; }

void lev_putdoor(int row, int col) {

  if (row + 2 < towerheight) {

    tower[row][col] = TB_DOOR;
    tower[row + 1][col] = TB_DOOR;
    tower[row + 2][col] = TB_DOOR;

    if ((tower[row][(col + (TOWERWID/2)) % TOWERWID] == 0) &&
        (tower[row + 1][(col + (TOWERWID/2)) % TOWERWID] == 0) &&
        (tower[row + 2][(col + (TOWERWID/2)) % TOWERWID] == 0)) {
      tower[row][(col + (TOWERWID/2)) % TOWERWID] = TB_DOOR;
      tower[row + 1][(col + (TOWERWID/2)) % TOWERWID] = TB_DOOR;
      tower[row + 2][(col + (TOWERWID/2)) % TOWERWID] = TB_DOOR;
    }
  }
}

void lev_puttarget(int row, int col) {
  if (row + 2 < towerheight) {
    tower[row][col] = TB_DOOR_TARGET;
    tower[row + 1][col] = TB_DOOR_TARGET;
    tower[row + 2][col] = TB_DOOR_TARGET;
  }
}

void lev_putstick(int row, int col) { tower[row][col] = TB_STICK; }
void lev_putbox(int row, int col) { tower[row][col] = TB_BOX; }
void lev_putelevator(int row, int col) { tower[row][col] = TB_ELEV_BOTTOM; }
void lev_putmiddlestation(int row, int col) { tower[row][col] = TB_STATION_MIDDLE; }
void lev_puttopstation(int row, int col) { tower[row][col] = TB_STATION_TOP; }


std::vector<Uint8> lev_save()
{
  std::vector<Uint8> data;
  data.resize(256*TOWERWID+1);

  data[0] = towerheight;
  memmove(data.data()+1, tower, 256 * TOWERWID);

  return data;
}

void lev_restore(const std::vector<Uint8> & data)
{
  memmove(tower, data.data()+1, 256 * TOWERWID);
  towerheight = data[0];
}

lev_problem lev_is_consistent(int &row, int &col) {

  int y;
  bool has_exit = false;
  // check first, if the starting point is correctly organized
  // so that there is no obstacle and we can survive there
  if ((tower[1][0] != TB_STICK) && (tower[1][0] != TB_STEP) &&
       (tower[1][0] != TB_STEP_LSLIDER) && (tower[1][0] != TB_STEP_RSLIDER) &&
       (tower[1][0] != TB_BOX) &&
       (tower[1][0] != TB_ELEV_BOTTOM) &&
       (tower[0][0] != TB_STICK) && (tower[0][0] != TB_STEP) &&
       (tower[0][0] != TB_STEP_LSLIDER) && (tower[0][0] != TB_STEP_RSLIDER) &&
       (tower[0][0] != TB_BOX) &&
       (tower[0][0] != TB_ELEV_BOTTOM)) {
    row = 1;
    col = 0;
    return TPROB_NOSTARTSTEP;
  }
  for (y = 2; y < 5; y++)
    if ((towerblockdata[tower[y][0]].tf & TBF_DEADLY) ||
        !(towerblockdata[tower[y][0]].tf & TBF_EMPTY)) {
      row = y;
      col = 0;
      return TPROB_STARTBLOCKED;
    }

  if (towerheight < 4) return TPROB_SHORTTOWER;

  for (int r = 0; r < towerheight; r++)
    for (int c = 0; c < TOWERWID; c++) {
      // check for undefined symbols
      if (tower[r][c] >= NUM_TBLOCKS) {
          row = r;
          col = c;
          return TPROB_UNDEFBLOCK;
      }

      // check if elevators always have an opposing end without unremovable
      // obstacles
      if (tower[r][c] == TB_ELEV_BOTTOM) {
        int d = r + 1;
        while ((tower[d][c] != TB_STATION_TOP) && (d < towerheight)) {
          if ((tower[d][c] != TB_EMPTY) &&
              (tower[d][c] != TB_ROBOT1) &&
              (tower[d][c] != TB_ROBOT2) &&
              (tower[d][c] != TB_ROBOT3) &&
              (tower[d][c] != TB_ROBOT4) &&
              (tower[d][c] != TB_ROBOT5) &&
              (tower[d][c] != TB_ROBOT6) &&
              (tower[d][c] != TB_ROBOT7) &&
              (tower[d][c] != TB_BOX) &&
              (tower[d][c] != TB_STATION_MIDDLE) &&
              (tower[d][c] != TB_STEP_VANISHER) &&
              (tower[d][c] != TB_DOOR) &&
              (tower[d][c] != TB_DOOR_TARGET)) {
            row = r;
            col = c;
            return TPROB_ELEVATORBLOCKED;
          }
          d++;
        }
        if (d >= towerheight) {
          row = r;
          col = c;
          return TPROB_NOELEVATORSTOP;
        }
      }

      if (tower[r][c] == TB_STATION_MIDDLE) {
        int d = r + 1;
        while ((tower[d][c] != TB_STATION_TOP) && (d < towerheight)) {
          if ((tower[d][c] != TB_EMPTY) &&
              (tower[d][c] != TB_ROBOT1) &&
              (tower[d][c] != TB_ROBOT2) &&
              (tower[d][c] != TB_ROBOT3) &&
              (tower[d][c] != TB_ROBOT4) &&
              (tower[d][c] != TB_ROBOT5) &&
              (tower[d][c] != TB_ROBOT6) &&
              (tower[d][c] != TB_ROBOT7) &&
              (tower[d][c] != TB_BOX) &&
              (tower[d][c] != TB_STATION_MIDDLE) &&
              (tower[d][c] != TB_DOOR) &&
              (tower[d][c] != TB_DOOR_TARGET) &&
              (tower[d][c] != TB_STEP_VANISHER)) {
            row = r;
            col = c;
            return TPROB_ELEVATORBLOCKED;
          }
          d++;
        }
        if (d >= towerheight) {
          row = r;
          col = c;
          return TPROB_NOELEVATORSTOP;
        }
        d = r - 1;
        while ((tower[d][c] != TB_ELEV_BOTTOM) && (d >= 0)) {
          if ((tower[d][c] != TB_EMPTY) &&
              (tower[d][c] != TB_ROBOT1) &&
              (tower[d][c] != TB_ROBOT2) &&
              (tower[d][c] != TB_ROBOT3) &&
              (tower[d][c] != TB_ROBOT4) &&
              (tower[d][c] != TB_ROBOT5) &&
              (tower[d][c] != TB_ROBOT6) &&
              (tower[d][c] != TB_ROBOT7) &&
              (tower[d][c] != TB_BOX) &&
              (tower[d][c] != TB_STATION_MIDDLE) &&
              (tower[d][c] != TB_DOOR) &&
              (tower[d][c] != TB_DOOR_TARGET) &&
              (tower[d][c] != TB_STEP_VANISHER)) {
            row = r;
            col = c;
            return TPROB_ELEVATORBLOCKED;
          }
          d--;
        }
        if (d < 0) {
          row = r;
          col = c;
          return TPROB_NOELEVATORSTOP;
        }
      }

      if (tower[r][c] == TB_STATION_TOP) {
        int d = r - 1;
        while ((tower[d][c] != TB_ELEV_BOTTOM) && (d >= 0)) {
          if ((tower[d][c] != TB_EMPTY) &&
              (tower[d][c] != TB_ROBOT1) &&
              (tower[d][c] != TB_ROBOT2) &&
              (tower[d][c] != TB_ROBOT3) &&
              (tower[d][c] != TB_ROBOT4) &&
              (tower[d][c] != TB_ROBOT5) &&
              (tower[d][c] != TB_ROBOT6) &&
              (tower[d][c] != TB_ROBOT7) &&
              (tower[d][c] != TB_BOX) &&
              (tower[d][c] != TB_STATION_MIDDLE) &&
              (tower[d][c] != TB_DOOR) &&
              (tower[d][c] != TB_DOOR_TARGET) &&
              (tower[d][c] != TB_STEP_VANISHER)) {
            row = r;
            col = c;
            return TPROB_ELEVATORBLOCKED;
          }
          d--;
        }
        if (d < 0) {
          row = r;
          col = c;
          return TPROB_NOELEVATORSTOP;
        }
      }

      /* check for exit, and that it's reachable */
      if (tower[r][c] == TB_DOOR_TARGET) {
        int d = r - 1;

        if (d < 0) {
          row = 0;
          col = c;
          return TPROB_UNREACHABLEEXIT;
        }

        while ((d >= 0) && (tower[d][c] == TB_DOOR_TARGET))  d--;
        if (d >= 0) {
          if ((tower[d][c] != TB_STICK) && (tower[d][c] != TB_STEP) &&
              (tower[d][c] != TB_BOX) && (tower[d][c] != TB_ELEV_BOTTOM) &&
              (tower[d][c] != TB_STICK)) {
            row = r;
            col = c;
            return TPROB_UNREACHABLEEXIT;
          }
        }
        has_exit = true;
      }

      // check doors
      if ((tower[r][c] == TB_DOOR) &&
          !lev_is_door(r, (c + (TOWERWID/2)) % TOWERWID)) {
        row = r;
        col = c;
        return TPROB_NOOTHERDOOR;
      }
      if (lev_is_door(r,c)) {
        bool A = (r > 0) && (tower[r-1][c] == tower[r][c]);
        bool B = (r > 1) && (tower[r-2][c] == tower[r][c]);
        bool D = (r + 1 < towerheight) && (tower[r+1][c] == tower[r][c]);
        bool E = (r + 2 < towerheight) && (tower[r+2][c] == tower[r][c]);

        if (!((A&&B)||(A&&D)||(D&&E))) {
          row = r;
          col = c;
          return TPROB_BROKENDOOR;
        }
      }
    }

  if (!has_exit) return TPROB_NOEXIT;

  /* other, non-tower related problems */
  if (lev_towertime() < 5) return TPROB_SHORTTIME;
  if (lev_towername().empty()) return TPROB_NONAME;

  return TPROB_NONE;
}

/* the functions for mission creation */

static FILE * fmission = NULL;
static Uint8 nmission = 0;
static Uint32 missionidx[256];

bool lev_mission_new(const std::string & name, const std::string & filname, Uint8 prio) {
  assert_msg(!fmission, "called mission_finish twice");

  // TODO remove
  char fname[200];
  snprintf(fname, 200, "%s.ttm", filname.c_str());

  fmission = create_local_data_file(fname);

  if (!fmission) return false;

  unsigned char tmp = name.size();

  /* write out name */
  fwrite(&tmp, 1, 1, fmission);
  fwrite(name.c_str(), 1, tmp, fmission);

  fwrite(&prio, 1, 1, fmission);

  /* placeholders for towernumber and indexstart */
  fwrite(&tmp, 1, 1, fmission);
  fwrite(&tmp, 1, 4, fmission);

  nmission = 0;

  return true;
}

static void write_fmission_section(Uint8 section, Uint32 section_len) {
    Uint8 tmp;
    fwrite(&section, 1, 1, fmission);

    tmp = section_len & 0xff;
    fwrite(&tmp, 1, 1, fmission);
    tmp = (section_len >> 8) & 0xff;
    fwrite(&tmp, 1, 1, fmission);
    tmp = (section_len >> 16) & 0xff;
    fwrite(&tmp, 1, 1, fmission);
    tmp = (section_len >> 24) & 0xff;
    fwrite(&tmp, 1, 1, fmission);
}

void lev_mission_addtower(const std::string & name) {
  assert_msg(fmission, "called mission_addtower without mission_new");

  Uint8 rows, col;
  Sint16 row;
  Uint8 namelen, tmp;
  Uint32 section_len;

  missionidx[nmission] = ftell(fmission);
  nmission++;

  if (!lev_loadtower(name)) return;

  namelen = towername.size();
  write_fmission_section(TSS_TOWERNAME, namelen);
  fwrite(towername.c_str(), 1, namelen, fmission);

  write_fmission_section(TSS_ROBOT, 1);
  fwrite(&towerrobot, 1, 1, fmission);

  write_fmission_section(TSS_TOWERTIME, 2);
  tmp = towertime & 0xff;
  fwrite(&tmp, 1, 1, fmission);
  tmp = (towertime >> 8) & 0xff;
  fwrite(&tmp, 1, 1, fmission);

  write_fmission_section(TSS_TOWERCOLOR, 3);
  fwrite(&towercolor_red, 1, 1, fmission);
  fwrite(&towercolor_green, 1, 1, fmission);
  fwrite(&towercolor_blue, 1, 1, fmission);

  rows = towerheight;

  /* calculate tower data section length */
  section_len = 2*towerheight;
  for (row = 0; row < rows; row++)
    for (col = 0; col < TOWERWID; col++)
      if (tower[row][col]) section_len++;
  write_fmission_section(TSS_TOWERDATA, section_len + 1);


  fwrite(&towerheight, 1, 1, fmission);

  /* output bitmap */
  for (row = 0; row < rows; row++) {

    Uint8 c = 0;
    for (col = 0; col < 8; col ++)
      if (tower[row][col])
        c |= (0x80 >> col);

    fwrite(&c, 1, 1, fmission);

    c = 0;
    for (col = 0; col < 8; col ++)
      if (tower[row][col + 8])
        c |= (0x80 >> col);

    fwrite(&c, 1, 1, fmission);
  }

  /* output bytemap */
  for (row = 0; row < rows; row++)
    for (col = 0; col < TOWERWID; col++)
      if (tower[row][col])
        fwrite(&tower[row][col], 1, 1, fmission);

  /* output towerdemo */

  if (!towerdemo.empty())
  {
    Uint8 run;
    Uint16 data;

    /* calc data length */
    run = 1;
    data = towerdemo[0];
    section_len = 2;

    for (size_t idx = 1; idx < towerdemo.size(); idx++) {
      if ((data != towerdemo[idx]) || (run == 0xff)) {
        section_len += 3;

        data = towerdemo[idx];
        run = 1;
      } else
        run ++;
    }
    if (run) section_len += 3;

    write_fmission_section(TSS_DEMO, section_len);

    /* output length */
    tmp = towerdemo.size() & 0xff;
    fwrite(&tmp, 1, 1, fmission);
    tmp = (towerdemo.size() >> 8) & 0xff;
    fwrite(&tmp, 1, 1, fmission);

    /* output data using a simple runlength encoder */
    run = 1;
    data = towerdemo[0];

    for (size_t idx = 1; idx < towerdemo.size(); idx++) {
      if ((data != towerdemo[idx]) || (run == 0xff)) {
        fwrite(&run, 1, 1, fmission);
        tmp = data & 0xff;
        fwrite(&tmp, 1, 1, fmission);
        tmp = (data >> 8) & 0xff;
        fwrite(&tmp, 1, 1, fmission);

        data = towerdemo[idx];
        run = 1;
      } else
        run ++;
    }

    if (run) {
      fwrite(&run, 1, 1, fmission);
      tmp = data & 0xff;
      fwrite(&tmp, 1, 1, fmission);
      tmp = (data >> 8) & 0xff;
      fwrite(&tmp, 1, 1, fmission);
    }
  }

  write_fmission_section(TSS_END, 0);
}

void lev_mission_finish() {
  assert_msg(fmission, "called mission_finish without mission_new");

  Uint8 c;

  /* save indexstart and write out index */
  Uint32 idxpos = ftell(fmission);
  for (Uint8 i = 0; i < nmission; i++) {

    c = missionidx[i] & 0xff;
    fwrite(&c, 1, 1, fmission);
    c = (missionidx[i] >> 8) & 0xff;
    fwrite(&c, 1, 1, fmission);
    c = (missionidx[i] >> 16) & 0xff;
    fwrite(&c, 1, 1, fmission);
    c = (missionidx[i] >> 24) & 0xff;
    fwrite(&c, 1, 1, fmission);
  }

  /* write out the number of towers in this mission */
  fseek(fmission, 0, SEEK_SET);
  size_t read = fread(&c, 1, 1, fmission);
  assert_msg(read == 1, "could not read data");

  fseek(fmission, c + 2, SEEK_SET);
  fwrite(&nmission, 1, 1, fmission);

  /* write out index position */
  c = idxpos & 0xff;
  fwrite(&c, 1, 1, fmission);
  c = (idxpos >> 8) & 0xff;
  fwrite(&c, 1, 1, fmission);
  c = (idxpos >> 16) & 0xff;
  fwrite(&c, 1, 1, fmission);
  c = (idxpos >> 24) & 0xff;
  fwrite(&c, 1, 1, fmission);

  fclose(fmission);

  fmission = NULL;
}

