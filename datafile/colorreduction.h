typedef struct {
  Uint16 rmin, rmax, gmin, gmax, bmin, bmax;
  Uint32 count;
} boundary;

Uint32 color_cube[256][256][256];
boundary boundaries[256];

Uint8 colors[3*256];

Uint32 count(SDL_Surface *s) {

  Uint32 x, y;

  for (y = 0; y < s->h; y++) {

    for (x = 0; x < s->w; x++) {
      Uint8 r, g, b;
      Uint32 c;

      c = *((Uint32*)(((Uint8*)(s->pixels)) + y*s->pitch + x*s->format->BytesPerPixel));

      SDL_GetRGB(c, s->format, &r, &g, &b);

      color_cube[r][g][b]++;
    }
  }

  return s->h * s->w;
}

void minimize(Uint16 num) {

  Uint16 r, g, b;

  int found;

  if (boundaries[num].rmin< boundaries[num].rmax) {

    /* increase rmin */
    do {
      found = 0;
      for (g = boundaries[num].gmin; g <= boundaries[num].gmax; g++) {
        for (b = boundaries[num].bmin; b <= boundaries[num].bmax; b++) {
          if (color_cube[boundaries[num].rmin][g][b]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].rmin++;
    } while (!found);
  
    /* decrease rmax */
    do {
      found = 0;
      for (g = boundaries[num].gmin; g <= boundaries[num].gmax; g++) {
        for (b = boundaries[num].bmin; b <= boundaries[num].bmax; b++) {
          if (color_cube[boundaries[num].rmax][g][b]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].rmax--;
    } while (!found);
  }

  if (boundaries[num].gmin< boundaries[num].gmax) {
  
    /* increase gmin */
    do {
      found = 0;
      for (r = boundaries[num].rmin; r <= boundaries[num].rmax; r++) {
        for (b = boundaries[num].bmin; b <= boundaries[num].bmax; b++) {
          if (color_cube[r][boundaries[num].gmin][b]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].gmin++;
    } while (!found);
  
    /* decrease gmax */
    do {
      found = 0;
      for (r = boundaries[num].rmin; r <= boundaries[num].rmax; r++) {
        for (b = boundaries[num].bmin; b <= boundaries[num].bmax; b++) {
          if (color_cube[r][boundaries[num].gmax][b]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].gmax--;
    } while (!found);
  }

  if (boundaries[num].bmin< boundaries[num].bmax) {

    /* increase bmin */
    do {
      found = 0;
      for (r = boundaries[num].rmin; r <= boundaries[num].rmax; r++) {
        for (g = boundaries[num].gmin; g <= boundaries[num].gmax; g++) {
          if (color_cube[r][g][boundaries[num].bmin]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].bmin++;
    } while (!found);
  
    /* decrease bmax */
    do {
      found = 0;
      for (r = boundaries[num].rmin; r <= boundaries[num].rmax; r++) {
        for (g = boundaries[num].gmin; g <= boundaries[num].gmax; g++) {
          if (color_cube[r][g][boundaries[num].bmax]) {
            found = 1;
            break;
          }
        }
        if (found) break;
      }
      if (!found)
        boundaries[num].bmax--;
    } while (!found);
  }

  if (boundaries[num].rmin > boundaries[num].rmax)
    boundaries[num].rmin = boundaries[num].rmax = 0;
  if (boundaries[num].gmin > boundaries[num].gmax)
    boundaries[num].gmin = boundaries[num].gmax = 0;
  if (boundaries[num].bmin > boundaries[num].bmax)
    boundaries[num].bmin = boundaries[num].bmax = 0;
}

Uint16 max (Uint16 a, Uint16 b, Uint16 c) {
  if (a > b)
    if (a > c)
      return a;
    else
      return c;
  else
    if (b > c)
      return b;
    else
      return c;
}

Uint16 findbiggest(Uint16 c) {

  int n;
  int largest_block = 0;
  int largest_dim = max(boundaries[0].rmax-boundaries[0].rmin,
                        boundaries[0].gmax-boundaries[0].gmin,
                        boundaries[0].bmax-boundaries[0].bmin);

  for (n = 1; n < c; n++) {
    int ld = max(boundaries[n].rmax-boundaries[n].rmin,
                 boundaries[n].gmax-boundaries[n].gmin,
                 boundaries[n].bmax-boundaries[n].bmin);
    if (ld > largest_dim) {
      largest_block = n;
      largest_dim = ld;
    }
  }

  /*
  int n;
  int largest_block = -1;

  for (n = 0; n < c; n++) {
    if ((boundaries[n].rmax != boundaries[n].rmin) ||
        (boundaries[n].gmax != boundaries[n].gmin) ||
        (boundaries[n].bmax != boundaries[n].bmin))
      if ((largest_block == -1) || (boundaries[n].count > boundaries[largest_block].count))
         largest_block = n;
  } */

  if (largest_block != -1)
    return largest_block;
  else
    return 0;
}

void split(Uint16 block, Uint16 block2) {

  Uint8 color;
  Uint16 x1, x2;
  Uint32 c1, c2;
  Uint16 i1, i2;

  /* find out the color along which to split */
  if (boundaries[block].rmax-boundaries[block].rmin > boundaries[block].gmax-boundaries[block].gmin)
    if (boundaries[block].rmax-boundaries[block].rmin > boundaries[block].bmax-boundaries[block].bmin)
      color = 0;
    else
      color = 2;
  else
    if (boundaries[block].gmax-boundaries[block].gmin > boundaries[block].bmax-boundaries[block].bmin)
      color = 1;
    else
      color = 2;

  switch (color) {
  case 0:
    /* cut along r */
    x1 = boundaries[block].rmin;
    x2 = boundaries[block].rmax;

    if (x1 == x2) {
      boundaries[block2].rmin = boundaries[block2].rmax =
        boundaries[block2].gmin = boundaries[block2].gmax =
        boundaries[block2].bmin = boundaries[block2].bmax = 0;
      break;
    }

    c1 = c2 = 0;

    for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
      for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++) {
        c1 += color_cube[x1][i1][i2];
        c2 += color_cube[x2][i1][i2];
      }

    while (x1+1 < x2) {
      if (c1 < c2) {
        x1++;
        for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
          for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++)
            c1 += color_cube[x1][i1][i2];

      } else {
        x2--;
        for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
          for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++)
            c2 += color_cube[x2][i1][i2];
      }
    }

    boundaries[block2].gmin = boundaries[block].gmin;
    boundaries[block2].gmax = boundaries[block].gmax;
    boundaries[block2].bmin = boundaries[block].bmin;
    boundaries[block2].bmax = boundaries[block].bmax;

    boundaries[block2].rmax = boundaries[block].rmax;
    boundaries[block2].rmin = x2;
    boundaries[block].rmax = x1;

    boundaries[block].count = c1;
    boundaries[block2].count = c2;

    break;
  case 1:
    /* cut along g */
    x1 = boundaries[block].gmin;
    x2 = boundaries[block].gmax;
    c1 = c2 = 0;

    if (x1 == x2) {
      boundaries[block2].rmin = boundaries[block2].rmax =
        boundaries[block2].gmin = boundaries[block2].gmax =
        boundaries[block2].bmin = boundaries[block2].bmax = 0;
      break;
    }

    for (i1 = boundaries[block].rmin; i1 <= boundaries[block].rmax; i1++)
      for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++) {
        c1 += color_cube[i1][x1][i2];
        c2 += color_cube[i1][x2][i2];
      }

    while (x1+1 < x2) {
      if (c1 < c2) {
        x1++;
        for (i1 = boundaries[block].rmin; i1 <= boundaries[block].rmax; i1++)
          for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++)
            c1 += color_cube[i1][x1][i2];

      } else {
        x2--;
        for (i1 = boundaries[block].rmin; i1 <= boundaries[block].rmax; i1++)
          for (i2 = boundaries[block].bmin; i2 <= boundaries[block].bmax; i2++)
            c2 += color_cube[i1][x2][i2];
      }
    }

    boundaries[block2].rmin = boundaries[block].rmin;
    boundaries[block2].rmax = boundaries[block].rmax;
    boundaries[block2].bmin = boundaries[block].bmin;
    boundaries[block2].bmax = boundaries[block].bmax;

    boundaries[block2].gmax = boundaries[block].gmax;
    boundaries[block2].gmin = x2;
    boundaries[block].gmax = x1;

    boundaries[block].count = c1;
    boundaries[block2].count = c2;


    break;
  case 2:
    /* cut along b */
    x1 = boundaries[block].bmin;
    x2 = boundaries[block].bmax;
    c1 = c2 = 0;

    if (x1 == x2) {
      boundaries[block2].rmin = boundaries[block2].rmax =
        boundaries[block2].gmin = boundaries[block2].gmax =
        boundaries[block2].bmin = boundaries[block2].bmax = 0;
      break;
    }

    for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
      for (i2 = boundaries[block].rmin; i2 <= boundaries[block].rmax; i2++) {
        c1 += color_cube[i2][i1][x1];
        c2 += color_cube[i2][i1][x2];
      }

    while (x1+1 < x2) {
      if (c1 < c2) {
        x1++;
        for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
          for (i2 = boundaries[block].rmin; i2 <= boundaries[block].rmax; i2++)
            c1 += color_cube[i2][i1][x1];

      } else {
        x2--;
        for (i1 = boundaries[block].gmin; i1 <= boundaries[block].gmax; i1++)
          for (i2 = boundaries[block].rmin; i2 <= boundaries[block].rmax; i2++)
            c2 += color_cube[i2][i1][x2];
      }
    }

    boundaries[block2].gmin = boundaries[block].gmin;
    boundaries[block2].gmax = boundaries[block].gmax;
    boundaries[block2].rmin = boundaries[block].rmin;
    boundaries[block2].rmax = boundaries[block].rmax;

    boundaries[block2].bmax = boundaries[block].bmax;
    boundaries[block2].bmin = x2;
    boundaries[block].bmax = x1;

    boundaries[block].count = c1;
    boundaries[block2].count = c2;

    break;
  }

  minimize(block);
  minimize(block2);
}


void calc_colors(Uint16 num_colors, Uint32 cnt) {

  Uint16 n, c;

  n = 1;
  boundaries[0].rmin = boundaries[0].gmin = boundaries[0].bmin = 0;
  boundaries[0].rmax = boundaries[0].gmax = boundaries[0].bmax = 255;
  boundaries[0].count = cnt;

  minimize(0);

  while (n < num_colors) {
    printf("%i / %i \r", n, num_colors);
    fflush(stdout);
    c = findbiggest(n);
    split(c, n);
    n++;
  }

  /* ok ne have all the colors write the result in the colors table*/

  for (n = 0; n < num_colors; n++) {
    colors[3*n+0] = (boundaries[n].rmax + boundaries[n].rmin) / 2,
    colors[3*n+1] = (boundaries[n].gmax + boundaries[n].gmin) / 2;
    colors[3*n+2] = (boundaries[n].bmax + boundaries[n].bmin) / 2;
  }
}

void reduce(SDL_Surface *in, SDL_Surface *out, Uint16 num_colors) {
  Uint16 x, y, n;
  Uint32 c;
  Uint8 r, g, b;

  for (y = 0; y < in->h; y++) {
    for (x = 0; x < in->w; x++) {
      c = *(Uint32*)(((Uint8*)(in->pixels))+y*in->pitch+x*in->format->BytesPerPixel);
      SDL_GetRGB(c, in->format, &r, &g, &b);

      for (n = 0; n < num_colors; n++)
        if ((boundaries[n].rmin <= r) && (r <= boundaries[n].rmax) &&
            (boundaries[n].gmin <= g) && (g <= boundaries[n].gmax) &&
            (boundaries[n].bmin <= b) && (b <= boundaries[n].bmax)) {
          ((Uint8*)out->pixels)[y*out->pitch+x] = n;
          break;
        }
    }
  }
  for (n = 0; n < num_colors; n++) {
    out->format->palette->colors[n].r = colors[3*n+0];
    out->format->palette->colors[n].g = colors[3*n+1];
    out->format->palette->colors[n].b = colors[3*n+2];
  }
}

SDL_Surface * colorreduction(SDL_Surface *in_image, Uint16 colors_num) {
  Uint32 c = count(in_image);

  calc_colors(colors_num, c);
  printf("\n");

  SDL_Surface *out_image = SDL_CreateRGBSurface(0, in_image->w, in_image->h, 8, 0, 0, 0, 0);

  reduce(in_image, out_image, colors_num);

  return out_image;
}

