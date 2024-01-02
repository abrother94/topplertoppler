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

#include "archi.h"
#include "decl.h"

#include <zlib.h>

archive::archive(FILE *stream) : f(stream)
{
    size_t read = 0;

    assert_msg(f, "Data file not found");

    uint8_t filecount;

    /* find out the number of files inside the archive
     * alloce the neccessary memory
     */
    read = fread(&filecount, 1, 1, f);
    assert_msg(read == 1, "failed to read data");
    files.resize(filecount);

    /* read the information for each file */
    for (auto & file : files)
    {
        while (true)
        {
            char c;
            read = fread(&c, 1, 1, f);
            assert_msg(read == 1, "failed to read data");

            if (!c) break;

            file.name += c;
        }

        assert_msg(read == 1, "failed to read data");

        uint8_t buf[4*3];

        read = fread(buf, 1, 4*3, f);
        assert_msg(read == 3*4, "failed to read data");

        file.start    = buf[0] + (buf[1] << 8) + (buf[ 2] << 16) + (buf[ 3] << 24);
        file.size     = buf[4] + (buf[5] << 8) + (buf[ 6] << 16) + (buf[ 7] << 24);
        file.compress = buf[8] + (buf[9] << 8) + (buf[10] << 16) + (buf[11] << 24);
    }
}

archive::~archive()
{
    fclose(f);
}

archive::file archive::open(const std::string & name)
{
    for (auto & ff : files)
    {
        if (ff.name == name)
        {
            /* allocate buffer for compressed data */
            auto b = std::make_unique<Uint8[]>(ff.compress);

            /* allocate buffer for uncompressed data */
            auto buffer = std::make_unique<Uint8[]>(ff.size);


            /* read the compressed data */
            fseek(f, ff.start, SEEK_SET);
            size_t read = fread(b.get(), 1, ff.compress, f);
            assert_msg(read == ff.compress, "failed to read data");

            /* decompress it and check results */
            uLongf fsize = ff.size;
            assert_msg(uncompress(buffer.get(), &fsize, b.get(), ff.compress) == Z_OK, "Decompression problem, data file corrupt?");
            assert_msg(fsize == ff.size, "Data file corrupt.");

            return file(std::move(buffer), ff.size);
        }
    }

    /* if we arrive here we couldn't find the file we looked for */
    assert_msg(0, "File not found in archive!");
}

Uint32 archive::file::read(void *buf, Uint32 size)
{
    if (bufferpos + size > fsize)
        size = fsize - bufferpos;

    memcpy(buf, &buffer[bufferpos], size);
    bufferpos += size;

    return size;
}

Uint16 archive::file::getword(void)
{
    if (bufferpos + 2 >= fsize)
        return 0;

    Uint16 w = (Uint16)buffer[bufferpos] + ((Uint16)buffer[bufferpos+1] << 8);
    bufferpos+=2;

    return w;
}

SDL_RWops * archive::file::rwOps(void) {
    return SDL_RWFromMem(buffer.get(), fsize);
}


std::unique_ptr<archive> dataarchive;
