#include "../environment.pov"

union {
  sphere { <0,0,0> 1.3 texture { T2 } }

  union {
    sphere { <0,2.2,0> 0.8 rotate z*180*rot rotate x*45}
    sphere { <0,-2.2,0> 0.8 rotate z*180*rot rotate -x*45 }
    texture { T1 }
  }

  union {
    sphere { <0,0,2> 0.6 }
    sphere { <0,2,0> 0.6 }
    sphere { <0,0,-2> 0.6 }
    sphere { <0,-2,0> 0.6 }

    texture { T3 }
  }

  rotate z*rot*180
  rotate x*rot*180

  scale 0.9
}
