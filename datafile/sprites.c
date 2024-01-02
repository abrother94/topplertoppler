#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>

#include <math.h>

#include "pngsaver.h"
#include "colorreduction.h"

Uint8 get_color(SDL_Surface *s, int x, int y) {
  return ((Uint8*)s->pixels)[y*s->pitch+x];
}

Uint8 get_alpha(SDL_Surface *s, int x, int y) {
  return s->format->palette->colors[((Uint8*)s->pixels)[y*s->pitch+x]].r;
}

void write_palette(FILE *out, SDL_Surface *s) {
  int i;
  Uint8 c = s->format->palette->ncolors - 1;

  fwrite(&c, 1, 1, out);

  for (i = 0; i < s->format->palette->ncolors; i++) {
    fwrite(&s->format->palette->colors[i].r, 1, 1, out);
    fwrite(&s->format->palette->colors[i].g, 1, 1, out);
    fwrite(&s->format->palette->colors[i].b, 1, 1, out);
  }
}


/*
 * rensers a curve with the following property:
 *
 * x(t) = (cos(t))^n
 * y(t) = (sin(t))^n
 *
 * t(x) = acos(x ^ (1/n))
 * y(x) = (sin(acos(x^(1/n))))^n  (-1 <= x <= 1)
 *
 */


void render_star(SDL_Surface *s, Uint16 size, double power) {
  Uint16 x, y, t, z;
  double alias;

  Uint16 rad = size / 2;

  for (x = 0; x < rad; x++)
    for (y = 0; y < rad; y++) {
      alias = 0;
      for (t = 0; t < 8; t++)
        for (z = 0; z < 8; z++) {
          double dx = x * 8 + t;
          double dy = y * 8 + z;

          dx /= ((rad+1) * 8);
          dy /= ((rad+1) * 8);

          double o = pow(sin(acos(pow(dx, 1/power))), power);

          if (dy < o)
            alias++;
        }

      alias = alias * 255 / 64 + 0.5;


      ((Uint8*)s->pixels)[y*s->pitch+x] = 255-(Uint8)alias;
      ((Uint8*)s->pixels)[(size-2-y)*s->pitch+x] = 255-(Uint8)alias;
      ((Uint8*)s->pixels)[y*s->pitch+size-2-x] = 255-(Uint8)alias;
      ((Uint8*)s->pixels)[(size-2-y)*s->pitch+size-2-x] = 255-(Uint8)alias;
    }
}

unsigned char getpixel(SDL_Surface *s, Uint16 x, Uint16 y)
{
  return ((Uint8*)(s->pixels))[y*s->pitch+x*s->format->BytesPerPixel];
}

void generate_fishes(SDL_Surface **c, SDL_Surface **m) {
  int i, x, y;

  SDL_Surface *col = SDL_CreateRGBSurface(0, 32*40, 2*40, 24, 0xff, 0xff00, 0xff0000, 0);
  SDL_Surface *msk = SDL_CreateRGBSurface(0, 32*40, 2*40, 24, 0xff, 0xff00, 0xff0000, 0);

  *c = col;
  *m = msk;

  for (i = 0; i < 32; i++) {
    char s[100];

    sprintf(s, "fish/render/%05i.tga", i+1);

    SDL_Surface *img = IMG_LoadTGA_RW(SDL_RWFromFile(s, "rb"));

    for (y = 0; y < 40; y++)
      for (x = 0; x < 40; x++) {
        Uint8 r, g, b, a;
        Uint32 pix = *((Uint32*)((Uint8*)img->pixels + y*img->pitch+x*img->format->BytesPerPixel));
        SDL_GetRGBA(pix, img->format, &r, &g, &b, &a);

        *((Uint32*)((Uint8*)col->pixels + y*col->pitch + (x+i*40)*col->format->BytesPerPixel)) = SDL_MapRGB(col->format, r, g, b);
        *((Uint32*)((Uint8*)msk->pixels + y*msk->pitch + (x+i*40)*msk->format->BytesPerPixel)) = SDL_MapRGB(msk->format, a, a, a);
      }

    SDL_FreeSurface(img);

    sprintf(s, "fish/render/%5i.tga", 10001+i);

    img =  IMG_LoadTGA_RW(SDL_RWFromFile(s, "rb"));

    for (y = 0; y < 40; y++)
      for (x = 0; x < 40; x++) {
        Uint8 r, g, b, a;
        Uint32 pix = *((Uint32*)((Uint8*)img->pixels + y*img->pitch+x*img->format->BytesPerPixel));
        SDL_GetRGBA(pix, img->format, &r, &g, &b, &a);

        *((Uint32*)((Uint8*)col->pixels + (y+40)*col->pitch + (x+i*40)*col->format->BytesPerPixel)) = SDL_MapRGB(col->format, r, g, b);
        *((Uint32*)((Uint8*)msk->pixels + (y+40)*msk->pitch + (x+i*40)*msk->format->BytesPerPixel)) = SDL_MapRGB(msk->format, a, a, a);
      }

    SDL_FreeSurface(img);
  }
}

void generate_submarine(SDL_Surface **c, SDL_Surface **m) {
  int i, x, y;

  SDL_Surface *col = SDL_CreateRGBSurface(0, 120, 80*(22+9), 24, 0xff, 0xff00, 0xff0000, 0);
  SDL_Surface *msk = SDL_CreateRGBSurface(0, 120, 80*(22+9), 24, 0xff, 0xff00, 0xff0000, 0);

  *c = col;
  *m = msk;

  for (i = 0; i < 9; i++) {
    char s[100];

    sprintf(s, "submarine/render/%04i.tga", i*4+1);

    SDL_Surface *img = IMG_LoadTGA_RW(SDL_RWFromFile(s, "rb"));

    for (y = 0; y < 80; y++)
      for (x = 0; x < 120; x++) {
        Uint8 r, g, b, a;
        Uint32 pix = *((Uint32*)((Uint8*)img->pixels + y*img->pitch+x*img->format->BytesPerPixel));
        SDL_GetRGBA(pix, img->format, &r, &g, &b, &a);

        if (a == 0) {
            r = g = b = 0;
        }

        *((Uint32*)((Uint8*)col->pixels + (y+i*80)*col->pitch + x*col->format->BytesPerPixel)) = SDL_MapRGB(col->format, r, g, b);
        *((Uint32*)((Uint8*)msk->pixels + (y+i*80)*msk->pitch + x*msk->format->BytesPerPixel)) = SDL_MapRGB(msk->format, a, a, a);
      }

    SDL_FreeSurface(img);
  }

  for (i = 0; i < 22; i++) {
    char s[100];

    sprintf(s, "submarine/render/%04i.tga", i+50);

    SDL_Surface *img = IMG_LoadTGA_RW(SDL_RWFromFile(s, "rb"));

    for (y = 0; y < 80; y++)
      for (x = 0; x < 120; x++) {
        Uint8 r, g, b, a;
        Uint32 pix = *((Uint32*)((Uint8*)img->pixels + y*img->pitch+x*img->format->BytesPerPixel));
        SDL_GetRGBA(pix, img->format, &r, &g, &b, &a);

        if (a == 0) {
            r = g = b = 0;
        }

        *((Uint32*)((Uint8*)col->pixels + (y+80*(i+9))*col->pitch + x*col->format->BytesPerPixel)) = SDL_MapRGB(col->format, r, g, b);
        *((Uint32*)((Uint8*)msk->pixels + (y+80*(i+9))*msk->pitch + x*msk->format->BytesPerPixel)) = SDL_MapRGB(msk->format, a, a, a);
      }

    SDL_FreeSurface(img);
  }
}

int main() {

  Uint16 s, n, x, y;
  Uint8 c;

  FILE *outf = fopen("sprites.dat", "wb");

  SDL_Surface *colors = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_robots_colors.png", "rb"));
  SDL_Surface *mask = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_robots_mask.png", "rb"));

  write_palette(outf, colors);

  printf("robots\n");

  c = 8;
  fwrite(&c, 1, 1, outf);

  for (s = 0; s < 8; s++) {

    c = 32;
    fwrite(&c, 1, 1, outf);

    for (n = 0; n < 32; n++) {
      for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
          Uint8 b;

          b = get_color(colors, n*32+x, s*32+y);
          fwrite(&b, 1, 1, outf);
          b = get_alpha(mask, n*32+x, s*32+y);
          fwrite(&b, 1, 1, outf);
        }
      }
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  colors = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_balls_colors.png", "rb"));
  mask = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_balls_mask.png", "rb"));

  printf("balls\n");

  write_palette(outf, colors);

  for (n = 0; n < 2; n++) {
    for (y = 0; y < 32; y++) {
      for (x = 0; x < 32; x++) {
        Uint8 b;

        b = get_color(colors, n*32+x, y);
        fwrite(&b, 1, 1, outf);
        b = get_alpha(mask, n*32+x, y);
        fwrite(&b, 1, 1, outf);
      }
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  colors = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_box_colors.png", "rb"));
  mask = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_box_mask.png", "rb"));

  printf("box\n");

  write_palette(outf, colors);

  for (n = 0; n < 16; n++) {
    for (y = 0; y < 16; y++) {
      for (x = 0; x < 16; x++) {
        Uint8 b;

        b = get_color(colors, n*16+x, y);
        fwrite(&b, 1, 1, outf);
        b = get_alpha(mask, n*16+x, y);
        fwrite(&b, 1, 1, outf);
      }
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  colors = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_snowball_colors.png", "rb"));
  mask = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_snowball_mask.png", "rb"));

  printf("snowball\n");

  write_palette(outf, colors);

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
        Uint8 b;

        b = get_color(colors, x, y);
        fwrite(&b, 1, 1, outf);
        b = get_alpha(mask, x, y);
        fwrite(&b, 1, 1, outf);
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  printf("star\n");

  SDL_Surface *src = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 8, 0, 0, 0, 0);

  for (n = 0; n < 256; n++) {
    src->format->palette->colors[n].r = n;
    src->format->palette->colors[n].g = n;
    src->format->palette->colors[n].b = n;
  }

  write_palette(outf, src);


  for (n = 0; n < 16; n++) {
    render_star(src, 32, 3 * exp((-1.0*n)/5));
    for (y = 0; y < 32; y++) {
      for (x = 0; x < 32; x++) {
          Uint8 b;

          b = 255;
          fwrite(&b, 1, 1, outf);
          b = get_alpha(src, x, y);
          fwrite(&b, 1, 1, outf);
      }
    }
  }

  SDL_FreeSurface(src);

  printf("fisch\n");
  generate_fishes(&colors, &mask);

  {
    SDL_Surface *col2 = colorreduction(colors, 256);
    SDL_Surface *msk2 = colorreduction(mask, 256);

    SDL_FreeSurface(colors);
    SDL_FreeSurface(mask);

    colors = col2;
    mask = msk2;
  }

  write_palette(outf, colors);

  for (n = 0; n < 2; n++) {
    for (s = 0; s < 32; s++) {
      for (y = 0; y < 40; y++) {
        for (x = 0; x < 40; x++) {
          Uint8 b;

          b = get_color(colors, x+s*40, y+n*40);
          fwrite(&b, 1, 1, outf);
          b = get_alpha(mask, x+s*40, y+n*40);
          fwrite(&b, 1, 1, outf);
        }
      }
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  printf("submarine\n");
  generate_submarine(&colors, &mask);

  {
    SDL_Surface *col2 = colorreduction(colors, 256);
    SDL_Surface *msk2 = colorreduction(mask, 256);

    SDL_FreeSurface(colors);
    SDL_FreeSurface(mask);

    colors = col2;
    mask = msk2;
  }

  write_palette(outf, colors);

  for (n = 0; n < 31; n++) {
    for (y = 0; y < 80; y++) {
      for (x = 0; x < 120; x++) {
        Uint8 b;

        b = get_color(colors, x, y+n*80);
        fwrite(&b, 1, 1, outf);
        b = get_alpha(mask, x, y+n*80);
        fwrite(&b, 1, 1, outf);
      }
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  colors = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_torpedo_colors.png", "rb"));
  mask = IMG_LoadPNG_RW(SDL_RWFromFile("sprites_torpedo_mask.png", "rb"));

  printf("torpedo\n");

  write_palette(outf, colors);

  for (y = 0; y < 6; y++) {
    for (x = 0; x < 32; x++) {
        Uint8 b;

        b = get_color(colors, x, y);
        fwrite(&b, 1, 1, outf);
        b = get_alpha(mask, x, y);
        fwrite(&b, 1, 1, outf);
    }
  }

  SDL_FreeSurface(colors);
  SDL_FreeSurface(mask);

  fclose(outf);
}



