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

#ifndef TXTSYS_H
#define TXTSYS_H

#include "menusys.h"

#include <string>
#include <vector>

class textsystem {

    public:
        textsystem(std::string title, menuopt_callback_proc pr);
        ~textsystem() {}
        void addline(std::string line);
        void run();

    private:
        void draw();

        std::string title;
        int  shownlines;                // # of lines shown on screen
        int  ystart;                    // screen y coord where text starts
        long max_length;                // how long is the longest line?
        std::vector<std::string> lines;
        menuopt_callback_proc mproc;    // background drawing proc
        long xoffs;                     // current x offset
        long yoffs;                     // current y offset
        long disp_xoffs;                // displayed x offset
        long disp_yoffs;                // displayed y offset
};

#endif /* TXTSYS_H */
