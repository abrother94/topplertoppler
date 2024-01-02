#include "../environment.pov"

declare pyramide = intersection {
  box {<-3,-3,-3> <3,3,3>}
  plane { z+y ,sqrt(4.5) }
  plane { z-y ,sqrt(4.5) }
  plane { z+x ,sqrt(4.5) }
  plane { z-x ,sqrt(4.5) }
}

union {
  difference {
    box { <-3,-3,-3> <3,3,3> }
    object { pyramide translate -z*3.5 }
    object { pyramide rotate x*90  translate y*3.5 }
    object { pyramide rotate x*180 translate z*3.5 }
    object { pyramide rotate -x*90 translate -y*3.5 }
    object { pyramide rotate y*90  translate -x*3.5 }
    object { pyramide rotate -y*90 translate x*3.5 }

    texture { T2 }
  }

  union {
    torus { 3, 0.2 }
    torus { 3, 0.2 rotate x*90 }
    torus { 3, 0.2 rotate z*90 }
    texture { T1 }
  }

  scale 1/sqrt(3)
  scale 0.9

  rotate x*45
  rotate y*asin(1/sqrt(3))*180/pi
  rotate z*120*rot

}
