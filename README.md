# Tower Topper aka Nebulus

## Introduction

In this puzzle platformer where you have to help a cute little green dude
switch off some kind of "evil" mechanism. The "power off switch" is hidden
somewhere in high towers. On your way to the target you need to avoid a lot of
strange robots that guard the tower.

That sounds all like a normal jump and run game. What makes this game different
is that you walk around the tower which is revolving on the screen, so that you
only see the 180° that are currently visible.

The game is a reimplementation of the old game known as Tower Toppler or Nebulus.
At its time it was ported available for a lot of different platforms like PC,
Atari, C64. The author was J.M.Phillips and was published by Hewson software.

Here are a few shots of some of the available original versions

![](/doc/old-amiga.png)
![](/doc/old-atari.png)
![](/doc/old-c64.png)
![](/doc/old-cpc.png)
![](/doc/old-cga.png)
![](/doc/old-ega.png)


Because I really enjoyed this game when it was published I reprogrammed it some years ago using PASCAL.
I painted all the necessary graphics on my own (or used POVRAY) and finally got a program that behaves
nearly identical if compared with the original.

Some time ago I decided to port this version to LINUX and C using SDL for the graphic and sound
output. And here is the result.

## Status

The game fully playable. Everything is in place. Some more sounds could be added, surely some bugs
are left but there should be nothing too serious.

There is a level editor that allows everyone to create new towers and mission files. So I am awaiting all your
self-made missions.

## Latest changes

Version 1.3 6.Feb.2022 maintenance release

- fix centering of background image in highscore menus
- add a bigger icon
- fix global highscore folder
- data file is no longer re-created unnecessarily
- install man-page
- updated translation for Czech and Finnish language

Version 1.2

![](/doc/TowerGraphicsCompare12.png)


User visible changes:
- port to SDL2 with the accompanying benefits:
  - proper fullscreen support
  - resizeable window
  - variable aspect ratio properly used (instead of letterboxing) so you can finally enjoy Toppler on a widescreen
- improved visuals:
  - new brick textures that look quite a bit more 3d
  - shadows behind objects (robots, tower elements)
  - objects get darker as they recede behind the tower
  - better quality for the colour reduced data resulting in all around better looking graphics (only slightly though)
  - improved bonus level graphics (better horizontal match, better antialiasing)
  - fix a bug in sprite rendering that resulted in an strange looking edge for objects
  - smoother animation for the robots with twice the framerate and also slightly improved animations

Under the hood
- Drop autoconf stuff and use a makefile instead
- fix all warnings
- cleanups inside the datafile creation tools
- remove the worst non-c++ stuff and replace by c++ nicer constructs I think the code looks much better now
- datafile optimisations (a slight bit smaller though that got eaten up by the graphics improvements from above)

For a complete release history look into doc/Changelog.md

## Building and Installation

Dependencies for building the game only are

- SDL2_mixer  (with ogg support if you want music)
- sdl2
- zlib

if you want to rebuild the datafile you also need

- libpng
- povray
- gimp
- blender
- imagemagick


If you got all these dependencies just type

```
make 
```

and if your want to install

```
make install
```

but you an also directly start the game from the source directory using 

```
./toppler 
```

## Game goal

The goal of the game is to reach the target door of each of the 
towers in the packages mission. This door is usually at the very top of the tower.

But finding the way by using elevators and walking trough a maze of
doors and platforms is not the only problem you have to solve. There
are a bunch of other creatures living on the tower that will hinder you
to reach your target by pushing you over the edge of the platforms.

The only weapon of defence you have is to throw a little snowball. But
most of the other creatures just don't care about this. So you must
avoid them.

A little submarine brings you from one tower to the next. On this way
you have the chance to get some bonus points by catching fish. All you
have to do is to catch a fish in a bubble with your torpedo and then
collect the fish.

## Commandline Parameters

-f starts the game in fullscreen mode. But be careful! If the game
crashes it doesn't restore the original resolution, it stays in 640x480
pixel mode.

-s makes the game silent. If you don't have a soundcard or for another
reason get an "can't open audio" error try this option.

## Options Menu

### Game Options

- Password: this menu entry allows you to define a password. The
password us used to restart a mission from a tower that is not the
first. You get a password for every third tower while you are playing

- Lives: for the tough guys you can decrease the number of lives you
start the game

- Status on top: if you prefer the game status at the bottom of the
screen. Here is the place to switch

- Game speed: if you want faster gameplay, then this is the option for
you. This option doesn't influence the behaviour. Only the delays
between the frames is decreased. 

### Redefine Keys

The options here allow you to redefine the keys that are important for
playing. Select the key you want to redefine. Press Enter and then the
new key you want to use. Bare in mind, that you are not overwriting the
old keys but just define an alternative that has a lower priority than
the original keys. So you can not use "p", because this is already used
for pause.

### Graphics

The submenus here allow you to finely tune the looks of tower toppler.
Depending on your computers capabilities and you graphic card you can
change here between very low requirements and modest ones. I have an
Intel Celeron 366 and can play Tower Toppler with everything switched on
except for the expensive waves.

- Fullscreen: Toggle between windowed and fullscreen display.

- Alpha options toggle the usage of alpha for the different objects
that are drawn on screen. Alpha blitting requires a bit more time, so
if the game runs too slow, switch some of them off.

  - Fonts: for all the text displayed

  - Sprites: for all robots and the toppler

  - Scroller: for the underwater scroller in the bonus level

  - Shadowing: toggles if the menu uses a translucent bar and the
screen gets darker if you start the menu in the game and on some other
occasions. Shadowing takes up quite a lot of time.

- waves: here you can toggle between nonreflecting, simple and
expensive waves. Nonreflecting waves take up the lest amount of CPU
time but don't look very good. Expensive waves look quite good but
require quite a bit of CPU time.

- scroller: here you can toggle between a 2 layer scroller and the
complete (currently 3 layer) scroller. The 2 layer scroller only draws
the front and the backround layer of the underwater scroller in the
bonus level.

- shadows: toggles drawing of shadows behind objects and fading objects to black as they recede behind the tower.
Enable this to get a much more 3d-looking level display at the cost of a bit of performance.

Bonuses:

This toggle allows you to suppress bonus levels. Some people don't enjoy
hunting fish as much as climbing towers, this allows them to only play the
fun part ;-).

### Speed Indicator

In the normal tower climbing game there is an indicator displaying if
your computer is fast enough for the current playing mode. If you can
see a small red square in the top left corner of the game then the game
can not run as fast as it needs to. Consider the following options:

- drop game speed in the game options menu

- don't use the expensive waves, they ARE expensive in CPU time

- switch off some of the alpha options

### Controls

In the menu you can select the mission you want to play next with the
left and right cursor keys. Press space or return on the start menu
item to start the game.

The animal is controlled by the cursor keys and space (or return). Left
and Right make the animal walk. Up and down make the elevators move if
you are on one. (The elevator platforms are a little bit smaller than
the normal platforms.)  If you are in front of a door press up to enter
it.  Pressing the space key will either throw a snowball if you are
standing still or make the animal jump if you are walking.

### For the Leveldesigner


The game finally has a leveleditor. Because I've been too lazy to make
new graphics for special objects all tower objects are drawn using the
existing graphics.

Because the only thing you can rely on on a keyboard are the numbers
and the letter keys I have used only these. The layout is chosen so
that keys with a similar task are next to each other and not that the
letters mean something.

Otherwise the editor is relatively simple:

#### help

| key | description |
|-----|:-----------:|
| h | shows a list of all keys with explanations |

You can move in the list with up, down and pgup, pgdown

#### cursor movement

cursor keys

| key | description |
|-----|:-----------:|
| page up and down | 5 rows up and down |
| y | half turn of the tower (for doors) |
| HOME | go to starting position |

#### actions

| key | description |
|-----|:-----------:|
| p   | play, to test the level you are just editing |
| ESC | leave the editor |
| l |   load tower |
| o |   save tower |
| z |   check consistency of tower |

The consistency check checks if all doors are exactly 3 layers high, if
there isn't a undefined symbol in there and if there is always the
opposite platform for the elevators (without unremovable obstacles
in between). If there is something wrong with the tower, the cursor
moves to the position of the problem.

#### editing

| key | description |
|-----|:-----------:|
| ins and f | inserts a row below the current position |
| del and g | deletes the current row |

with these actions you must be careful because they can destroy doors.

#### objects

| key | description |
|-----|:-----------:|
| space |  clear field |
| q  |     normal pillar |
| a  |     flashing box |
| w |      normal step |
| s |      vanishing step        displayed as flashing step |
| x |      sliding step          displayed as sideways moving step |
| e |      elevator top station |
| d |      elevator middle station |
| c |      elevator top station |

the elevator stations are shown by a moving platform. The platform is
moving into the direction the elevator can move. So for the bottom
stations the platform is repeatingly moving up and jumping down.

| key | description |
|-----|:-----------:|
| 1-7 | the different kinds of robots |

for the eye robot the movement is shown by the movement of the robot in
the editor. The slow moving version moves half as far as the fast
moving one.

The two jumping balls can be identified by ball movement: one is
standing still. This is the one that jumps on its place until the
toppler gets near. The ball that has a sideway movement from it's
beginning on is moving in the editor too.

| key | description |
|-----|:-----------:|
| i |      normal door |
| k |      target door           displayed as flashing door |

This action always places a whole door (in 3 layers) and tries to place a door on the opposite side, if it's empty. Doors should never be higher then 3 layers. The clear command (' ') will always remove the whole door but not the one on the opposite side of the tower.

The target door command doesn't place a door on the opposite side

#### parameter of the tower

the parameter are always in a range of values. To allow fast
alterations of the values the difference between the old and the
changed value increases every time you go into the same direction. So
if you increase the time for the tower by pressing 'b' the first time
the value increases by one the second time by two the third time by
three and so on. This size is reset to one as soon as you change the
direction of your changes. This sounds complicated but the best thing is to
try it

| key | description |
|-----|:-----------:|
| b |      increase time |
| n |      decrease time |
| v |      open palette dialog |

In the palette dialog up and down change the color you want to change.
Left and Right change the value by one, PgUp and PgDown change by 10
and the numbers make the value jump to 10 equally distributed positions on the
scale

### Tower File Format

Just in case you are interested in the format of the tower file:

The format is similar to the windows configuration format. Each section
starts with its name in square brackets "[]". Unknown sections are
ignored. Most sections are self-explanatory, except for the data
section.

This section has the tower height in its first line followed by a table
where each character represents one position on the tower. The
following table explains the characters used for the tower bitmap and
their meaning.

| character | description |
|-----|:-----------:|
| space|    empty |
| v | top elevator station |
| + | middle elevator station |
| ^ | bottom elevator station |
| 1 | freeze ball |
| 2 | jumping ball moving from the start |
| 3 | jumping ball standing and then moving to you |
| 4 | robot, up down |
| 5 | robot, up down fast |
| 6 | robot left right |
| 7 | robot left right fast |
| ! | pillar |
| - | platform |
| b | flashing box |
| . | vanishing platform |
| > | slippery platform |
| < | slippery platform |
| # | normal door |
| T | target door |

The demo section is also not really obvious. The numbers represent
states of the important keys. Each bit in the number corresponds to one
key. If it is set the key is pressed, otherwise not. Because Tower
Toppler is strictly deterministic and uses fixed intervals for display
and update each line corresponds to one cycle of update and display.

I can not promise that the program will be able to load the files if
you are changing them manually, so better leave your fingers away from
these.

Because of the ongoing development of Tower Toppler this format is
likely to change. New sections may be added, other ones deprecated. I
hope that the section format will help to achieve backwards
compatibility but I promise, of course, nothing. If you have problems
loading your old tower files, send them to me and I'll do my best to
convert them.

Also, if you want to send me your missions, please do not send the
mission files, but the towers. Because the mission files are binary and
are changing as well it may be quite complicated to convert a mission
file generated with an old version of Tower Toppler to a new version.
But it will nearly always be possible to edit the tower files.

### Mission File Format

And another paragraph for the interested ones. Here the format of the
mission files is explained.

First in the mission files is the mission name. The first byte is the
length of the string followed by the text itself. Then comes a
priority. This value is used to sort the missions in the game menu.
Then comes the number of towers in the mission followed by the offset
of the tower offset table (b bytes). This table contains 4 bytes for
every tower showing the starting position of the tower inside the file.
The offset table is normally at the end of the file.

Now to the towers. They are structured into sections just as the tower
file is. The section header contains 5 bytes. One shows the section
type. The other 4 the length in bytes of the section data. Then the
data follows. This allows the program to skip unknown sections.

The section numbers can be found in level.cc.

Most of the sections are fairly simple. Except for the tower and demo.

The tower data is split into 2 parts. First a bitmap detailing which
places do contain something. For each occupied place there is a byte in
the following data. The byte details what occupies the field. Details
in the source.

The demo uses a run length encoding. One byte for the run length and
one word for the key state.

So this should help to find your way into the code, if it's necessary.

### Mission Creation

With the key m in the level editor you start creating a new mission. Be
aware that mission contain your towers in a compressed format and I
will not provide a method to extract the original tower from this to
prevent player from cheating. So keep your original tower files in case
there are bugs in your towers and you need to change things.

What follows now are a pile of questions you have to answer by keyboard
input.

First the name of the mission. The mission creator will write a file to
your disk with the same name as the name of the mission, but you are
welcome to rename the file because the mission name is saved inside the
file too. The only thing you have to take care of is that the file has
".ttm" (tower toppler mission) as extension.

Now input all the filenames of the towers you want to add to this
mission. The limit to the number of towers is 255 but I'd suggest to
not use this many because it would take a lot of time to play through
the complete mission.

If all towers have been input enter an empty name to finish this list.
Now the file will be written and you should be able to select your new
mission in the main menu.

IMPORTANT: please check your towers, before you create a mission, for
inconsistencies, because this is not done in the created mission file.

### Helpful Hints

Finally some details about the inner workings of the game that might
help design levels.

- There are never more than 4 robots on screen (including the
  horizontal spinning object)
- robots that go below the screen vanish
- robots that fall into water vanish
- the spinning object has a timeout, if this time passed it will come
  as soon as there are less then 4 robots on screen
- other the robots appear as soon as they are inside the screen and
  there are less then 4 robots on screen
- the behaviour of the robots is completely deterministic
- the cross throws you 4 layers down, the other robots much more
- the starting place on the base is always the same, it's on top of the
  2nd row on angle 0 (where the cursor is at the start HOME moves the
  cursor to this position)

### FILETYPES

Where does the program look for the different files?

- datafiles (graphics, sounds), readonly
  - first in local directory,
  - then global (per compiler switch /usr/local/share/toppler)

- local configuration file, read/write/create
  - in home directory

- level editor, new missions, towers read/write/create
  - in home dir (/home/user/.toppler/...)

- highscore file, read/write/create
  - in global dir, home,

# Contact

Webpage: toppler.sf.net
Email  : roever@users.sf.net

# Thank yous

Thank you goes to all the people out there who sent me bug reports and praise. But ecspecially to
- Pasi Kallinen for tons of improvements. He coded the menu system, the demos and provided many
other features for the game,
- Bastian Salmela for his very nice graphics,
- Clarence Ball for his missions,
- Andreas Wagner for the web page design.

