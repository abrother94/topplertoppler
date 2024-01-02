#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>


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

int main() {

  FILE *outf = fopen("cross.dat", "wb");

  SDL_Surface * colors = IMG_LoadPNG_RW(SDL_RWFromFile("cross_colors.png", "rb"));
  SDL_Surface * mask = IMG_LoadPNG_RW(SDL_RWFromFile("cross_mask.png", "rb"));

  printf("writing palette\n");
  write_palette(outf, colors);

  int n, x, y;

  for (n = 0; n < 120; n++) {
    for (y = 0; y < 32; y++) {
      for (x = 0; x < 32; x++) {
        Uint8 c = get_color(colors, x, n*32 + y);
        fwrite(&c, 1, 1, outf);

        c = get_alpha(mask, x, n*32 + y);
        fwrite(&c, 1, 1, outf);
      }
    }
  }

  fclose(outf);
}


