TOPPLERDIR = ../../toppler

LEVELDIR = ../../levels/mission1
#LEVELDIR = ../../levels/mission2
#LEVELDIR = ../../levels/kallinen1
#LEVELDIR = ../../levels/ball1
#LEVELDIR = ../../levels/ball2
#LEVELDIR = ../../levels/ball3

LEVELS = `find $(LEVELDIR) -maxdepth 1 -type f`

OBJS = $(TOPPLERDIR)/decl.o

all: tower2inc
	( ln -s `pwd`/$(LEVELDIR) $(HOME)/.toppler/t && \
	./tower2inc "turm%i.inc" t/ $(LEVELS) && \
	rm $(HOME)/.toppler/t && \
	povray menu.ini )

tower2inc: tower2inc.cc
	g++ tower2inc.cc -I$(TOPPLERDIR) -I/usr/include/SDL $(OBJS) -lSDL -lz -o tower2inc

clean:
	rm -f tower2inc turm*.inc menu_rgb.png
