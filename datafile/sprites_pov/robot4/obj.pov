#include "../environment.pov"

declare pyramide = intersection {
  box {<-3,-3,0> <3,3,3>}
  plane { z+y ,sqrt(4.5) rotate z*0 }
  plane { z+y ,sqrt(4.5) rotate z*72 }
  plane { z+y ,sqrt(4.5) rotate z*144 }
  plane { z+y ,sqrt(4.5) rotate z*216 }
  plane { z+y ,sqrt(4.5) rotate z*288 }
  scale 0.5
  scale z*2
  texture { T1 }
}

declare skrew = difference {
  cylinder { <0, 0, -1> < 0, 0, 1> 0.5 }
  box { <-0.1, -2, 2> <0.1, 2, 0.5> }
  box { <-0.1, -2, -2> <0.1, 2, -0.5> }
}

#local a1 = 2*72*(1-cos(rot*3.1415926))/2;

union {
  cylinder { <0,0,-0.1> <0,0,0.1> 3 texture { T2 } }

  object { pyramide }
  object { pyramide rotate x*180 rotate z*36 }
  union {
    object { skrew rotate z * 34 translate y*2.5 rotate z*360/10*0 }
    object { skrew rotate z * 14 translate y*2.5 rotate z*360/10*1 }
    object { skrew rotate z * 47 translate y*2.5 rotate z*360/10*2 }
    object { skrew rotate z * 32 translate y*2.5 rotate z*360/10*3 }
    object { skrew rotate z * 25 translate y*2.5 rotate z*360/10*4 }
    object { skrew rotate z * 75 translate y*2.5 rotate z*360/10*5 }
    object { skrew rotate z * 99 translate y*2.5 rotate z*360/10*6 }
    object { skrew rotate z * 65 translate y*2.5 rotate z*360/10*7 }
    object { skrew rotate z * 44 translate y*2.5 rotate z*360/10*8 }
    object { skrew rotate z * 35 translate y*2.5 rotate z*360/10*9 }

    texture { T3 }
  }

  rotate z*a1
  rotate y*rot*180
  scale 0.9
}

