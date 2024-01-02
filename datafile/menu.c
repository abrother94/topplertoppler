#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

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

int main()
{
  int x, y;

  SDL_Surface *colors =  IMG_LoadPNG_RW(SDL_RWFromFile("menu.png", "rb"));

  FILE *outp = fopen("menu.dat", "wb");

  write_palette(outp, colors);

  for (y = 0; y < 480; y++)
    for (x = 0; x < 1280; x++)
      fwrite(&((Uint8*)colors->pixels)[y*colors->pitch+x], 1, 1, outp);

  fclose(outp);

}
