#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>

/*
 * this program loads a few picture files and creates a file with information
 * for the program for the scroller
 * the input pictures must be .PNG
 *
 * the data file should allow for any number of layers, each layer can be up to 480 pixel high
 * but can have a different width
 * you must specify the speed by which the image is moved with 2 numbers. the speed
 * will actually be the fraction of number one and two
 *
 * so you can create a standing background image by specifying a 0 for speed
 */

/*
 * ok this structure contains the data for one layer
 */
typedef struct layer {

  unsigned int xpos, ypos;
  unsigned int width, height;
  unsigned int numerator, denominator;
  unsigned int xrepeat;

  SDL_Surface * colors;
  SDL_Surface * mask;

  struct layer * next;
} layer;

/*
 * the layer list anchor
 */
layer * layer_ancor = NULL;

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

void write_uint(unsigned int x, FILE * out) {
    unsigned char c;
    
    c = x & 0xff;
    fwrite(&c, 1, 1, out);
    c = x >> 8;
    fwrite(&c, 1, 1, out);
}

/*
 * writes on layer to the file
 */
void write_layer(layer * l, FILE * out) {

  int x, y;

  static int first = 0;

  Uint8 b;

  write_uint(l->xpos, out);
  write_uint(l->ypos, out);
  write_uint(l->width, out);
  write_uint(l->height, out);
  write_uint(l->numerator, out);
  write_uint(l->denominator, out);
  write_uint(l->xrepeat, out);

  write_palette(out, l->colors);

  for (y = 0; y < l->height; y++)
    for (x = 0; x < l->width; x++) {
      b = get_color(l->colors, x, y);
      fwrite(&b, 1, 1, out);
      if (first) {
        b = get_alpha(l->mask, x, y);
        fwrite(&b, 1, 1, out);
      }
    }

  first = 1;
}

void save(layer * l, FILE * o) {

  if (l) {
    save(l->next, o);
    write_layer(l, o);
  }
}

/*
 *
 * scroller number_of_layers tower_pos tower_spd_num/tower_spd_den \
 *     [ pic_file mask_file xpos/ypos/xrepeat spd_num/spd_den ]...
 */

int main(int argv, char* args[]) {

  int l;
  int layers; // number of layers
  unsigned int towerpos;
  unsigned int towerspeed_num, towerspeed_den;

  if (argv < 3) return 1;

  printf("start\n");

  sscanf(args[1], "%i", &layers);
  printf("generating %i layers\n", layers);

  sscanf(args[2], "%i", &towerpos);
  sscanf(args[3], "%i/%i", &towerspeed_num, &towerspeed_den);
  printf("putting tower as layer %i with speed %i/%i\n", towerpos, towerspeed_num, towerspeed_den);

  for (l = 0; l < layers; l++) {
    layer * la = (layer*)malloc(sizeof(layer));

    if (l == towerpos) printf("tower, speed %i/%i\n", towerspeed_num, towerspeed_den);
      
    la->colors = IMG_LoadPNG_RW(SDL_RWFromFile(args[4+4*l], "rb"));
    la->mask = IMG_LoadPNG_RW(SDL_RWFromFile(args[4+4*l+1], "rb"));

    la->width = la->colors->w;
    la->height = la->colors->h;

    sscanf(args[4+4*l+2], "%i/%i/%i", &la->xpos, &la->ypos, &la->xrepeat);

    sscanf(args[4+4*l+3], "%i/%i", &la->numerator, &la->denominator);

    printf("image %s, size (%i,%i), at (%i,%i), speed %i/%i, repeat %i\n", args[4+4*l],
	   la->width, la->height,
	   la->xpos, la->ypos,
	   la->numerator, la->denominator,
	   la->xrepeat);

    la->next = layer_ancor;
    layer_ancor = la;
  }

  printf("\n saving...\n");

  FILE * o = fopen("scroller.dat", "wb");

  fwrite(&layers, 1, 1, o);
  fwrite(&towerpos, 1, 1, o);

  write_uint(towerspeed_num, o);
  write_uint(towerspeed_den, o);

  save(layer_ancor, o);

  return 0;
}



