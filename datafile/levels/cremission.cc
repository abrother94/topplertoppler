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
 * all right this program creates a mission from given towers
 *
 * first argument: mission name
 * second argument: priority in mission list
 * rest: filenames of towers
 */

int main(int argn, char *args[]) {

  int i;

  if (argn < 6) {
      printf("Usage: %s mission_name mission_file_name priority textoutputfile towerfile [towerfile ...]\n", args[0]);
      return 1;
  }

  printf("create %s\n", args[1]);
  lev_mission_new(args[1], args[2], atoi(args[3]));

  FILE * names = fopen(args[4], "w");

  fprintf(names, "_(\"%s\")\n", args[1]);

  for (i = 5; i < argn; i++) {
    printf("add %s\n", args[i]);
    lev_mission_addtower(args[i]);

    fprintf(names, "/* Tower name, you can translate freely */\n_(\"%s\")\n", lev_towername().c_str());
  }

  printf("finish\n");
  lev_mission_finish();
}
