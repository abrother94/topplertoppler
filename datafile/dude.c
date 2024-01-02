#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>

Uint8 get_color(SDL_Surface *s, int x, int y) {
  return ((Uint8*)s->pixels)[y*s->pitch+x];
}

Uint8 get_alpha(SDL_Surface *s, int x, int y) {
  return s->format->palette->colors[((Uint8*)s->pixels)[y*s->pitch+x]].r;
}

void getpixel(Uint16 z, Uint8 fx, Uint16 x, Uint16 y,
              FILE *out, SDL_Surface *col, SDL_Surface *msk)
{
  if (fx)
    x = 39 - x;

  y = z + y;

  Uint8 c;

  c = get_color(col, x, y);
  fwrite(&c, 1, 1, out);

  c = get_alpha(msk, x, y);
  fwrite(&c, 1, 1, out);

}

void get(Uint16 z, Uint8 fx, FILE *out, SDL_Surface *col, SDL_Surface *msk)
{
  unsigned short x, y;

  for (y = 0; y < 40; y++)
    for (x = 0; x < 40; x++) {
      getpixel(z, fx, x, y, out, col, msk);
    }
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

int main()
{

  SDL_Surface * colors = IMG_LoadPNG_RW(SDL_RWFromFile("dude_colors.png", "rb"));
  SDL_Surface * mask = IMG_LoadPNG_RW(SDL_RWFromFile("dude_mask.png", "rb"));

  FILE *outp = fopen("dude.dat", "wb");

  write_palette(outp, colors);

  get( 0 * 40, 0, outp, colors, mask);
  get( 1 * 40, 0, outp, colors, mask);
  get( 2 * 40, 0, outp, colors, mask);
  get( 3 * 40, 0, outp, colors, mask);
  get( 4 * 40, 0, outp, colors, mask);
  get( 5 * 40, 0, outp, colors, mask);
  get( 6 * 40, 0, outp, colors, mask);
  get( 7 * 40, 0, outp, colors, mask);
  get( 8 * 40, 0, outp, colors, mask);
  get( 9 * 40, 0, outp, colors, mask);
  get(10 * 40, 0, outp, colors, mask);
  get(11 * 40, 0, outp, colors, mask);

  get(12 * 40, 0, outp, colors, mask);
  get(13 * 40, 0, outp, colors, mask);
  get(14 * 40, 0, outp, colors, mask);
  get(15 * 40, 0, outp, colors, mask);
  get(16 * 40, 0, outp, colors, mask);
  get(17 * 40, 0, outp, colors, mask);
  get(18 * 40, 0, outp, colors, mask);
  get(19 * 40, 0, outp, colors, mask);
  get(20 * 40, 0, outp, colors, mask);
  get(21 * 40, 0, outp, colors, mask);
  get(22 * 40, 0, outp, colors, mask);
  get(23 * 40, 0, outp, colors, mask);

  get(24 * 40, 0, outp, colors, mask);
  get(25 * 40, 0, outp, colors, mask);
  get(26 * 40, 0, outp, colors, mask);
  get(27 * 40, 0, outp, colors, mask);
  get(28 * 40, 0, outp, colors, mask);
  get(29 * 40, 0, outp, colors, mask);
  get(30 * 40, 0, outp, colors, mask);
  get(31 * 40, 0, outp, colors, mask);
  get(32 * 40, 0, outp, colors, mask);
  get(33 * 40, 0, outp, colors, mask);
  get(34 * 40, 0, outp, colors, mask);
  get(35 * 40, 0, outp, colors, mask);

  get(36 * 40, 0, outp, colors, mask);

  get( 0 * 40, 1, outp, colors, mask);
  get( 1 * 40, 1, outp, colors, mask);
  get( 2 * 40, 1, outp, colors, mask);
  get( 3 * 40, 1, outp, colors, mask);
  get( 4 * 40, 1, outp, colors, mask);
  get( 5 * 40, 1, outp, colors, mask);
  get( 6 * 40, 1, outp, colors, mask);
  get( 7 * 40, 1, outp, colors, mask);
  get( 8 * 40, 1, outp, colors, mask);
  get( 9 * 40, 1, outp, colors, mask);
  get(10 * 40, 1, outp, colors, mask);
  get(11 * 40, 1, outp, colors, mask);

  get(12 * 40, 1, outp, colors, mask);
  get(13 * 40, 1, outp, colors, mask);
  get(14 * 40, 1, outp, colors, mask);
  get(15 * 40, 1, outp, colors, mask);
  get(16 * 40, 1, outp, colors, mask);
  get(17 * 40, 1, outp, colors, mask);
  get(18 * 40, 1, outp, colors, mask);
  get(19 * 40, 1, outp, colors, mask);
  get(20 * 40, 1, outp, colors, mask);
  get(21 * 40, 1, outp, colors, mask);
  get(22 * 40, 1, outp, colors, mask);
  get(23 * 40, 1, outp, colors, mask);

  get(24 * 40, 1, outp, colors, mask);
  get(25 * 40, 1, outp, colors, mask);
  get(26 * 40, 1, outp, colors, mask);
  get(27 * 40, 1, outp, colors, mask);
  get(28 * 40, 1, outp, colors, mask);
  get(29 * 40, 1, outp, colors, mask);
  get(30 * 40, 1, outp, colors, mask);
  get(31 * 40, 1, outp, colors, mask);
  get(32 * 40, 1, outp, colors, mask);
  get(33 * 40, 1, outp, colors, mask);
  get(34 * 40, 1, outp, colors, mask);
  get(35 * 40, 1, outp, colors, mask);

  get(36 * 40, 1, outp, colors, mask);

  fclose(outp);
}

