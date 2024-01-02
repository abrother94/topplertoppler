#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include <string>

#define MAXFNAMELEN 150 /* pathnames need a bot more space */
#define NUMFILES 256
#define BUFPADDING 0

typedef struct filenode {
  char name[MAXFNAMELEN];   // actual file name
  std::string aname;  // name of file in the archive
  long start, size, compressed;
} filenode;

int filesize(FILE *f) {
  long pos = ftell(f);
  int erg;
  fseek(f, 0, SEEK_END);
  erg = ftell(f);
  fseek(f, pos, SEEK_SET);
  return erg;
}

int calcsize(filenode * files, int count) {
  int sz = 0;

  for (int i = 0; i < count; i++)
    sz += 13 + files[i].aname.size();

  printf("%i files %i headersize\n", count, sz);
  return sz;
}


int main(int argc, char *argv[])
{
  filenode files[NUMFILES];
  int t, i, filecount = 0;
  FILE *f;
  FILE *outf;
  unsigned char *buffer, *buffer2, *buffer3;

  long erg;
  long largestsize = 0;

  if (argc < 2) return 1;

  for (t = 2; (t < argc) && (filecount < NUMFILES); t++) {
    strncpy(files[filecount].name, argv[t], MAXFNAMELEN);

    files[filecount].aname = files[filecount].name;
    files[filecount].aname = files[filecount].aname.substr(files[filecount].aname.rfind("/")+1);

    printf("%s -> %s\n", files[filecount].name, files[filecount].aname.c_str());


    f = fopen(argv[t], "rb");
    if (!f)
      printf("Error reading file %s!\n", argv[t]);
    else {
      files[filecount].size = filesize(f);
      if (largestsize < (files[filecount].size + BUFPADDING))
        largestsize = files[filecount].size + BUFPADDING;
      filecount++;
      fclose(f);
    }
  }


  buffer = (unsigned char *)malloc(largestsize);
  buffer2 = (unsigned char *)malloc(largestsize);
  buffer3 = (unsigned char *)malloc(largestsize);

  if (!buffer || !buffer2 || !buffer3) {
    printf("Error allocating memory!\n");
    return 1;
  }

  outf = fopen(argv[1], "wb");

  if (!outf) {
    printf("Error writing file %s!\n", argv[1]);
    return 1;
  }

  fwrite(&filecount, 1, 1, outf);
  fwrite(files, calcsize(files, filecount), 1, outf);

  for (t = 0; t < filecount; t++) {
    f = fopen(files[t].name, "rb");

    files[t].size = filesize(f);
    files[t].start = ftell(outf);

    fread(buffer, files[t].size, 1, f);

    files[t].compressed = largestsize;
    compress2(buffer2, (uLongf *)&(files[t].compressed), buffer, files[t].size, 9);
    erg = largestsize;
    uncompress(buffer3, (uLongf *)&(erg), buffer2, files[t].compressed);

    if (erg != files[t].size)
      printf("Different sizes (%li <-> %li)\n", erg, files[t].size);

    for (i = 0; i < files[t].size; i++)
      if (buffer[i] != buffer3[i]) {
        printf("Error in compression %i\n", i);
        break;
      }

    fwrite(buffer2, files[t].compressed, 1, outf);

    fclose(f);
  }
  rewind(outf);

  fwrite(&filecount, 1, 1, outf);

  for (t = 0; t < filecount; t++) {
    fwrite(files[t].aname.c_str(), files[t].aname.size()+1, 1, outf);

    unsigned char tmp = (files[t].start >> 0) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].start >> 8) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].start >> 16) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].start >> 24) & 0xff;
    fwrite(&tmp, 1, 1, outf);

    tmp = (files[t].size >> 0) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].size >> 8) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].size >> 16) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].size >> 24) & 0xff;
    fwrite(&tmp, 1, 1, outf);

    tmp = (files[t].compressed >> 0) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].compressed >> 8) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].compressed >> 16) & 0xff;
    fwrite(&tmp, 1, 1, outf);
    tmp = (files[t].compressed >> 24) & 0xff;
    fwrite(&tmp, 1, 1, outf);
  }

  fclose(outf);

  free(buffer);
  free(buffer2);
  free(buffer3);

  return 0;
}


