#define CREATOR

#define NULL 0
#define TOP_DATADIR "./"

#include <configuration.h>

#include <archi.h>
#include <archi.cc>
#include <level.h>
#include <level.cc>
#include <decl.h>
#include <decl.cc>

#include <stdio.h>
#include <stdlib.h>

/*
 * This program reads a text-format tower, and outputs
 * the tower in a format that can be used with povray,
 * to generate the main menu background picture towers.
 */
int main(int argc, char **argv) {
  int r, x, y;

  FILE *outf;

  if (argc < 4) {
    fprintf(stderr, "Usage: outfileformat path towerfile [towerfile ...]\n");
    return 1;
  }

  for (r = 3; r < argc; r++) {
    char buf[256];

    sprintf(buf, "%s/%s", argv[2], argv[r]);
    if (!lev_loadtower(buf)) {
      fprintf(stderr, "Couldn't load tower file %s!\n", buf);
      return 2;
    }

    sprintf(buf, argv[1], r-2);

    outf = fopen(buf, "w");

    if (!outf) {
      fprintf(stderr, "Couldn't create file %s!\n", buf);
      return 3;
    }

    fprintf(outf,"\n/* %s */\n", lev_towername().c_str());

    fprintf(outf, "\n#declare farb = color rgb <%d/255,%d/255,%d/255>;\n\n",
            lev_towercol_red(), lev_towercol_green(), lev_towercol_blue());

    fprintf(outf, "#declare turm = union {\n");

    for (y = 0; y < lev_towerrows(); y++) {
      bool has_door = false;
      for (x = 0; x < 16; x++)
        if (lev_is_door(y, x)) has_door = true;

      if (has_door) {
        fprintf(outf,"  difference {\n");
        fprintf(outf,"    layer()\n");
        for (x = 0; x < 16; x++)
          if (lev_is_door(y, x))
            fprintf(outf,"    door(%i)\n", x);
        fprintf(outf,"  }\n");
      } else fprintf(outf,"  layer()\n");

      for (x = 0; x < 16; x++) {
        const char *txt = NULL;
        Uint8 block = lev_set_tower(y, x, 0);
        (void)lev_set_tower(y, x, block);
        switch (block) {
        case TB_STEP:
        case TB_STEP_VANISHER:
        case TB_STEP_LSLIDER:  txt = "step"; break;
        case TB_ELEV_BOTTOM:
        case TB_ELEV_MIDDLE:
        case TB_ELEV_TOP:      txt = "elev"; break;
        case TB_STICK:         txt = "pillar"; break;
        case TB_BOX:           txt = "boxx"; break;
        default: break;
        }
        if (txt)
          fprintf(outf,"  %s(%i)\n", txt, x);
      }
    }
    fprintf(outf,"}\n");
    fclose(outf);
  }
}

