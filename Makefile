# Tower Toppler
#
# Tower Toppler is the legal property of its developers, whose
# names are listed in the AUTHORS file, which is included
# within the source distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
STATEDIR = /var/toppler
LOCALEDIR = $(DATADIR)/locale
MANDIR =  $(DATADIR)/man
PKGDATADIR = $(DATADIR)/toppler
DESTDIR =
CROSS =
INSTALL = install
INSTALL_DIR = $(INSTALL) -m755 -d
INSTALL_PROGRAM = $(INSTALL) -m755
INSTALL_DATA = $(INSTALL) -m644

CONVERT = convert
CXX = $(CROSS)$(CXX_NATIVE)
CXX_NATIVE = g++
CXXFLAGS = -Wall -Wextra -g -O2 -std=c++17
EXEEXT = $(if $(filter %-w64-mingw32.static-,$(CROSS)),.exe,)
INSTALL = install
LDFLAGS =
MSGFMT = msgfmt
MSGMERGE = msgmerge
PKG_CONFIG = $(CROSS)$(PKG_CONFIG_NATIVE)
PKG_CONFIG_NATIVE = pkg-config
POVRAY = povray
XGETTEXT = xgettext
GIMP = gimp

MSGID_BUGS_ADDRESS := roever@users.sf.net

.DELETE_ON_ERROR:

.PHONY: default
default: all

VERSION := $(shell cat src/version)
ALL_SOURCES := $(wildcard src/*)

PKGS += SDL2_mixer
PKGS += sdl2
PKGS += zlib

PKGS_NATIVE += zlib
PKGS_NATIVE += sdl2
PKGS_NATIVE += libpng
PKGS_NATIVE += SDL2_image

PKG_CFLAGS_NATIVE = $$($(PKG_CONFIG_NATIVE) --cflags $(PKGS_NATIVE))
PKG_LIBS_NATIVE = $$($(PKG_CONFIG_NATIVE) --libs $(PKGS_NATIVE))

DEFS += -DVERSION='"$(VERSION)"'
DEFS += -DTOP_DATADIR='"$(PKGDATADIR)"'
DEFS += -DHISCOREDIR='"$(STATEDIR)"'
DEFS += -DENABLE_NLS=1
DEFS += -DLOCALEDIR='"$(LOCALEDIR)"'

FILES_H := $(wildcard src/*.h)
FILES_CPP := $(wildcard src/*.cc)
FILES_O := $(patsubst src/%,_build/$(CROSS)/%.o,$(FILES_CPP))
.SECONDARY: $(FILES_O)
_build/$(CROSS)/%.o: src/% src/version $(FILES_H)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $$($(PKG_CONFIG) --cflags $(PKGS)) $(DEFS) -c -o $@ $<

FILES_BINDIR += $(CROSS)toppler$(EXEEXT)
$(CROSS)toppler$(EXEEXT): $(FILES_O)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(FILES_O) $$($(PKG_CONFIG) --libs $(PKGS)) $(LIBS)

# the rules for the datafile

SOUNDS = datafile/sounds/alarm.wav datafile/sounds/boing.wav datafile/sounds/bubbles.wav datafile/sounds/fanfare.wav \
         datafile/sounds/hit.wav datafile/sounds/honk.wav datafile/sounds/rumble.wav datafile/sounds/score.wav \
         datafile/sounds/sonar.wav datafile/sounds/splash.wav datafile/sounds/swoosh.wav datafile/sounds/tap.wav \
         datafile/sounds/tick.wav datafile/sounds/torpedo.wav datafile/sounds/water.wav

DATFILES += $(SOUNDS)

#-------------------------------------------------------#
# translation files
#-------------------------------------------------------#

TRANSLATIONFILES_PO := $(wildcard src/po/*.po)
TRANSLATIONFILES_MO := $(patsubst src/po/%.po,locale/%/LC_MESSAGES/toppler.mo,$(TRANSLATIONFILES_PO))
TRANSLATIONFILES_INST := $(patsubst src/po/%.po,$(DESTDIR)$(LOCALEDIR)/%/LC_MESSAGES/toppler.mo,$(TRANSLATIONFILES_PO))
FILES_BINDIR += $(TRANSLATIONFILES_MO)

locale/%/LC_MESSAGES/toppler.mo: src/po/%.po
	@mkdir -p $(dir $@)
	msgfmt $< -o $@

$(DESTDIR)$(LOCALEDIR)/%/LC_MESSAGES/toppler.mo: locale/%/LC_MESSAGES/toppler.mo
	$(INSTALL) -m755 -d $(dir $@)
	$(INSTALL) -m644 $< $@

#-------------------------------------------------------#
# rules to create the data files necesary for the cross #
#-------------------------------------------------------#
DATFILES += _build/cross.dat
.SECONDARY: _build/cross.dat
_build/cross.dat: _build/tools/cross _build/tools/assembler datafile/cross_pov/cross.pov datafile/cross_pov/cross.ini datafile/sprites_pov/environment.pov
	( cd _build && mkdir -p cross_pov )
	( cd _build/cross_pov && $(POVRAY) ../../datafile/cross_pov/cross.ini +L../../datafile/cross_pov -D 2>> ../pov.log )
	( cd _build && ./tools/assembler vm cross_rgb cross_pov/*.png )
	( cd _build && convert cross_rgb_mask.png -colors 256 PNG8:cross_mask.png )
	( cd _build && convert cross_rgb_colors.png -colors 256 PNG8:cross_colors.png )
	( cd _build && ./tools/cross )

#------------------------------------------------------#
# rules to create the data files necesary for the font #
#------------------------------------------------------#
DATFILES += _build/font.dat
.SECONDARY: _build/font.dat
_build/font.dat: _build/tools/font datafile/font.xcf
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/font.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"font_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/font.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 1) \"font_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && convert font_colors_rgb.png -colors 256 PNG8:font_colors.png )
	( cd _build && convert font_mask_rgb.png -colors 256 PNG8:font_mask.png )
	( cd _build && ./tools/font )

#----------------------------------------------------------#
# rules to create the data files necesary for the graphics #
#----------------------------------------------------------#
DATFILES += _build/graphics.dat
.SECONDARY: _build/graphics.dat
_build/graphics.dat: _build/tools/graphics datafile/graphics_brick.xcf
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/graphics_brick.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"graphics_brick.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/graphics_pinacle.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"graphics_pinacle.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && ./tools/graphics )

#-------------------------------------------------------#
# rules to create the data files necesary for the mennu #
#-------------------------------------------------------#
DATFILES += _build/menu.dat
MENULEVELS_DIR=datafile/levels/mission1
MENULEVELS=$(wildcard ${MENULEVELS_DIR}/*)
.SECONDARY: _build/menu.dat
_build/menu.dat: _build/tools/menu _build/tools/tower2inc datafile/menu_pov/menu.ini datafile/menu_pov/menu.pov ${MENULEVELS}
	( cd _build && mkdir -p menu_pov )
	( cd _build/menu_pov && ../tools/tower2inc "turm%i.inc" ../.. $(MENULEVELS) )
	( cd _build/menu_pov && $(POVRAY) ../../datafile/menu_pov/menu.ini +L../../datafile/menu_pov -D 2>> ../pov.log )
	( cd _build && convert menu_pov/menu_rgb.png -colors 256 PNG8:menu.png )
	( cd _build && ./tools/menu )

#----------------------------------------------------------#
# rules to create the data files necesary for the scroller #
#----------------------------------------------------------#
DATFILES += _build/scroller.dat
.SECONDARY: _build/scroller.dat
_build/scroller.dat: _build/tools/scroller datafile/scroller.xcf
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"scr1_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 1) \"scr2_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 2) \"scr3_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 3) \"scr1_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 4) \"scr2_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/scroller.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 5) \"scr3_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && ./tools/assembler hm scr1_rgb scr1_colors_rgb.png scr1_mask_rgb.png )
	( cd _build && ./tools/assembler hm scr2_rgb scr2_colors_rgb.png scr2_mask_rgb.png )
	( cd _build && ./tools/assembler hm scr3_rgb scr3_colors_rgb.png scr3_mask_rgb.png )
	( cd _build && convert scr3_rgb_mask.png -colors 256 PNG8:scroller3_mask.png )
	( cd _build && convert scr2_rgb_mask.png -colors 256 PNG8:scroller2_mask.png )
	( cd _build && convert scr1_rgb_mask.png -colors 256 PNG8:scroller1_mask.png )
	( cd _build && convert scr3_rgb_colors.png -colors 256 PNG8:scroller3_colors.png )
	( cd _build && convert scr2_rgb_colors.png -colors 256 PNG8:scroller2_colors.png )
	( cd _build && convert scr1_rgb_colors.png -colors 256 PNG8:scroller1_colors.png )
	( cd _build && ./tools/scroller 3 2 1/1 \
		       scroller3_colors.png scroller3_mask.png 0/0/640 1/2 \
                       scroller2_colors.png scroller2_mask.png 0/0/640 1/1 \
                       scroller1_colors.png scroller1_mask.png 0/0/640 2/1 )

#---------------------------------------------------------#
# rules to create the data files necesary for the sprites #
#---------------------------------------------------------#

.SECONDARY: _build/sprites_pov/robot0_rgb_colors.png _build/sprites_pov/robot0_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot1_rgb_colors.png _build/sprites_pov/robot1_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot2_rgb_colors.png _build/sprites_pov/robot2_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot3_rgb_colors.png _build/sprites_pov/robot3_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot4_rgb_colors.png _build/sprites_pov/robot4_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot5_rgb_colors.png _build/sprites_pov/robot5_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot6_rgb_colors.png _build/sprites_pov/robot6_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot7_rgb_colors.png _build/sprites_pov/robot7_rgb_mask.png
.SECONDARY: _build/sprites_pov/robot8_rgb_colors.png _build/sprites_pov/robot8_rgb_mask.png
_build/sprites_pov/robot%_rgb_colors.png _build/sprites_pov/robot%_rgb_mask.png: datafile/sprites_pov/environment.pov \
             datafile/sprites_pov/robot%/obj.pov datafile/sprites_pov/robot%/obj.ini \
             _build/tools/assembler
	( cd _build && mkdir -p sprites_pov  )
	( cd _build/sprites_pov && mkdir -p robot$* && cd robot$* && $(POVRAY) ../../../datafile/sprites_pov/robot$*/obj.ini +L../../../datafile/sprites_pov/robot$* -D 2>> ../pov.log )
	( cd _build && ./tools/assembler hm sprites_pov/robot$*_rgb sprites_pov/robot$*/*.png )

DATFILES += _build/sprites.dat
.SECONDARY: _build/sprites.dat
_build/sprites.dat: _build/tools/sprites _build/tools/assembler \
             datafile/sprites_pov/box/obj.pov datafile/sprites_pov/box/obj.ini \
             datafile/sprites_pov/balls/obj.pov datafile/sprites_pov/balls/obj.ini \
             datafile/sprites_pov/snowball/obj.pov datafile/sprites_pov/snowball/obj.ini \
             _build/sprites_pov/robot0_rgb_colors.png _build/sprites_pov/robot0_rgb_mask.png \
             _build/sprites_pov/robot1_rgb_colors.png _build/sprites_pov/robot1_rgb_mask.png \
             _build/sprites_pov/robot2_rgb_colors.png _build/sprites_pov/robot2_rgb_mask.png \
             _build/sprites_pov/robot3_rgb_colors.png _build/sprites_pov/robot3_rgb_mask.png \
             _build/sprites_pov/robot4_rgb_colors.png _build/sprites_pov/robot4_rgb_mask.png \
             _build/sprites_pov/robot5_rgb_colors.png _build/sprites_pov/robot5_rgb_mask.png \
             _build/sprites_pov/robot6_rgb_colors.png _build/sprites_pov/robot6_rgb_mask.png \
             _build/sprites_pov/robot7_rgb_colors.png _build/sprites_pov/robot7_rgb_mask.png \
             datafile/sprites_pov/environment.pov \
             $(wildcard datafile/fish/render/*.tga) \
             $(wildcard datafile/submarine/render/*.tga) \
	     datafile/sprites_torpedo_colors.png datafile/sprites_torpedo_mask.png
	( cd _build && mkdir -p sprites_pov  )
	( cd _build && ./tools/assembler v sprites_robots_rgb sprites_pov/robot*_rgb_colors.png sprites_pov/robot*_rgb_mask.png )
	( cd _build && mv sprites_robots_rgb_colors.png sprites_robots_colors_rgb.png )
	( cd _build && mv sprites_robots_rgb_mask.png sprites_robots_mask_rgb.png )
	( cd _build/sprites_pov && mkdir -p snowball && cd snowball && $(POVRAY) ../../../datafile/sprites_pov/snowball/obj.ini  +L../../../datafile/sprites_pov/snowball -D 2>> ../pov.log )
	( cd _build/sprites_pov && mkdir -p balls && cd balls && $(POVRAY) ../../../datafile/sprites_pov/balls/obj.ini  +L../../../datafile/sprites_pov/balls -D 2>> ../pov.log )
	( cd _build/sprites_pov && mkdir -p box && cd box && $(POVRAY) ../../../datafile/sprites_pov/box/obj.ini +L../../../datafile/sprites_pov/box -D 2>> ../pov.log )
	( cd _build && ./tools/assembler hm sprites_balls_rgb sprites_pov/balls/obj*.png )
	( cd _build && ./tools/assembler hm sprites_box_rgb sprites_pov/box/obj*.png )
	( cd _build && convert sprites_pov/snowball/obj1.png -colors 256 PNG8:sprites_snowball_mask.png )
	( cd _build && convert sprites_pov/snowball/obj0.png -colors 256 PNG8:sprites_snowball_colors.png )
	( cd _build && convert sprites_box_rgb_mask.png -colors 256 PNG8:sprites_box_mask.png )
	( cd _build && convert sprites_box_rgb_colors.png -colors 256 PNG8:sprites_box_colors.png )
	( cd _build && convert sprites_balls_rgb_mask.png -colors 256 PNG8:sprites_balls_mask.png )
	( cd _build && convert sprites_balls_rgb_colors.png -colors 256 PNG8:sprites_balls_colors.png )
	( cd _build && convert sprites_robots_mask_rgb.png -colors 256 PNG8:sprites_robots_mask.png )
	( cd _build && convert sprites_robots_colors_rgb.png -colors 256 PNG8:sprites_robots_colors.png )
	( mkdir -p _build/fish/render && cp datafile/fish/render/* _build/fish/render )
	( mkdir -p _build/submarine/render && cp datafile/submarine/render/* _build/submarine/render )
	( cp datafile/sprites_torpedo_* _build )
	( cd _build && ./tools/sprites )

#-------------------------------------------------------#
# rules to create the data files necesary for the title #
#-------------------------------------------------------#
DATFILES += _build/titles.dat
.SECONDARY: _build/titles.dat
_build/titles.dat: _build/tools/titles datafile/titles.xcf
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/titles.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"titles_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/titles.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 1) \"titles_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && convert titles_colors_rgb.png -colors 256 PNG8:titles_colors.png )
	( cd _build && convert titles_mask_rgb.png -colors 256 PNG8:titles_mask.png )
	( cd _build && ./tools/titles )

#-------------------------------------------------------#
# rules to create the data files necesary for the dude  #
#-------------------------------------------------------#
DATFILES += _build/dude.dat
.SECONDARY: _build/dude.dat
_build/dude.dat: _build/tools/dude datafile/dude.xcf
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/dude.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 1) \"dude_colors_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && $(GIMP) -i -b "(let* ((image (car(gimp-xcf-load 1 \"../datafile/dude.xcf\" \"ttt\")))(layers (cadr(gimp-image-get-layers image))))(file-png-save-defaults 1 image (aref layers 0) \"dude_mask_rgb.png\" \"ttt\")(gimp-quit 1))" )
	( cd _build && convert dude_colors_rgb.png -colors 256 PNG8:dude_colors.png )
	( cd _build && convert dude_mask_rgb.png -colors 256 PNG8:dude_mask.png )
	( cd _build && ./tools/dude )

#-------------------------------------------------------#
# rules to create the missions                          #
#-------------------------------------------------------#

# RULE FOR MISSION 1

MISSIONFILES += /m1
.SECONDARY: _build/m1.ttm _build/nms_m1.txt
_build/m1.ttm _build/nms_m1.txt: _build/tools/cremission \
	datafile/levels/mission1/m1t1 datafile/levels/mission1/m1t2 datafile/levels/mission1/m1t3 datafile/levels/mission1/m1t4 datafile/levels/mission1/m1t5 datafile/levels/mission1/m1t6 datafile/levels/mission1/m1t7 datafile/levels/mission1/m1t8
	( cd _build && tools/cremission "Mission 1" m1 10 nms_m1.txt \
		../datafile/levels/mission1/m1t1 \
		../datafile/levels/mission1/m1t2 \
		../datafile/levels/mission1/m1t3 \
		../datafile/levels/mission1/m1t4 \
		../datafile/levels/mission1/m1t5 \
		../datafile/levels/mission1/m1t6 \
		../datafile/levels/mission1/m1t7 \
		../datafile/levels/mission1/m1t8 )

# RULE FOR MISSION 2

MISSIONFILES += /m2
.SECONDARY: _build/m2.ttm _build/nms_m2.txt
_build/m2.ttm _build/nms_m2.txt: _build/tools/cremission \
	datafile/levels/mission2/m2t1 datafile/levels/mission2/m2t2 datafile/levels/mission2/m2t3 datafile/levels/mission2/m2t4 datafile/levels/mission2/m2t5 datafile/levels/mission2/m2t6 datafile/levels/mission2/m2t7 datafile/levels/mission2/m2t8
	( cd _build && tools/cremission "Mission 2" m2 11 nms_m2.txt \
		../datafile/levels/mission2/m2t1 \
		../datafile/levels/mission2/m2t2 \
		../datafile/levels/mission2/m2t3 \
		../datafile/levels/mission2/m2t4 \
		../datafile/levels/mission2/m2t5 \
		../datafile/levels/mission2/m2t6 \
		../datafile/levels/mission2/m2t7 \
		../datafile/levels/mission2/m2t8 )

# RULE FOR CLARENCE MISSION 1

MISSIONFILES += /ball1
.SECONDARY: _build/ball1.ttm _build/nms_ball1.txt
_build/ball1.ttm _build/nms_ball1.txt: _build/tools/cremission \
	datafile/levels/ball1/lev1 datafile/levels/ball1/lev2 datafile/levels/ball1/lev3 datafile/levels/ball1/lev4 datafile/levels/ball1/lev5 datafile/levels/ball1/lev6 datafile/levels/ball1/lev7 datafile/levels/ball1/lev8
	( cd _build && tools/cremission "Ball 1" ball1 13 nms_ball1.txt \
		../datafile/levels/ball1/lev1 \
		../datafile/levels/ball1/lev2 \
		../datafile/levels/ball1/lev3 \
		../datafile/levels/ball1/lev4 \
		../datafile/levels/ball1/lev5 \
		../datafile/levels/ball1/lev6 \
		../datafile/levels/ball1/lev7 \
		../datafile/levels/ball1/lev8 )

# RULE FOR CLARENCE MISSION 2

MISSIONFILES += /ball2
.SECONDARY: _build/ball2.ttm _build/nms_ball2.txt
_build/ball2.ttm _build/nms_ball2.txt: _build/tools/cremission \
	datafile/levels/ball2/lev1 datafile/levels/ball2/lev2 datafile/levels/ball2/lev3 datafile/levels/ball2/lev4 datafile/levels/ball2/lev5 datafile/levels/ball2/lev6 datafile/levels/ball2/lev7 datafile/levels/ball2/lev8
	( cd _build && tools/cremission "Ball 2" ball2 14 nms_ball2.txt \
		../datafile/levels/ball2/lev1 \
		../datafile/levels/ball2/lev2 \
		../datafile/levels/ball2/lev3 \
		../datafile/levels/ball2/lev4 \
		../datafile/levels/ball2/lev5 \
		../datafile/levels/ball2/lev6 \
		../datafile/levels/ball2/lev7 \
		../datafile/levels/ball2/lev8 )

# RULE FOR CLARENCE MISSION 3

MISSIONFILES += /ball3
.SECONDARY: _build/ball3.ttm _build/nms_ball3.txt
_build/ball3.ttm _build/nms_ball3.txt: _build/tools/cremission \
	datafile/levels/ball3/lev1 datafile/levels/ball3/lev2 datafile/levels/ball3/lev3 datafile/levels/ball3/lev4 datafile/levels/ball3/lev5 datafile/levels/ball3/lev6 datafile/levels/ball3/lev7 datafile/levels/ball3/lev8
	( cd _build && tools/cremission "Ball 3" ball3 15 nms_ball3.txt \
		../datafile/levels/ball3/lev1 \
		../datafile/levels/ball3/lev2 \
		../datafile/levels/ball3/lev3 \
		../datafile/levels/ball3/lev4 \
		../datafile/levels/ball3/lev5 \
		../datafile/levels/ball3/lev6 \
		../datafile/levels/ball3/lev7 \
		../datafile/levels/ball3/lev8 )

# RULE FOR PASIS MISSION 1

MISSIONFILES += /abc
.SECONDARY: _build/abc.ttm _build/nms_abc.txt
_build/abc.ttm _build/nms_abc.txt: _build/tools/cremission \
	datafile/levels/kallinen1/t1 datafile/levels/kallinen1/t2 datafile/levels/kallinen1/t3 datafile/levels/kallinen1/t4 datafile/levels/kallinen1/t5 datafile/levels/kallinen1/t6 datafile/levels/kallinen1/t7 datafile/levels/kallinen1/t8
	( cd _build && tools/cremission "ABC Towers" abc 12 nms_abc.txt \
		../datafile/levels/kallinen1/t1 \
		../datafile/levels/kallinen1/t2 \
		../datafile/levels/kallinen1/t3 \
		../datafile/levels/kallinen1/t4 \
		../datafile/levels/kallinen1/t5 \
		../datafile/levels/kallinen1/t6 \
		../datafile/levels/kallinen1/t7 \
		../datafile/levels/kallinen1/t8 )

# RULE FOR PASIS MISSION 2

MISSIONFILES += /pasi2
.SECONDARY: _build/pasi2.ttm _build/nms_pasi2.txt
_build/pasi2.ttm _build/nms_pasi2.txt: _build/tools/cremission \
	datafile/levels/kallinen2/t1 datafile/levels/kallinen2/t2
	( cd _build && tools/cremission "Pasis 2 Towers" pasi2 12 nms_pasi2.txt \
		../datafile/levels/kallinen2/t1 \
		../datafile/levels/kallinen2/t2 )

# RULE FOR DAVIDS MISSION 1

MISSIONFILES += /david1
_build/david1.ttm _build/nms_david1.txt: _build/tools/cremission \
	datafile/levels/david1/lev1 datafile/levels/david1/lev2 datafile/levels/david1/lev3 datafile/levels/david1/lev4 datafile/levels/david1/lev5 datafile/levels/david1/lev6 datafile/levels/david1/lev7 datafile/levels/david1/lev8
	( cd _build && tools/cremission "Challenge 1" david1 12 nms_david1.txt \
		../datafile/levels/david1/lev1 \
		../datafile/levels/david1/lev2 \
		../datafile/levels/david1/lev3 \
		../datafile/levels/david1/lev4 \
		../datafile/levels/david1/lev5 \
		../datafile/levels/david1/lev6 \
		../datafile/levels/david1/lev7 \
		../datafile/levels/david1/lev8 )

# RULE FOR DAVIDS MISSION 2

MISSIONFILES += /david2
_build/david2.ttm _build/nms_david2.txt: _build/tools/cremission \
	datafile/levels/david2/l1 datafile/levels/david2/l2 datafile/levels/david2/l3 datafile/levels/david2/l4
	( cd _build && tools/cremission "Challenge 2" david2 12 nms_david2.txt \
		../datafile/levels/david2/l1 \
		../datafile/levels/david2/l2 \
		../datafile/levels/david2/l3 \
		../datafile/levels/david2/l4 )

# the automatic stuff for the mission files
# create mission build files
MISSIONBUILTFILES = $(patsubst /%,_build/%.ttm,$(MISSIONFILES))
.SECONDARY: $(MISSIONBUILTFILES)
DATFILES += $(MISSIONBUILTFILES)

# create the names files
MISSIONNAMESFILES = $(patsubst /%,_build/nms_%.txt,$(MISSIONFILES))

#-------------------------------------------------------#
# rules to create the tool programs                     #
#-------------------------------------------------------#
.SECONDARY: _build/tools/assembler
_build/tools/assembler: datafile/assembler.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/crearc
_build/tools/crearc: datafile/crearc.cpp
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/dude
_build/tools/dude: datafile/dude.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/titles
_build/tools/titles: datafile/titles.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/sprites
_build/tools/sprites: datafile/sprites.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/scroller
_build/tools/scroller: datafile/scroller.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/font
_build/tools/font: datafile/font.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/cross
_build/tools/cross: datafile/cross.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/graphics
_build/tools/graphics: datafile/graphics.c datafile/colorreduction.h datafile/pngsaver.h
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/menu
_build/tools/menu: datafile/menu.c
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/tower2inc
_build/tools/tower2inc: datafile/menu_pov/tower2inc.cc
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -g -Isrc -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

.SECONDARY: _build/tools/cremission
_build/tools/cremission: datafile/levels/cremission.cc
	@mkdir -p $(dir $@)
	$(CXX_NATIVE) $(CXXFLAGS) -g -Isrc -o $@ $< $(PKG_CFLAGS_NATIVE) $(PKG_LIBS_NATIVE)

#---------------------------------------------------------------------------------------#
# global make, this creates the final data file packing all things togethers using zlib #
#---------------------------------------------------------------------------------------#
FILES_BINDIR += toppler.dat
toppler.dat: _build/tools/crearc $(DATFILES)
	./_build/tools/crearc toppler.dat $(DATFILES)


.PHONY: clean
clean:
	rm -rf _build
	rm toppler


.PHONY: distclean
distclean:
	rm -rf _build
	rm toppler
	rm toppler.dat


.PHONY: all
all: $(FILES_BINDIR)


.PHONY: translation
translation: $(TRANSLATIONFILES_PO) _build/toppler.pot

_build/toppler.pot: $(FILES_CPP) $(MISSIONNAMESFILES)
	xgettext --c++ -o - -k_ -kN_ $(FILES_CPP) $(MISSIONNAMESFILES) > _build/toppler.pot

src/po/%.po: _build/toppler.pot
	msgmerge -U $@ _build/toppler.pot


#---------------------------------------------------------------------------------------#
# packaging and installation stuff                                                      #
#---------------------------------------------------------------------------------------#

# TODO dist and windist..

.PHONY: install
install: toppler.dat toppler $(TRANSLATIONFILES_INST)
	$(INSTALL_DIR) $(DESTDIR)$(PKGDATADIR)
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR)
	$(INSTALL_DIR) $(DESTDIR)$(MANDIR)/man6
	$(INSTALL_DATA) toppler.dat $(DESTDIR)$(PKGDATADIR)/toppler.dat
	$(INSTALL_DATA) toppler.ogg $(DESTDIR)$(PKGDATADIR)/toppler.ogg
	$(INSTALL_PROGRAM) toppler $(DESTDIR)$(BINDIR)/toppler
	$(INSTALL_DATA) dist/toppler.6 $(DESTDIR)$(MANDIR)/man6/toppler.6
