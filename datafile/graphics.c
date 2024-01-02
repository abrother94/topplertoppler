#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>

#include "colorreduction.h"
#include "pngsaver.h"

#define radius          96
#define steinzahl       16
#define hoehe           16

// angle to rotate the light source to the left
#define wadd            (2 * M_PI / steinzahl)

#define steprad         20
#define elevatorrad     16
#define stickrad        7

#define zinnenrad       (radius + 48)

SDL_Surface *display;
SDL_Surface *brick, *zinne, *brickn, *zinnen;

double lw = 10 * M_PI / 180;

double asin(double x)
{
  if (x == 1)
    return (M_PI / 2);
  else {
    if (x == -1)
      return (M_PI / -2);
    else
      return atan(x / sqrt(1 - x * x));
  }
}

void putpixel(unsigned short x, unsigned short y, Uint32 b)
{
  ((Uint8*)display->pixels)[y*display->pitch+x*display->format->BytesPerPixel+2] = b;
  ((Uint8*)display->pixels)[y*display->pitch+x*display->format->BytesPerPixel+1] = b >> 8;
  ((Uint8*)display->pixels)[y*display->pitch+x*display->format->BytesPerPixel+0] = b >> 16;
}

Uint8 getpixel(Uint16 x, Uint16 y)
{
   return ((Uint8*)display->pixels)[y * display->pitch + x * display->format->BytesPerPixel];
}

double fmax(double a, double b)
{
  if (a > b)
    return a;
  else
    return b;
}

void createdoor(double w, unsigned short ys, unsigned short xm, double doorwidth, double door_rad, double height, double doorl)
{
  int xl, xr, y, a;
  double nl, nr;
  double ll, lr;
  double x;
  Uint32 b;

  double doorlength = sqrt(4*door_rad*door_rad-doorwidth*doorwidth);

  if (doorl <= 0)
    doorl = doorlength;

  double beta = atan2(doorwidth, doorlength);


  while (w >= M_PI)
    w -= 2 * M_PI;
  while (w < -M_PI)
    w += 2 * M_PI;

  /* ok, lets first calculate the x coordinates of the borders of the door */
  xl = (int)(door_rad * sin(w - beta) + 0.5);
  xr = (int)(door_rad * sin(w + beta) + 0.5);

  /* the door is invisible if the left end is right of the right end */
  if (xl > xr)
    return;

  /* now to the light calculations */

  /* these are the angles the light hits the left and right wall of the
   doorway */
  nl = fmax(0.2, cos(-M_PI/2 - lw - w));
  nr = fmax(0.2, cos(M_PI/2 - lw - w));

  /* and now how far the light reaches into the tunnel bevore
   the other edge casts a shadow on it */
  {
    /* we do this by looking how far we can look into the tunnel at the
     opposide edge */
    double dxl = door_rad * sin (w - beta + lw);
    double dxr = door_rad * sin (w + beta + lw);

    ll = ((double)dxl-dxr) / ((double)dxr+dxl) * doorlength;
    lr = ((double)dxr-dxl) / ((double)dxr+dxl) * doorlength;
  }

  /* so, now lets draw the doors line by line from
   top to bottom, the size is fixed: three layers */
  for (y = 0; y < height; y++) {

    /* calculate the layer and the hight inside the layer */
    Uint8 layer = y / hoehe;
    Uint8 ystone = y % hoehe;

    /* draw from left end to the right end */
    for (x = xl; x < xr; x++) {

      /* we do anti aliasing so we need to sum up the colors */
      double c1, c2, c3;

      c1 = c2 = c3 = 0;

      /* eight times antialiasing */
      for (a = 0; a < 8; a++) {

        /* calc the atual position */
        double xpos = x + (double)a/8;

        /* if xpos < -xr then we are drawing part of the left wall, if xpos > -xl then
         we draw part of the right wall, in the last case we draw nothing because
         we look through the tunnel */
        if (xpos < -xr) {

          /* both cases are more or less the same, so I comment only one */

          /* first calculate what position of the wall we see at the current x-pos
           this is a simple linear equation that is 0 at xl (front edge) and doorlength
           at -xr (back edge of the tunnesl wall */
          double pixelpos = ((double)xl-xpos) / ((double)xr+xl) * doorlength;

          /* for rounding */
          pixelpos += 0.5;

          /* now lets calculate the position on the stone we see with the wall position given */
          double stonepos = pixelpos;

          /* at the middle layer it's shifted by half a stone width */
          if (layer == 1)
            stonepos += (brick->w/2);

          while (stonepos >= brick->w)
            stonepos -= brick->w;

          /* get the color from the brick */
          b = *((Uint32 *)(((Uint8*)brick->pixels) + ystone * brick->pitch + (int)stonepos * brick->format->BytesPerPixel));

          /* calculate the light itensity using smooth shadow that is exactly on the
           middle way between dark and the max light at the sharow start postition we
           have calculated above */
          double n = (1-(pixelpos - (ll-10)) / (2*10)) * (nl -0.2) + 0.2;
          if (n < 0.2) n = 0.2;
          if (n > nl) n = nl;
          if (pixelpos > doorl) n = 0;

          /* add shadowed values */
          c1 += (double)(b & 0xff) * nl;
          c2 += (double)((b >> 8) & 0xff) * nl;
          c3 += (double)((b >> 16) & 0xff) * nl;

        } else if (xpos > -xl) {
          double pixelpos = ((double)xr-xpos) / ((double)xr+xl) * doorlength;

          pixelpos += 0.5;

          double stonepos = pixelpos;

          if (layer == 1)
            stonepos += (brick->w/2);

          while (stonepos >= brick->w)
            stonepos -= brick->w;

          b = *((Uint32 *)(((Uint8*)brick->pixels) + ystone * brick->pitch + (int)stonepos * brick->format->BytesPerPixel));

          double n = (1-(pixelpos - (lr-5)) / (2*5)) * (nr -0.2) + 0.2;
          if (n < 0.2) n = 0.2;
          if (n > nr) n = nr;
          if (pixelpos > doorl) n = 0;

          c1 += (double)(b & 0xff) * n;
          c2 += (double)((b >> 8) & 0xff) * n;
          c3 += (double)((b >> 16) & 0xff) * n;
        }

      }

      /* calc average of the 8 samples */
      c1 /= 8;
      c2 /= 8;
      c3 /= 8;

      /* draw the pixel */
      b = (int)c3;
      b = (b << 8) | (int)c2;
      b = (b << 8) | (int)c1;
      putpixel((int)x+xm, y + ys, b);
    }
  }

  /* finally draw a black border around the door */
  /*
  for (y = ys; y < ys + hoehe * 3; y++) {
    putpixel(xl+xm, y, 0);
    putpixel(xr+xm, y, 0);
  }
  for (x = xl; x < xr; x++) {
    putpixel(x+xm, ys, 0);
    putpixel(x+xm, ys + hoehe*3-1, 0);
    }
    */
}


// nax is the angle x rotation part of the normal of the texture to apply, 0 means the normal goes vertically
// out of the screen
void putstein_slice(unsigned short ys, unsigned short x1, unsigned short x2, double nax) {
  unsigned short x, y;
  long m;
  Uint32 b, a;
  double c1, c2, c3;

  // the coordinate system used:
  // x axis: to the top of the tower
  // y axis: to the right the screen
  // z axis: into the screen (to be right handed)

  // the normal vector for the light
  double ln[3] = { 0, sin(-2*lw), -cos(-2*lw) };

  for (y = 0; y < hoehe; y++) {
    for (x = x1; x < x2; x++) {

      c1 = c2 = c3 = 0.0;

      for (a = 0; a < 8; a++) {
        m = (long)((double)(x + (float)a / 8 - x1) / (x2 - x1) * brickn->w + 0.5);
        if (m >= brickn->w)
          m = brickn->w - 1;

        b = *((Uint32 *)(((Uint8*)brickn->pixels) + y * brickn->pitch + m * brickn->format->BytesPerPixel));

        c1 += b & 0xff;
        c2 += (b >> 8) & 0xff;
        c3 += (b >> 16) & 0xff;
      }

      c1 /= 8;
      c2 /= 8;
      c3 /= 8;

      c1 /= 255;
      c2 /= 255;
      c3 /= 255;

      c1 -= 0.5;
      c2 -= 0.5;
      c3 -= 0.5;

      c1 *= 7;
      c2 *= 7;

      // calculate the normal vector for the brick totated by nax around the y axis
      double nm[3] = { c2, sin(c1+nax), -cos(c1+nax) };


      //calculate the angle between ln and nm

      double cosa = nm[0]*ln[0] + nm[1]*ln[1] + nm[2]*ln[2];
      cosa /= sqrt(nm[0]*nm[0]+nm[1]*nm[1]+nm[2]*nm[2]);
      cosa /= sqrt(ln[0]*ln[0]+ln[1]*ln[1]+ln[2]*ln[2]);

      double angle = fabs(acos(cosa));
      double highlight = 4*angle;

      double n = angle / (M_PI/1.7);
      if (n > 1.0) n = 1.0;

      double nh = highlight / (M_PI/2);
      if (nh > 1.0) nh = 1.0;

      //printf("[%f %f %f] %f\n", nm[0], nm[1], nm[2], n);

      c1 = c2 = c3 = 0;//128-128*nh;
      c1 = 200-200*n;
      if (y == 0 || x == x1)
      {
          c1 = 4*c1/5;
          c2 = c3 = c1;
      }

      b = (int)c3;
      b = (b << 8) | (int)c2;
      b = (b << 8) | (int)c1;

      putpixel(x, y + ys, b);
    }
  }
}

void createslice(double w, unsigned short ys) {
  double e;
  int t;

  for (t = 0; t < steinzahl; t++) {
    e = 2 * M_PI * t / steinzahl + w;
    int x1 = (int)(radius * cos(e) + radius + 0.5);
    int x2 = (int)(radius * cos(e + wadd) + radius + 0.5);
    if (x1 < x2)
      putstein_slice(ys, x1, x2, e+M_PI/2);
  }
}

void createcyl(Uint16 r, FILE *out)
{
  int x, y, h;

  for (y = 0; y < hoehe - 1; y++) {
    for (x = -r; x <= r-1; x++) {
      h = (int)(255 * (1-fabs(sin(asin((double)x / r) + lw))) + 0.5);
      Uint8 c = h;

      fwrite(&c, 1, 1, out);
    }
  }
}


void putstein_pinacle(unsigned short ys, unsigned short x1, unsigned short x2, double n, double e, double nax)
{
  unsigned short x, y;
  double m;
  Uint32 b, a;
  double c1, c2, c3;

  static bool s_init = false;
  static int b1 = 0;
  static int b2 = 0;
  static int b3 = 0;

  if (!s_init) {
    s_init = true;

    x = 0;

    while (*(Uint32*)((Uint8*)zinne->pixels + x * zinne->format->BytesPerPixel) == 0) {
      x++;
    }

    b1 = x;

    while (*(Uint32*)((Uint8*)zinne->pixels + x * zinne->format->BytesPerPixel) != 0) {
      x++;
      b2++;
    }

    b3 = zinne->w - b1 - b2;
  }

  e -= M_PI + M_PI/2;
  double nn = b2 * sin(e) / 2;
  double n55 = fmax(0.2, fabs( sin(e+lw)));

  n = -n;

  if (n < 0.2) n = 0.2;

  double ln[3] = { 0, sin(-2*lw), -cos(-2*lw) };

  for (y = 0; y < hoehe * 3; y++) {
    for (x = 0; x < 2*zinnenrad; x++) {

      c1 = c2 = c3 = 0;

      for (a = 0; a < 8; a++) {
        m = (x + (double)a / 8 - x1) / (x2 - x1) * zinne->w + 0.5;

        if ((m >= zinne->w) || (m < 0))
          b = 0;
        else
          b = *((Uint32 *)(((Uint8*)zinne->pixels) + y * zinne->pitch + ((long)m) * zinne->format->BytesPerPixel));

        /* outside of the texture, here it is possible that another texture is used */
        if (b == 0) {

          if (m < zinne->w/2) {

            /* left void */
            if ((x + (double)a/8 > x1 + b1 * (x2-x1)/zinne->w - nn) && (nn > 0)) {

              b = *((Uint32 *)(((Uint8*)zinne->pixels) +
                               y * zinne->pitch +
                               (long)((x+(double)a/8 - x1 - b1 * (x2-x1)/zinne->w) / (-nn-2) * b2 + b1 + 1.5) * zinne->format->BytesPerPixel));

              c1 += n55 * (b & 0xff);
              c2 += n55 * ((b >> 8) & 0xff);
              c3 += n55 * ((b >> 16) & 0xff);
            }
          } else {
            /* right void */
            if ((x + (double)a/8 < x1 + (b1 + b2) * (x2-x1)/zinne->w - nn) && (nn < 0)) {

              long xx = ((x+(double)a/8 - x1 - (b1 + b2) * (x2-x1)/zinne->w) / (-nn-2) * b2 + b1 + 1.5);

              if (xx < zinne->w && xx >= 0)
                  b = *((Uint32 *)(((Uint8*)zinne->pixels) +
                                   y * zinne->pitch + xx * zinne->format->BytesPerPixel));
              else
                  b = 0;

              c1 += n55 * (b & 0xff);
              c2 += n55 * ((b >> 8) & 0xff);
              c3 += n55 * ((b >> 16) & 0xff);
            }
          }

        } else {

          // inside texture, do the whole shenanigans again but using
          // the bumpmap tecture

          long m2 = (long)((double)(x + a/8.0 - x1) / (x2 - x1) * zinnen->w);
          if (m2 >= zinnen->w)
            m2 = zinnen->w - 1;
          if (m2 < 0)
            m2 = 0;

          b = *((Uint32 *)(((Uint8*)zinnen->pixels) + y * zinnen->pitch + (long)m2 * zinnen->format->BytesPerPixel));

          double c1n = b & 0xff;
          double c2n = (b >> 8) & 0xff;
          double c3n = (b >> 16) & 0xff;

          c1n /= 255;
          c2n /= 255;
          c3n /= 255;

          c1n -= 0.5;
          c2n -= 0.5;
          c3n -= 0.5;

          c1n *= 7;
          c2n *= 7;

          // calculate the normal vector for the brick totated by nax around the y axis
          double nm[3] = { c2n, sin(c1n+nax), -cos(c1n+nax) };

          //calculate the angle between ln and nm

          double cosa = nm[0]*ln[0] + nm[1]*ln[1] + nm[2]*ln[2];
          cosa /= sqrt(nm[0]*nm[0]+nm[1]*nm[1]+nm[2]*nm[2]);
          cosa /= sqrt(ln[0]*ln[0]+ln[1]*ln[1]+ln[2]*ln[2]);

          double angle = fabs(acos(cosa));
          double highlight = 4*angle;

          double n = angle / (M_PI/1.7);
          if (n > 1.0) n = 1.0;

          double nh = highlight / (M_PI/2);
          if (nh > 1.0) nh = 1.0;

          //printf("[%f %f %f] %f\n", nm[0], nm[1], nm[2], n);

          c1n = c2n = c3n = 0;//128-128*nh;
          c1n = 200-200*n;
          if (y == 0 || x == x1)
          {
              c1n = 4*c1n/5;
              c2n = c3n = c1n;
          }

          c1 += c1n;
          c2 += c2n;
          c3 += c3n;
        }
      }

      c1 /= 8;
      c2 /= 8;
      c3 /= 8;

      c1 += 0.5;
      c2 += 0.5;
      c3 += 0.5;

      b = (long)c3;
      b = (b << 8) | (long)c2;
      b = (b << 8) | (long)c1;

      if (b)
        putpixel(x, y + ys, b);
    }
  }
}

void writebrickpixel(Uint16 x, Uint16 y, FILE *out) {

  Uint8 c = getpixel(x, y);

  fwrite(&c, 1, 1, out);
}


void createzinne(unsigned short ys, double w)
{
  double e, n;
  long t, x1, x2, xl;
  long xmin = zinnenrad, xmax = zinnenrad;

  for (t = 0; t < steinzahl; t++) {
    e = 2 * M_PI * t / steinzahl + w;
    x1 = (long)floor(zinnenrad * cos(e) + zinnenrad + 0.5);
    x2 = (long)floor(zinnenrad * cos(e + wadd) + zinnenrad + 0.5);
    n = sin(e + wadd + lw);
    if ((x1 < x2) && (e < 3*M_PI/2-wadd/2) ) {
      putstein_pinacle(ys, (int)x1, (int)x2, n, e+ wadd/2, e+M_PI/2);
      xl = x2;
      if (x1 < xmin) xmin = x1;
      if (x2 > xmax) xmax = x2;
    }
  }

  for (t = steinzahl-1; t >= 0; t--)
  {
    e = 2 * M_PI * t / steinzahl + w;
    x1 = (long)floor(zinnenrad * cos(e) + zinnenrad + 0.5);
    x2 = (long)floor(zinnenrad * cos(e + wadd) + zinnenrad + 0.5);
    n = sin(e + wadd + lw);
    if ((x1 < x2) && (e >= 3*M_PI/2-wadd/2) ) {
      putstein_pinacle(ys, (int)x1, (int)x2, n, e+ wadd/2, e+M_PI/2);
      xl = x2;
      if (x1 < xmin) xmin = x1;
      if (x2 > xmax) xmax = x2;
    }
  }
}

void getdoor(Uint16 ys, FILE *out) {

  int xs = -1;
  int br = 1;
  int x, y, t;

  Uint8 key = getpixel(display->w-1, 0);

  for (x = 0; x < 2*radius + 10; x++) {
    if (getpixel(x, ys) != key) {
      if (xs == -1)
        xs = x;
      else
        br++;
    }
  }

  if (xs == -1) {
    xs = 0;
    fwrite(&xs, 1, 2, out);
    fwrite(&xs, 1, 2, out);
  } else {
    /*int x, y, t;*/
    xs -= radius;
    fwrite(&xs, 1, 2, out);
    fwrite(&br, 1, 2, out);
    xs += radius;
    for (t = 0; t < 3; t++)
      for (y = 0; y < 16; y++)
        for (x = 0; x < br; x++) {
          writebrickpixel(x+xs, ys+ t*16+y, out);
        }
  }
}

int main() {

  FILE *outp = fopen("graphics.dat", "wb");

  int y, t, x;

  SDL_Init(SDL_INIT_VIDEO);

  int img_height = 8*16+8*3*16+73*16*3;

  display = SDL_CreateRGBSurface(SDL_SWSURFACE, 2* zinnenrad, img_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
  //display = SDL_SetVideoMode(640, 480, 32, 0);

  brick = IMG_LoadPNG_RW(SDL_RWFromFile("graphics_brick.png", "rb"));
  zinne = IMG_LoadPNG_RW(SDL_RWFromFile("graphics_pinacle.png", "rb"));

  // create a normal map for the bricks... same size
  brickn = SDL_CreateRGBSurface(0, brick->w, brick->h, 32, 0x00FF0000, 0xFF00, 0xFF, 0);
  zinnen = SDL_CreateRGBSurface(0, zinne->w, zinne->h, 32, 0x00FF0000, 0xFF00, 0xFF, 0);
  srand(10293);
  // fill with random values
  //
  for (x = 0; x < brickn->w; x++)
      for (y = 0; y < brickn->h; y++)
          for (t = 0; t < brickn->format->BytesPerPixel; t++)
              *((uint8_t*)brickn->pixels + (y*brickn->pitch + x * brickn->format->BytesPerPixel + t)) = 128 + rand() % 61 - 30;

  // do some smoothing passes
  for (int i = 0; i < 2; i++)
  {
      for (t = 0; t < brickn->format->BytesPerPixel; t++)
      {
          std::vector<uint8_t> sm(brickn->w * brickn->h);

          for (x = 0; x < brickn->w; x++)
              for (y = 0; y < brickn->h; y++)
              {
                  int a = 0;
                  int b = 0;

                  for (int dx = -1; dx <= 1; dx++)
                      for (int dy = -1; dy <= 1; dy++)
                          if (x+dx > 0 && x+dx < brickn->w && y+dy > 0 && y+dy < brickn->h)
                          {
                              a += *((uint8_t*)brickn->pixels + ((y+dy)*brickn->pitch + (x+dx) * brickn->format->BytesPerPixel + t));
                              b++;
                          }

                  sm[y*brickn->w+x] = a/b;
              }
          for (x = 0; x < brickn->w; x++)
              for (y = 0; y < brickn->h; y++)
                  *((uint8_t*)brickn->pixels + (y*brickn->pitch + x * brickn->format->BytesPerPixel + t)) = sm[y*brickn->w+x];
      }
  }

  // add a bevel around the border
  for (y = 0; y < brickn->h; y++)
  {
      *((uint8_t*)brickn->pixels + (y*brickn->pitch + 0*brickn->format->BytesPerPixel + 1)) = 128;
      *((uint8_t*)brickn->pixels + (y*brickn->pitch + 0*brickn->format->BytesPerPixel + 0)) = 128;
      *((uint8_t*)brickn->pixels + (y*brickn->pitch + 1*brickn->format->BytesPerPixel + 0)) = 128-40;
      *((uint8_t*)brickn->pixels + (y*brickn->pitch + (brickn->w-1)*brickn->format->BytesPerPixel + 0)) = 128+40;
  }
  for (x = 0; x < brickn->w; x++)
  {
      *((uint8_t*)brickn->pixels + (0*brickn->pitch + x*brickn->format->BytesPerPixel + 0)) = 128;
      *((uint8_t*)brickn->pixels + (0*brickn->pitch + x*brickn->format->BytesPerPixel + 1)) = 128;
      *((uint8_t*)brickn->pixels + (1*brickn->pitch + x*brickn->format->BytesPerPixel + 1)) = 128-40;
      *((uint8_t*)brickn->pixels + ((brickn->h-1)*brickn->pitch + x * brickn->format->BytesPerPixel + 1)) = 128+40;
  }

  // same for the battlement texture
  for (x = 0; x < zinnen->w; x++)
      for (y = 0; y < zinnen->h; y++)
          for (t = 0; t < zinnen->format->BytesPerPixel; t++)
              *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + x * zinnen->format->BytesPerPixel + t)) = 128 + rand() % 61 - 30;

  // do some smoothing passes
  for (int i = 0; i < 2; i++)
  {
      for (t = 0; t < zinnen->format->BytesPerPixel; t++)
      {
          std::vector<uint8_t> sm(zinnen->w * zinnen->h);

          for (x = 0; x < zinnen->w; x++)
              for (y = 0; y < zinnen->h; y++)
              {
                  int a = 0;
                  int b = 0;

                  for (int dx = -1; dx <= 1; dx++)
                      for (int dy = -1; dy <= 1; dy++)
                          if (x+dx > 0 && x+dx < zinnen->w && y+dy > 0 && y+dy < zinnen->h)
                          {
                              a += *((uint8_t*)zinnen->pixels + ((y+dy)*zinnen->pitch + (x+dx) * zinnen->format->BytesPerPixel + t));
                              b++;
                          }

                  sm[y*zinnen->w+x] = a/b;
              }
          for (x = 0; x < zinnen->w; x++)
              for (y = 0; y < zinnen->h; y++)
                  *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + x * zinnen->format->BytesPerPixel + t)) = sm[y*zinnen->w+x];
      }
  }

  for (y = 0; y < zinnen->h; y++)
  {
      if (y < 21)
      {
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 13*zinnen->format->BytesPerPixel + 1)) = 128;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 13*zinnen->format->BytesPerPixel + 0)) = 128;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 14*zinnen->format->BytesPerPixel + 0)) = 128-40;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + (zinnen->w-14)*zinnen->format->BytesPerPixel + 0)) = 128+40;
      }
      else
      {
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 0*zinnen->format->BytesPerPixel + 1)) = 128;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 0*zinnen->format->BytesPerPixel + 0)) = 128;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + 1*zinnen->format->BytesPerPixel + 0)) = 128-40;
          *((uint8_t*)zinnen->pixels + (y*zinnen->pitch + (zinnen->w-1)*zinnen->format->BytesPerPixel + 0)) = 128+40;
      }
  }
  for (x = 0; x < zinnen->w; x++)
  {
      if (x < 14 || x > 42)
      {
          *((uint8_t*)zinnen->pixels + (20*zinnen->pitch + x*zinnen->format->BytesPerPixel + 0)) = 128;
          *((uint8_t*)zinnen->pixels + (20*zinnen->pitch + x*zinnen->format->BytesPerPixel + 1)) = 128;
          *((uint8_t*)zinnen->pixels + (21*zinnen->pitch + x*zinnen->format->BytesPerPixel + 1)) = 128-40;
          *((uint8_t*)zinnen->pixels + ((zinnen->h-1)*zinnen->pitch + x * zinnen->format->BytesPerPixel + 1)) = 128+40;
      }
      else
      {
          *((uint8_t*)zinnen->pixels + (0*zinnen->pitch + x*zinnen->format->BytesPerPixel + 0)) = 128;
          *((uint8_t*)zinnen->pixels + (0*zinnen->pitch + x*zinnen->format->BytesPerPixel + 1)) = 128;
          *((uint8_t*)zinnen->pixels + (1*zinnen->pitch + x*zinnen->format->BytesPerPixel + 1)) = 128-40;
          *((uint8_t*)zinnen->pixels + ((zinnen->h-1)*zinnen->pitch + x * zinnen->format->BytesPerPixel + 1)) = 128+40;
      }
  }


  for (int ii = 0; ii < zinne->w * zinne->h; ii++)
    *(Uint32*)((Uint8*)zinne->pixels + ii * zinne->format->BytesPerPixel) =
      *(Uint32*)((Uint8*)zinne->pixels + ii * zinne->format->BytesPerPixel) & 0xffffff;

  for (int ii = 0; ii < brick->w * brick->h; ii++)
    *(Uint32*)((Uint8*)brick->pixels + ii * brick->format->BytesPerPixel) =
      *(Uint32*)((Uint8*)brick->pixels + ii * brick->format->BytesPerPixel) & 0xffffff;

  SDL_Rect r;
  r.w = 640;
  r.h = img_height;
  r.x = 0;
  r.y = 0;

  SDL_FillRect(display, &r, SDL_MapRGBA(display->format, 0,0,255,255));

  /* generate the slices */
  for (t = 0; t < 8; t++)
    createslice(wadd / 8 * t, t*16);

  for (t = 0; t < 8; t++)
    createzinne(t*16*3 + 8*16, wadd / 8 * t);

  /* generate doors and save them */
  for (t = -36; t < 37; t++)
    createdoor(t * wadd / 8, 8*16 + 8*3*16 + (t+36)*16*3, radius, 30, radius, 3*hoehe, 0);

  SDL_Surface *disp2 = colorreduction(display, 256);
  SDL_FreeSurface(display);
  display = disp2;

  /* write out palette for bricks */
  for (t = 0; t < 256; t++) {
    Uint8 c;
    c = display->format->palette->colors[t].g;
    fwrite(&c, 1, 1, outp);
    c = display->format->palette->colors[t].r;
    fwrite(&c, 1, 1, outp);

    if (abs((int)display->format->palette->colors[t].g - (int)display->format->palette->colors[t].g) > 5)
      printf("oops color problems\n");
  }

  /* save the slices */
  for (t = 0; t < 8; t++)
    for (y = 0; y < 16; y++)
      for (x = 0; x < 2*radius; x++)
        writebrickpixel(x, t*16+y, outp);

  /* save the battlement */
  for (t = 0; t < 8; t++)
    for (y = 0; y < 3*16; y++)
      for (x = 0; x < 2*zinnenrad; x++)
        writebrickpixel(x, 8*16 + t*16*3+y, outp);

  /* save doors */
  for (t = -36; t < 37; t++) {
    getdoor( 8*16 + 8*3*16 + (t+36)*16*3, outp);
  }

  /* write out palette for environment */
  for (t = 0; t < 256; t++) {
    Uint8 c = t;
    fwrite(&c, 1, 1, outp);
    fwrite(&c, 1, 1, outp);
  }

  createcyl(steprad, outp);
  createcyl(elevatorrad, outp);
  createcyl(stickrad, outp);

  SDL_Quit();
}


