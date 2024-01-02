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

  printf("loading image: colors\n");
  SDL_Surface *colors = IMG_LoadPNG_RW(SDL_RWFromFile("titles_colors.png", "rb"));
  printf("loading image: mask\n");
  SDL_Surface *mask = IMG_LoadPNG_RW(SDL_RWFromFile("titles_mask.png", "rb"));

  if (!colors || !mask) {
    printf("could not open images\n");
    return 1;
  }

  FILE *outp = fopen("titles.dat", "wb");

  printf("writing palette\n");
  write_palette(outp, colors);

  int x, y;

  printf("writing data\n");
  for (y = 0; y < colors->h/2; y++)
    for (x = 0; x < colors->w; x++) {
      Uint8 c;

      c = get_color(colors, x, y);
      fwrite(&c, 1, 1, outp);

      c = get_alpha(mask, x, y);
      fwrite(&c, 1, 1, outp);
    }

  for (y = 0; y < colors->h/2; y++)
    for (x = 0; x < colors->w; x++) {
      Uint8 c;

      c = get_color(colors, x, y + colors->h/2);
      fwrite(&c, 1, 1, outp);

      c = get_alpha(mask, x, y + colors->h/2);
      fwrite(&c, 1, 1, outp);
    }

  fclose(outp);

  return 0;
}
