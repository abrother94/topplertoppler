#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

/* creates a font.dat file out of specially formatted image files
 * there must be 2 files: one containing the color for each pixel
 * belonging to a letter, one containing a mast having the alpha value
 * for each pixel
 *
 * the colors files must contain additional information about the size
 * of each letter in form of specially places pixels. the color of the
 * marker pixels is defines by the pixel in the top left corner of the image
 * the best is, if you have a look at the example image
 */

int ypos, xpos;
Uint8 fontheight = 1;
Uint8 markercolor;
FILE *outp;

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

void get(SDL_Surface *colors, SDL_Surface * mask, unsigned short c)
{
  // xpos und ypos zeigen auf oberes linkes umrandungs pixel */
  // hoehe ist bekannt : 19
  // breite ist zu suchen

  int height = fontheight;
  unsigned char width = 0;
  int i, x, y;
  Uint8 b;

  while (get_color(colors, xpos + width + 1, ypos + 1) != markercolor)
    width++;

  i = 0;

  fwrite(&c, 1, 2, outp);
  fwrite(&width, 1, 1, outp);

  if (c < 0x100)
    printf(" get %c(%i) at pos (%i;%i)\n", c, c, xpos, ypos);
  else
    printf(" get  (%i) at pos (%i;%i)\n", c, xpos, ypos);

  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      b = get_color(colors, xpos + 1 + x, ypos + 1 + y);
      fwrite(&b, 1, 1, outp);
      b = get_alpha(mask, xpos + 1 + x, ypos + 1 + y);
      fwrite(&b, 1, 1, outp);
    }

  xpos += (1 + width);

  while (get_color(colors, xpos + 1, ypos + 1) == markercolor) xpos++;

  if (get_color(colors, xpos + 1, ypos) != markercolor) {
    ypos += 1 + height;
    xpos = 0;
  }
}

int main() {

  SDL_Surface *colors = IMG_LoadPNG_RW(SDL_RWFromFile("font_colors.png", "rb"));
  SDL_Surface *mask = IMG_LoadPNG_RW(SDL_RWFromFile("font_mask.png", "rb"));

  markercolor = get_color(colors, 0, 0);

  outp = fopen("font.dat", "wb");

  write_palette(outp, colors);

  /* ok first lets fins out the pixel height of the fonts, I
   assume that all the rows have the same hight, so the first is ok */

  while (get_color(colors, 1, 1+fontheight) != markercolor)
    fontheight++;

  fwrite(&fontheight, 1, 1, outp);

  /* and not lets get the pixels */
  xpos = ypos = 0;

  get(colors, mask, '0');
  get(colors, mask, '1');
  get(colors, mask, '2');
  get(colors, mask, '3');
  get(colors, mask, '4');
  get(colors, mask, '5');
  get(colors, mask, '6');
  get(colors, mask, '7');
  get(colors, mask, '8');
  get(colors, mask, '9');
  get(colors, mask, 'A');
  get(colors, mask, 'B');
  get(colors, mask, 'C');
  get(colors, mask, 'D');
  get(colors, mask, 'E');
  get(colors, mask, 'F');
  get(colors, mask, 'G');
  get(colors, mask, 'H');
  get(colors, mask, 'I');
  get(colors, mask, 'J');
  get(colors, mask, 'K');
  get(colors, mask, '.');
  get(colors, mask, 'M');
  get(colors, mask, 'N');
  get(colors, mask, 'O');
  get(colors, mask, 'P');
  get(colors, mask, 'Q');
  get(colors, mask, 'R');
  get(colors, mask, 'S');
  get(colors, mask, 'T');
  get(colors, mask, 'U');
  get(colors, mask, 'V');
  get(colors, mask, 'W');
  get(colors, mask, 'X');
  get(colors, mask, 'Y');
  get(colors, mask, 'Z');
  get(colors, mask, 'a');
  get(colors, mask, 'b');
  get(colors, mask, 'c');
  get(colors, mask, 'd');
  get(colors, mask, 'e');
  get(colors, mask, 'f');
  get(colors, mask, 'h');
  get(colors, mask, 'i');
  get(colors, mask, 'j');
  get(colors, mask, 'k');
  get(colors, mask, 'l');
  get(colors, mask, 'm');
  get(colors, mask, 'n');
  get(colors, mask, 'o');
  get(colors, mask, 'p');
  get(colors, mask, 'q');
  get(colors, mask, 'r');
  get(colors, mask, 's');
  get(colors, mask, 't');
  get(colors, mask, 'u');
  get(colors, mask, 'v');
  get(colors, mask, 'w');
  get(colors, mask, 'x');
  get(colors, mask, 'y');
  get(colors, mask, 'z');
  get(colors, mask, 'L');
  get(colors, mask, ',');
  get(colors, mask, '?');
  get(colors, mask, '+');
  get(colors, mask, '*');
  get(colors, mask, 'g');
  get(colors, mask, ':');
  get(colors, mask, ';');
  get(colors, mask, '(');
  get(colors, mask, ')');
  get(colors, mask, '!');
  get(colors, mask, '/');
  get(colors, mask, '%');
  get(colors, mask, '&');
  get(colors, mask, '\'');
  get(colors, mask, '"');
  get(colors, mask, '\x01'); /* fonttoppler */
  get(colors, mask, '\x02'); /* fontpoint */
  get(colors, mask, '\x03'); /* empty tagbox */
  get(colors, mask, '\x04'); /* taged tagbox */
  get(colors, mask, '\x05'); /* pointer up */
  get(colors, mask, '\x06'); /* pointer right */
  get(colors, mask, '\x07'); /* pointer down */
  get(colors, mask, '\x08'); /* pointer left */
  get(colors, mask, '-');
  get(colors, mask, (unsigned char)'ä');
  get(colors, mask, (unsigned char)'ö');
  get(colors, mask, (unsigned char)'ü');
  get(colors, mask, (unsigned char)'Ä');
  get(colors, mask, (unsigned char)'Ö');
  get(colors, mask, (unsigned char)'Ü');
  get(colors, mask, (unsigned char)'ß');

  get(colors, mask, (unsigned char)'é');
  get(colors, mask, (unsigned char)'è');
  get(colors, mask, (unsigned char)'á');
  get(colors, mask, (unsigned char)'à');
  get(colors, mask, (unsigned char)'í');
  get(colors, mask, (unsigned char)'ì');
  get(colors, mask, (unsigned char)'ó');
  get(colors, mask, (unsigned char)'ò');
  get(colors, mask, (unsigned char)'ç');
  get(colors, mask, (unsigned char)'É');
  get(colors, mask, (unsigned char)'È');
  get(colors, mask, (unsigned char)'Á');
  get(colors, mask, (unsigned char)'À');
  get(colors, mask, (unsigned char)'Í');
  get(colors, mask, (unsigned char)'Ì');
  get(colors, mask, (unsigned char)'Ó');
  get(colors, mask, (unsigned char)'Ò');
  get(colors, mask, (unsigned char)'Ç');
  get(colors, mask, (unsigned char)'ñ');
  get(colors, mask, (unsigned char)'Ñ');
  get(colors, mask, (unsigned char)'õ');
  get(colors, mask, (unsigned char)'Õ');
  get(colors, mask, (unsigned char)'î');
  get(colors, mask, (unsigned char)'Î');

  get(colors, mask, (unsigned char)'ê');
  get(colors, mask, (unsigned char)'Ê');
  get(colors, mask, (unsigned char)'â');
  get(colors, mask, (unsigned char)'Â');
  get(colors, mask, (unsigned char)'ô');
  get(colors, mask, (unsigned char)'Ô');
  get(colors, mask, (unsigned char)'û');
  get(colors, mask, (unsigned char)'Û');

  get(colors, mask, 0x108);  // ^C
  get(colors, mask, 0x109);  // ^c
  get(colors, mask, 0x11c);  // ^G
  get(colors, mask, 0x11d);  // ^g
  get(colors, mask, 0x124);  // ^H
  get(colors, mask, 0x125);  // ^h
  get(colors, mask, 0x134);  // ^J
  get(colors, mask, 0x135);  // ^j
  get(colors, mask, 0x15c);  // ^S
  get(colors, mask, 0x15d);  // ^s
  get(colors, mask, 0x168);  // ~U
  get(colors, mask, 0x169);  // ~u
  get(colors, mask, 0xC5);   // °A
  get(colors, mask, 0xE5);   // °a
  get(colors, mask, 0x153);   // oe ½

  get(colors, mask, 0x152);   // OE ¼
  get(colors, mask, 0xC3);   // ~A Ã
  get(colors, mask, 0xE3);   // ~a ã
  get(colors, mask, 0xC6);   // AE Æ
  get(colors, mask, 0xE6);   // ae æ
  get(colors, mask, 0xCB);   // "E Ë
  get(colors, mask, 0xEB);   // "e ë
  get(colors, mask, 0xCF);   // "I Ï
  get(colors, mask, 0xEF);   // "i ï
  get(colors, mask, 0xD0);   // Thorn
  get(colors, mask, 0xF0);   // thorn
  get(colors, mask, 0xD5);   // ~O Õ
  get(colors, mask, 0xF5);   // ~o õ
  get(colors, mask, 0xD8);   // /O Ø
  get(colors, mask, 0xF8);   // /o ø
  get(colors, mask, 0xD9);   // `U Ù
  get(colors, mask, 0xF9);   // `u ù
  get(colors, mask, 0xDA);   // u Ú
  get(colors, mask, 0xFA);   // u ú
  get(colors, mask, 0xDD);   // ¿Y Ý
  get(colors, mask, 0xFD);   // ¿y ý
  get(colors, mask, 0xDE);   // |°
  get(colors, mask, 0xFE);   // |°
  get(colors, mask, 0xFF);   // "y
  
  get(colors, mask, 0x11A);  // inv^ E
  get(colors, mask, 0x11B);  // inv^ e
  get(colors, mask, 0x160);  // inv^ S
  get(colors, mask, 0x161);  // inv^ s
  get(colors, mask, 0x10C);  // inv^ C
  get(colors, mask, 0x10D);  // inv^ c
  get(colors, mask, 0x158);  // inv^ R
  get(colors, mask, 0x159);  // inv^ r
  get(colors, mask, 0x16E);  // °U
  get(colors, mask, 0x16F);  // °u
  get(colors, mask, 0x102);  // A with lower halve circle above
  get(colors, mask, 0x103);  // a with lower halve circle above
  get(colors, mask, 0x21A);  // T with comma below
  get(colors, mask, 0x21B);  // t with comma below
  get(colors, mask, 0x218);  // S with comma below
  get(colors, mask, 0x219);  // s with comma below
  get(colors, mask, 0x17D);  // inv^ Z
  get(colors, mask, 0x17E);  // inv^ z

  fclose(outp);
}

