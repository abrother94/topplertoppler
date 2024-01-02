/*
 * program to assemble a pile of little images into a bigger one to
 * calculate a common palette
 */

#include <SDL.h>
#include <SDL_image.h>

#include "pngsaver.h"

SDL_Surface *out_colors, *out_mask;
SDL_Surface *inp;

/*
 * first argument: horizontal or vertial (h/v/hm/vm)
 * second argument: output filename
 * 3rd- argument: input filenames
 *
 * the first halve of the images must be the color images and the second halve
 * the mask images
 *
 * all images should have same size otherwise strange things
 * may happen
 */

int main(int argn, char *args[]) {
  if (argn < 4)
    return 0;

  int vertical;
  int correction;
  int arg;
  int images = (argn - 3) / 2;

  if (args[1][0] == 'h')
    vertical = 0;
  else
    vertical = 1;

  if (args[1][1] == 'm')
    correction = 1;
  else
    correction = 0;

  arg = 3;

  SDL_Rect r;

  r.x = 0;
  r.y = 0;

  printf("  loading images\n");

  while (arg < images + 3) {
    inp = IMG_LoadPNG_RW(SDL_RWFromFile(args[arg], "rb"));

    if (!inp)
        inp = IMG_LoadTGA_RW(SDL_RWFromFile(args[arg], "rb"));

    if (arg == 3) {
      if (vertical) {
        out_colors = SDL_CreateRGBSurface(0, inp->w, inp->h * images, 32, 0xff0000, 0xff00, 0xff, 0);
        out_mask = SDL_CreateRGBSurface(0, inp->w, inp->h * images, 32, 0xff0000, 0xff00, 0xff, 0);
      } else {
        out_colors = SDL_CreateRGBSurface(0, inp->w * images, inp->h, 32, 0xff0000, 0xff00, 0xff, 0);
        out_mask = SDL_CreateRGBSurface(0, inp->w * images, inp->h, 32, 0xff0000, 0xff00, 0xff, 0);
      }
    }

    SDL_BlitSurface(inp, NULL, out_colors, &r);

    SDL_FreeSurface(inp);

    inp = IMG_LoadPNG_RW(SDL_RWFromFile(args[arg+images], "rb"));

    if (!inp)
        inp = IMG_LoadTGA_RW(SDL_RWFromFile(args[arg+images], "rb"));

    SDL_BlitSurface(inp, NULL, out_mask, &r);

    if (vertical)
      r.y += inp->h;
    else
      r.x += inp->w;

    SDL_FreeSurface(inp);

    arg++;
  }


  if (correction) {
    Uint32 x, y;
    printf("  modifying colors\n");

    for (y = 0; y < out_colors->h; y++)
      for (x = 0; x < out_colors->w; x++) {
        Uint8 b = ((Uint8*)out_mask->pixels)[y*out_mask->pitch+x*out_mask->format->BytesPerPixel];

        if (b != 0) {

          double a;

          double f = (double)b / 255;

          a = ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 0];
          a = a / f;
          ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 0] = (Uint8)(a+0.5);

          a = ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 1];
          a = a / f;
          ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 1] = (Uint8)(a+0.5);

          a = ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 2];
          a = a / f;
          ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 2] = (Uint8)(a+0.5);

          a = ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 3];
          a = a / f;
          ((Uint8*)out_colors->pixels)[y*out_colors->pitch+x*out_mask->format->BytesPerPixel + 3] = (Uint8)(a+0.5);
        }
      }
  }

  char s[500];

  printf("  saving\n");

  sprintf(s, "%s_colors.png", args[2]);
  printf("  colors into %s\n", s);
  SavePNGImage(s, out_colors);
  printf("  mask into %s\n", s);
  sprintf(s, "%s_mask.png", args[2]);
  SavePNGImage(s, out_mask);
  
  return 0;
}
