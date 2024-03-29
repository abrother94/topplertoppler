# 06.02.2022 Version 1.3

- fix centering of background image in highscore menus
- add a bigger icon
- fix global highscore folder
- data file is no longer re-created unnecessarily
- install man-page
- updated translation for Czech and Finnish language

# 30.01.2022 Version 1.2

User visible changes:
- port to SDL2 with the accompanying benefits:
  - proper fullscreen support
  - resizeable window
  - variable aspect ratio properly used (instead of letterboxing)
- improved visuals:
  - new brick textures that look quite a bit more 3d
  - shadows behind objects (robots, tower elements)
  - objects get darker as they receed behind the tower
  - better quality for the colour reductions
  - improved bonus level graphics (better horizontal match, better
     antialiasing)
  - fix a bug in sprite rendering that resulted in an ugly antialiasing border
  - smoother animation for the robots with twice the framerate and also
    slighty improved animations

Under the hood
- Drop autoconf stuff and use a makefile instead
- fix all warnings of modern compilers
- cleanups inside the datafile creation tools
- remove the worst non-c++ stuff and replace by c++ nicer constructs
   I think the code looks much better now
- datafile optimisations (a slight bit smaller, though that got eaten up by
   the graphics improvements from above)

# 21.12.2012 Version 1.1.6

- fix memory leaks
- proper program ending instead of an uncaught exception
- fix all warnings when compiling
- make compilable on modern systems
- included missions are now in the datafile insted of separate files
- fix joystick support

# Versions 1.1.4, 1.1.5 
not publicly announced and invalid

# 30.10.2006 Version 1.1.3

- add Mission "David 1"
- internationalisation support for Windows
- translations for Basque, Czech and Romanian

# 21.01.2006 Version 1.1.2

- another buildsystem update (by Volker Grabsch)
- background music (by Angel Keha)
- portugese localisation
- long tower names are broken into several lines
- swedish translation
- small bug that made the toppler get stuck under very special conditions

# 18.12.2004 Version 1.1.1

- really include pasis mision this time
- add finish translation
- updated to the french translation

# 05.12.2004 Version 1.1.0

- remove a possibility for the toppler to get stuck, when the elevator fell down
- make it possible to not use internationalisation for BeOS
- fix saving of bonus level option
- add joystick support (untested by me)
- new sound system
- update build system
- move the sound files into the data file
- add 2 tower mission with 2 towers from Pasi Kallinen
- make sticks in front of doors possible, so that elevators can pass doors now

# 08.02.2004 Version 1.0.6

- add 5000 points for each life left after the last tower is finished
- more characters, should now be possible to add all unicode character,
  it's not a nice solution, but simple and working
- add characters for Esperanto ^c, ^g, ^h, ^j, ^s and ~u
- fix mismatching new [] delete problem

# 25.01.2004 Version 1.0.5

- fix encoding bugs
- fix ê letter

# 18.01.2003 Version 1.0.4

- smoother toppler images
- move bonus level switch into game menu
- internationalze, add German, add French
- add some french letters, resize letters for accents

# 09.10.2003 Version 1.0.3

- add switch to skip bonus level, thanks to Beni Cherniavsky
- add checks for different video modis (thanks Stephane Marchesin)
- fixed bug with freeze balls that makes them go wild, when the
  ground blow them vanished
- hopefully fix bug that caused the timeout screen to become
  a highscore screen with timeout text
  

# 08.06.2003 Version 1.0.2 

- add check for sound on torpedo off, this fixes a crash
- update the green dude to have a higher resolution
- add not into README that signed versions of the game
  are allowed

# 15.04.2003 Version 1.0.1

- remove debug printf in highscore module
- add asert after screen initialisation to check for failure

# 08.03.2003 Version 1.0.0 

- check for the existence of the datafile
- merge the 2 READMEs the have somehow appeared
- some soundeffects for the bonusgame
- GPL header for files

# 23.02.2003 Version 0.99.2

- make tower beeing destroyed completely, even if target toor is not at top
- menu system cleanup, improvement in background image drawing
- fix in tower and level loader robot number are now taken modulo actual robot counter
- check mission prior to starting if there are unknown building block in there
  this should allow to add new features and the old games might still work

# 18.02.2003 Version 0.99.1 

- double file close on highscore saving lead to segmentation fault
- on windoes systems highscore table asn't opened in binary mode

# 17.02.2003 Version 0.99.0 

- fix highscore bug
- replace all sprintf with snprintf to avoid buffer overflows
- finally there is the global highscore table
- fix config saving bug
- cleanup of archive interface, makes it easier to handle
- separate out highscore handling
- rpm installes global highscore
- readme updates
- add clarences 3rd mission

# 14.01.2003 Version 0.98.4 

me:

- switch to 16 bit graphic mode -> huge speed improvement
- accelerate waves
- option to switch off all but the top and bottom layer in bonus game
- bugfix in level loader
- include new levels from clarence ball (ball 2)
- bugfix in event handling (mouse clicking)
- bugfix in level.cc that slightly changed the robot behaviour and made 
  the demos run differently
- allow to change the robot type in leveleditor
- use typedef instead of struct xxxxx
- use new and delete instead of malloc and free
- move function delcarations into c file in menu.cc, they are not
  needed outside of this module
- remove unnessesary this-> in textsys
- raise maximum number of active elevators because it's possible to
  create levels where you have that many (although not useful)
- create class for configuration
- cleanup in keyb, switch on key repeate
- speed indicator in tower game
- readme update (options menu, speed indicator, fileformats)
- use old highscore fileformat with fixed number of entries, because
  new code results in crashes with old fileformat
- fix windows sound
- fix keyboard problems on Windows systems and on some keyboards  

# 20.12.2002 Version 0.98.3 

pasi:

- sections in mission files, so from now on it should be
  possible to keep missions backward compatible.
  (text-format towers are still without this)
- dynamically allocated highscore list.
- highscore list also saves the tower reached.
  (-1 means the player played the mission through)
- highscore list doesn't show the pages with all scores
  0 and no names.
- bugfix: toggling alpha options doesn't change
  the star positions now.
- bugfix: the level editor key help showed the title
  screen background if you went to the key help right
  after entering the editor.
- debugging level: output via debugprintf(),
  debug level can be set on the command line. (-d option)
- updated the man-page to have the -d parameter,
  and some other small changes in it.
- level editor keys can now have modifiers:
  shift, control and/or alt.
- new type of tower block: step that slides to right.
  works like the slider left, exactly
- added "Put slider right" to the level editor keys,
  bound to shift+x.
- readded "Decrease time" to the level editor keys,
  bound to shift+n.
- level editor shows key, key modifier and action on
  the console if debug_level >= 4.
- in toppler.spec.in lowered the BuildRequires SDL-devel
  version from 1.2.4 down to 1.2.2.
  (I use that SDL version to compile, so i guess it works ;)
  [That is, if i'm guessing correctly what BuildRequires means...]
  
me:

- bugfix for big endian systemy
- some speed improvements (thanks to Aurel Schwarzentruber)
- lots of typos were fixed, some function names were renamed to better
  enlish names
- small updates to documentation
- h and F1 activate helpscreen in leveleditor
- one more type of water
- now it's possible to have different lenght robot animations
- sections in tower files, this breaks compatibility hopefully for the
  last time
- each tower can have a selectable robot

# 26.11.2002 Version 0.98.2

-when user selects "Quit game" from the game menu during
 game, do not show hiscores after that, but go directly
 back to main menu.

-Bugfix: In level editor, if you loaded a tower which was shorter
 the cursor stayed higher than the new tower's top.

-In text input routine, if a low-case letter is allowed but
 not high-case, and user enters high-case letter, convert
 the letter to low-case. And vice versa.

-you can now define justification better for menu options:
 left justify, right justify, or centered.
 (MOF_LEFT, MOF_RIGHT)

-if we have alpha for sprites on, use that instead of
 blinking the vanishing steps in the level editor.

-in level editor, added 2 new commands: cut and paste row(s).

-level editor tower start height and page up/down movement size
 are saved to config file.

-key redefinitions are saved to config file.

-Bugfix: when game was waiting in some loops,
 killing wasn't effective. (no checking of quit_action)
 (leveledit.cc: tower color changer and key help,
  game.cc: bonus(), keyb.cc: wait_for_focus())

-Use UNICODE to get input for input fields; this
 is toggleable in the config file, as it causes a
 bit of an overhead. If this is on, the keyboard works
 more like expected in men_input(), eg. shifted keys work.
 There's lots more characters missing from the char set
 grpahics, eg. @, -, which should be available...

-changed men_input() to nonblocking function.

-in level editor, in tower color changer, you can now
 press '.' to randomize the colors.

-generalized textsystem, similar to menu system.
 level editor key help uses the textsystem.

-level editor: new command: adjust tower height.
 it asks for an input, and depending on what you give,
 changes the tower height:
   100 => set tower height to 100
   -50 => shorten the tower by 50,
   +25 => make tower 25 lines taller.

 NOTE: there's no minus in the character set graphics!!

-you can now select how many lives you have when starting the
 game; 1-3 lives.

-tower passwords; you can enter a password in the menu, and
 then start a game. The passwords are calculated from the
 tower[][] -array. (see gen_passwd() in level.cc)
 The game shows the password every 3rd tower,
 unless it's the first or last tower of the mission.
 The last entered password is saved to config file.
 (see level.cc: gen_passwd(), lev_get_passwd(), lev_show_passwd(),
  lev_tower_passwd_entry())

-added a new level editor command: goto end. it finds the target
 door, and moves cursor there.

-level editor background screen is now darkened when the editor
 waits for input or shows some informational text on the screen.

-3 new text formatting commands for scr_writeformattext():
 ~T###  - same as ~t, but relative to x, instead of screen edge.
 ~b#    - show a tower block.
 ~e#    - show a tower block in level editor style.

-added a new function scr_formattextlength(), that returns
 the length of formatted text.
 (Needed to make the level editor key help work with the
 generalized text system as previously)


# 24.11.2002 Version 0.98.1 

- Added a new config option "status_top"; this tells whether
  the status line is on top or bottom of screen.
  Currently this option is not in the menus. (OK, so this is
  a new feature =)
  
- Fixed some typos.
  
- Don't play alarm-sound if time < 0
  (This should never happen, but just in case...)
  
- Fixed a bug that made mouse clicks not activate menu options.
  I noticed this one too =)
  
- Changed the key definitions from #defines to enum ttkey.
  
- Allow setting menu option centering individually for each menu option.
   |
   V
- Key redefinition menu now looks slightly better; the options
  do not move around.
  
- Hiscore-screen now shows the mission name.
  
- In hiscore screen, doubled the empty spaces between
  rank, score and playername, and added '.' to the
  rank number.
  
- Fixed a bug in the menu system, which allowed highlighting
  illegal options, with no text.
  
- Use enum for towerblocks, instead of hardwired bitmask values.
  (Now we have lots of room for new blocks!)
  You need to remake the missions.
  
- Fixed a bug in the config parser, which would cause strange crashes,
  if any string variable ('editor_towername') had empty string as param.
  (This is the bug that caused TT to crash when the 2nd satellite tried
   to come on screen)
  
- Config file parser now also accepts "yes" or "true"
  for boolean values.
  
- Config parser is now more tolerant of malformed config file, and
  fscanf() cannot overrun the char buffers.
  
- Fixed 2 bugs in main_game_loop():
  1) Don't try to load a tower for bonusgame after the last tower
     in a mission is finished.
  2) End mission when the last tower is finished, instead of
     after 8 towers.
  
- Fixed a bug in the bonus game that prevented the next tower color
  being set.
  
- Fixed the line input routine to work with the variable width font
  by showing arrows at the end of the input area if the string is
  longer than what can be seen.
  
- When the window loses focus, darken the window, and wait, giving
  timeslices away, until it regains focus.

me

- enclose debug settings inside TESTER compiler variable to prevent
  ever releasing a version with falst setings again
- added arrows to the left and right of the start menu entry to show
  what to do
- levels contain priority that orders them
- workaround for SDL RLEAccel bug


# 18.11.2002 Version 0.98.0 

pasi

- max. # of characters in the charset changed to 128.
- changed fonttoppler and fontpoint to be in the
  unprintable range. (frees '#' and '\')
- lev_is_consistent() returns 3 new errors: not enough time,
  tower is too short, and tower has no name.
  (these have the lowest priority)
- the new tower in level editor begins 4 rows taller
  (minimum legal height), and the time starts somewhere
  around 100 - 600.
- fixed a bug where the game would hang, if the hero
  reached the target door, and the door was on the 2nd
  row from bottom of the tower.
- toppler now dies gracefully, both with "kill" or
  from window manager "close window" -button click.
- you can now navigate the menus with mouse.
  (and depending whether you have the 4th and 5th button
  set as the wheel on wheelmouse, you'll be able to use that too)
- in fullscreen mode, mouse is disabled and becomes hidden.
- fixed a bug in level editor: not enough of the tower-array
  was cleared when starting the editor.
- very colorful highlight-bar, that moves in increments, instead
  of warping from one option to another.
- level editor: added a way to enter the tower time directly.
- level editor: input lines now show a text of what they ask.
- level editor: tweaked the colors in the tower color changer.
- commented out scaling-option from options -menu.
- you can now redefine keys, "redefine keys" option in
  the options-menu.
- fixed typos, mainly in comments.
- toppler.dat can now contain any number of files,
- changed archi.cc to have the same length file name
  in the fileindex as in the other diff. (8.3 filenames)
- added GAME_DEBUG_KEYS define to decl.h. If you
  define that, you can press up+down+left+right
  during game and get a debug menu. Hiscores are
  not saved if this is defined.
- changed data file names to have the '.dat' suffix
  in decl.h.
- editor now doesn't display the tower time during
  some operations. (eg. during key help)
- editor key help and menu quick keys now use SDL keysyms
  instead of chars.
- player lives are now shown on the upper right corner,
  and if player has more than 3 lives, they're shown
  in a 'short' format.
- screen.cc: bonus game now supports more than 3 scrolling
  layers, and loading the layer speeds (and the tower speed
  and layering depth) from toppler.dat. I haven't tested this
  much, but at least the current setup works.
- moved some hardwired constants from bonus game to decl.h
- stars.cc: made the max. # of stars dynamic.

my work

- switched to double resolution graphics and replaced all the
  graphics with new ones
- alpha blended sprites

# 06.10.2002 Version 0.97.1

- improved autoconf and automake (hopefully)
- switch from palettized to true color mode -> remove palette module
- fix bug in config.in enabled SDL_mixer always
- fix name clash with standard scandir and alphasort
- remove some warnings

# 28.09.2002 Version 0.97 

work from pasi

- better consistency check
- fixed typo "abbort".
- fixed bug that made stars appear in the sky when moving to right.
- fix compiler complaints.
- allow toggling sounds on/off from the options -menu.
- better looking hiscore list.
- blinking stars.
- spiffy new line inputting routine.
- more user-friendly Yes/No -menu, where user can select with keyboard.
- interactive tower color changer in level editor.
- status line in level editor.
- interactive key help in level editor accessed with the 'h' key

my work

- removed "original by" and "programmed by" from menu picture
- small letters in levereditor
- keyb can output small and capital letters
- adapt input to new font
- fix bug in menu creation that made the help text vanish
- fix bug in tower loading routine that screwed up the top line of the tower
- recreated mission files that were corruped by this bug
- fix buglet in mission 1 tower 7
- fix buglet in input code
- new better readable font (update to most output routines)
- different sized stars
- include basses new yellow submarine (tm)
- include basses nice new layered graphics for the bonus game
- include compression in the datafile (zlib)
- change layers data to use data file too (remove layer?.tga)
- get rid of this terrible page wise saving of data, so I could finally remove
  the routines that were descrambling the data

# 20.02.2002 Version 0.96

- first adaptions for background music
- fix potential bug with robot drawing when a mission has more than 8 towers
- remove nosound option in config file, use -s option if you need it
- fix bug with some elevators that didn't work properly
- better depth ordering in output of tower, should be always right now

# 06.02.2002 Version 0.95

- changes to compile successfully on sparc
- change some datatypes to Uintxx and Sintxx (from SDL)
- improved configure script a bit
- move the "quit" entry in main menu to the bottom
- added an option for scaling in the options menu (for slow computers)
- small buglet in leveleditor output routine that made robots flip from in front
  to behind the tower

# 26.01.2002 Version 0.73

- fixed bug with wrong loops in robots that overwrote some global variables on windows
- fixed bug in archive that made the font loader load one char too much
- added filename for loading and saving in leveleditor
- added security check on load if tower has been changed
- color and time changes are real changes and lead to the savety questions on exit adn load
- color circling in input line (to see it on darker backgrounds
- mission creator finished
- new highscore file format for missions, once again the old one is unusable :-(
- include code to search for missions and list them (need some improvement like sorting and
  remove double entries)

# 20.01.2002 Version 0.72

dthurston:
- Added a manual page, stolen from Debian package
- Use autoconf to control installaton directory
- Changed extra life logic to be more robust/flexible.

roever:
- mark new entry in highscore table view
- added configuration file
- changed palette handling
- added options and leveleditor to main menu, changed working of main menu
- introduced unified opening scheme for files
- made the program mute automatically, if audio could not be opened
- tried to incorporate c64 sound effects
- fixed too small data type for score, makes highscore list unusable,
  delete it
- fixed bug that allowed robots to throw you down when you were invisible
- more sound effects (timeout, bonus points)
- file open looks in actual directory as well as in data directory

# 03.08.2001 Version 0.71

- moved score table into home directory
- fixed bug in highscore table
- fixed bug in sample sound volume control that made other
  samples get volume changed
- pause and abbort in bonus game possible
- draw tower in bonusgame
- move submarine to tower at the end of the bonusgame
- save palette in bonusgame
- volume change in ball bouncing doubled
- fixed bug in pushing toppler aside when elevator falls down removed

# 31.07.2001 Version 0.70

- add bonuslevel (but it uses ugly graphics)
- add autoconf and automake support (I am a novice, so don't
  expect too much)
- push animal aside when the elevator comes down and the
  toppler is standing in the way

# 26.07.2001 Version 0.63

- bugfix, buggy bonuslevel deactivated

# 25.07.2001 Version 0.62

- renamed *.c to *.cc
- added bonuslevel (with really poor graphics)
- add more sound effects
- made ball bouncing volume dependend on angle difference
- made water sound start on arrival and end on pick up
  and stop water in pause and when ESC
- made the last bubbles diappear when drowned, shortened delay
- added lots of #includes to make toppler compile onf RedHat (tanks to
  Jean-Sébastien Lebacq)
- updated README with command line parameters
- optimized scaling function a bit
- change includes for SDL headers to be in subdirectors
- fill the gaps at the screen side with water

# 18.07.2001 Version 0.61

- fixed a nasty bug in drawing of highscores that made the game
  crash in many different points
- Hide mousecursor in window
- Add fullscreen option (Handle with care)

# 15.07.2001 Version 0.60

- set windowtitle to "Nebulus"
- made the elevator platform not get big when you get thrown off the
  elevator and when the elevator drops down
- bug when leaving the elevator at the bottom station it
  dropped further down
- added plashing waves sound
- added foot tapping
- added more sounds
- added a Makefile

# 08.07.2001 Version 0.52

- removed some unused functions
- more translations

# 01.07.2001 Version 0.51

- major code cleanup
- translate comments and functionname to english
- some little bug fixed


# 25.06.2001 Version 0.50

- initial release

