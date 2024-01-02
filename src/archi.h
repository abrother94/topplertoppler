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

#ifndef ARCHI_H
#define ARCHI_H

#include <SDL.h>

#include <vector>
#include <string>
#include <memory>

#include <cstdio>

/* this module contains a simple archive access class. an archive
 * is a collection of zlib compressed files with a header defining
 * compressed, uncompressed size and the start of the data inside
 * the archive
 *
 * because the file gets decompressed in one run and saved inside a
 * memory block be careful not to create too big files
 */

/* this class handles one archive, each archive can contain any number of
 * files with 0 terminated names.
 */
class archive {

    public:

        /* this class is used to handle the access to the different files inside
         * the archive(s)
         */
        class file {

            public:

                /* you need to give the archive with your file and
                 * the name of the file you want to open to the
                 * constructor
                 */
                file(std::unique_ptr<Uint8[]> && b, Uint32 s) : buffer(std::move(b)), bufferpos(0), fsize(s) {}

                /* returns the size of the currently opened file
                */
                Uint32 size(void) { return fsize; }

                /* returns true if the current file is completely read
                */
                bool eof(void) { return bufferpos >= fsize; }

                /* reads up to size bytes into the buffer, returning in result
                 * the real number read
                 */
                Uint32 read(void *buf, Uint32 size);

                /* read one byte from currently opened file
                */
                Uint8 getbyte(void) { return buffer[bufferpos++]; }

                /* read one word from currently opened file this read is endian save
                */
                Uint16 getword(void);

                /* creates an RW operation type from this file */
                SDL_RWops *rwOps(void);

            private:

                /* the buffer containing the uncompressed file data
                */
                std::unique_ptr<Uint8[]> buffer;

                /* current position inside the file data
                */
                Uint32 bufferpos;

                /* size of the file
                */
                unsigned long fsize;

        };

        /* opens the archive, you must give the FILE handle to the
         * file that is the archive to this constructor
         */
        archive(FILE *file);

        ~archive();

        file open(const std::string & name);

        const auto & filelist() const { return files; }

    private:

        FILE *f;

        /* this structur contains all the information inside the
         * header for one file, it's used build an array of
         * files upon archive opening
         */
        struct fileindex
        {
            std::string name;
            Uint32 start, size, compress;
        };

        /* the pointer to the file array
        */
        std::vector<fileindex> files;

};

/* it is arguably, if this is the right place for this declaration, but
 * if you assume that everybody who wants to access an archive does need the
 * class definition and this global variable at the same time, it's not so wrong
 * either
 */
extern std::unique_ptr<archive> dataarchive;

#endif
