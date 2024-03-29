#include <png.h>
#include <stdlib.h>

Uint32 getPNGpixel(SDL_Surface * surface, int x, int y)
{
    int bpp;
    Uint8 * p;
    Uint32 pixel;

    pixel = 0;

    bpp = surface->format->BytesPerPixel;
    p = (Uint8 *) (((Uint8 *)surface->pixels) +
		   (y * surface->pitch) +
		   (x * bpp));

    if (bpp == 1)
      pixel = *p;
    else if (bpp == 2)
      pixel = *(Uint16 *)p;
    else if (bpp == 3)
	{
	    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
	      pixel = p[0] << 16 | p[1] << 8 | p[2];
	    else
	      pixel = p[0] | p[1] << 8 | p[2] << 16;
	}
    else if (bpp == 4)
      pixel = *(Uint32 *)p;

    return pixel;
}


int SavePNGImage(char * fname, SDL_Surface * surf)
{
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char ** png_rows;
    Uint8 r, g, b;
    int x, y;

    FILE *fi = fopen(fname, "wb");
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL)
        {
	    fclose(fi);
	    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	    fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	}
    else
        {
	    info_ptr = png_create_info_struct(png_ptr);
	    if (info_ptr == NULL)
		{
		    fclose(fi);
		    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		    fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
		}
	    else
		{
		    if (setjmp(png_jmpbuf(png_ptr)))
			{
			    fclose(fi);
			    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
			    fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
			    return 0;
			}
		    else
			{
			    png_init_io(png_ptr, fi);
                            png_set_IHDR(png_ptr, info_ptr,
                                surf->w, surf->h,
                                8,  // bit depth
			       (surf->format->BytesPerPixel == 1) ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB,
                               PNG_INTERLACE_NONE,
                               PNG_COMPRESSION_TYPE_DEFAULT,
                               PNG_FILTER_TYPE_DEFAULT
                               );

			    if (surf->format->BytesPerPixel == 1) {

				int t;
				png_colorp palette = (png_colorp)(malloc(sizeof(png_color) * 256));
				for (t = 0; t < 256; t++) {
				    palette[t].red = surf->format->palette->colors[t].r;
				    palette[t].green = surf->format->palette->colors[t].g;
				    palette[t].blue = surf->format->palette->colors[t].b;
				}

                                png_set_PLTE(png_ptr, info_ptr, palette, 256);
                            }

			    png_write_info(png_ptr, info_ptr);

			    /* Save the picture: */

			    png_rows = (unsigned char **)(malloc(sizeof(char *) * surf->h));
			    for (y = 0; y < surf->h; y++)
				{
				    if (surf->format->BytesPerPixel == 1)
				      png_rows[y] = (unsigned char*)malloc(sizeof(char) * surf->w);
				    else
				      png_rows[y] = (unsigned char*)malloc(sizeof(char) * 3 * surf->w);

				    if (surf->format->BytesPerPixel == 1) {
					memcpy(png_rows[y], &((Uint8*)surf->pixels)[y*surf->pitch], surf->w);
				    } else {
					for (x = 0; x < surf->w; x++)
					    {
						SDL_GetRGB(getPNGpixel(surf, x, y), surf->format, &r, &g, &b);

						png_rows[y][x * 3 + 0] = r;
						png_rows[y][x * 3 + 1] = g;
						png_rows[y][x * 3 + 2] = b;
					    }
				    }
				}

			    png_write_image(png_ptr, png_rows);

			    for (y = 0; y < surf->h; y++)
			      free(png_rows[y]);

			    free(png_rows);

			    png_write_end(png_ptr, NULL);

			    png_destroy_write_struct(&png_ptr, &info_ptr);
			    fclose(fi);

			    return 1;
			}
		}
	}

    return 0;
}

