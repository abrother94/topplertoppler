#include <SDL.h>
#include <SDL_image.h>

#include "pngsaver.h"

#include "colorreduction.h"

int main(int argn, char *args[]) {
  Uint16 colors_num;

  if (argn != 4)
    return 0;

  SDL_Surface *in_image = IMG_LoadPNG_RW(SDL_RWFromFile(args[1], "rb"));

  if (in_image == NULL) {
    printf("oops, could not load file\n");
    return 0;
  }

  colors_num = atoi(args[2]);

  SDL_Surface *out_image = colorreduction(in_image, colors_num);

  SavePNGImage(args[3], out_image);

  return 0;
}
