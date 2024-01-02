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

#include "txtsys.h"

#include "screen.h"
#include "keyb.h"
#include "menu.h"

textsystem::textsystem(std::string t, menuopt_callback_proc pr)
{
    title = std::move(t);

    max_length = 0;
    xoffs = yoffs = disp_xoffs = disp_yoffs = 0;

    mproc = pr;

    ystart = (!title.empty()) ? FONTHEI + 15 : 0;

    shownlines = ((SCREENHEI - ystart) / FONTHEI) + 1;
}

void textsystem::addline(std::string line)
{
    lines.push_back(line);
    max_length = std::max(scr_formattextlength(0,line), max_length);
}

void textsystem::run()
{
    bool ende = false;
    SDL_Keycode key = SDLK_UNKNOWN;

    do {
        (void)key_readkey();

        draw();

        key = key_sdlkey();

        switch (key_sdlkey2conv(key, false))
        {
            case up_key:
                if (yoffs >= FONTHEI) yoffs -= FONTHEI;
                else yoffs = 0;
                break;

            case down_key:
                if (yoffs + (shownlines*FONTHEI) < ((long)lines.size()*FONTHEI)) yoffs += FONTHEI;
                else yoffs = (lines.size() - shownlines+1)*FONTHEI;
                break;

            case break_key:
                ende = true;
                break;

            case left_key:
                if (xoffs >= FONTWID) xoffs -= FONTWID;
                else xoffs = 0;
                break;

            case right_key:
                if (xoffs <= max_length-SCREENWID-FONTWID) xoffs += FONTWID;
                else xoffs = max_length-SCREENWID;
                break;

            default:
                switch (key) {
                    case SDLK_PAGEUP:
                        if (yoffs >= shownlines*FONTHEI) yoffs -= shownlines*FONTHEI;
                        else yoffs = 0;
                        break;
                    case SDLK_SPACE:
                    case SDLK_PAGEDOWN:
                        if ((yoffs/FONTHEI) + (shownlines*2) <= (int)lines.size())
                            yoffs += shownlines*FONTHEI;
                        else yoffs = (lines.size() - shownlines+1)*FONTHEI;
                        break;
                    case SDLK_HOME:
                        yoffs = 0;
                        break;
                    case SDLK_END:
                        yoffs = (lines.size() - shownlines+1)*FONTHEI;
                        break;
                    case SDLK_RETURN:
                    case SDLK_ESCAPE:
                        ende = true;
                        break;
                    default: break;
                }
        }

    } while (!ende);
}

void textsystem::draw()
{
    static const char pointup[2] = { fontptrup, 0 };
    static const char pointdown[2] = { fontptrdown, 0 };
    static const char pointleft[2] = { fontptrleft, 0 };
    static const char pointright[2] = { fontptrright, 0 };

    if (mproc)
        (*mproc) (NULL);

    scr_writetext_center(5, title);

    // smoothly move the actual position to the target position
    if (disp_yoffs < yoffs)
    {
        disp_yoffs += ((yoffs - disp_yoffs+3) / 4)+1;
        if (disp_yoffs > yoffs) disp_yoffs = yoffs;
    } else if (disp_yoffs > yoffs)
    {
        disp_yoffs -= ((disp_yoffs - yoffs+3) / 4)+1;
        if (disp_yoffs < yoffs) disp_yoffs = yoffs;
    }

    if (disp_xoffs < xoffs)
    {
        disp_xoffs += ((xoffs - disp_xoffs) / 4)+1;
        if (disp_xoffs > xoffs) disp_xoffs = xoffs;
    } else if (disp_xoffs > xoffs)
    {
        disp_xoffs -= ((disp_xoffs - xoffs) / 4)+1;
        if (disp_xoffs < xoffs) disp_xoffs = xoffs;
    }

    scr_setclipping(0, ystart, SCREENWID, SCREENHEI);

    // draw the text lines
    for (int k = 0; k <= shownlines; k++)
        if (k+(disp_yoffs / FONTHEI) < (int)lines.size())
            if (!lines[k+(disp_yoffs / FONTHEI)].empty())
                scr_writeformattext(-disp_xoffs,
                        k*FONTHEI + ystart - (disp_yoffs % FONTHEI),
                        lines[k+(disp_yoffs / FONTHEI)]);

    scr_setclipping();

    // draw arrows
    if (disp_yoffs > 0)
        scr_writetext(SCREENWID-FONTWID, 34, pointup);

    if ((disp_yoffs / FONTHEI) + shownlines < (int)lines.size())
        scr_writetext(SCREENWID-FONTWID, SCREENHEI-FONTHEI, pointdown);

    if (disp_xoffs > 0)
        scr_writetext(FONTWID, 5, pointleft);

    if (disp_xoffs < max_length - SCREENWID)
        scr_writetext(SCREENWID-FONTWID, 5, pointright);

    scr_swap();
    dcl_wait();
}
