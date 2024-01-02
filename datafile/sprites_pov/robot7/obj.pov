#include "../environment.pov"

union {

  torus { 1.5 0.5
    texture { T3 }
    rotate y*90
    rotate z*rot*180
    rotate x*45
  }

  torus { 2.5 0.5
    texture { T1 }
    rotate y*90
    rotate z*rot*180+z*33
    rotate -x*33
  }

  sphere { <0,0,0> 1
    texture { T2 }
  }

  scale 0.9
}
