#include "../environment.pov"

#declare HK=union {
  intersection {
    union {
      sphere { 0 2 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*0 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*30 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*60 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*90 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*120 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*150 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*180 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*210 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*240 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*270 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*300 }
      cylinder { z*-0.1 z*0.1 2.3 rotate x*60 rotate z*330 }
    }

    box { <-3,-3,0>, <3,3,3> }

    texture { T2 }
  }

  union {
    sphere { <0,2,0.4> 0.4 rotate z*360/6*0 }
    sphere { <0,2,0.4> 0.4 rotate z*360/6*1 }
    sphere { <0,2,0.4> 0.4 rotate z*360/6*2 }
    sphere { <0,2,0.4> 0.4 rotate z*360/6*3 }
    sphere { <0,2,0.4> 0.4 rotate z*360/6*4 }
    sphere { <0,2,0.4> 0.4 rotate z*360/6*5 }

    texture { T1 }
  }
}

#declare cyl = union {
  cone { 0, 0.3, x*2, 0 }

  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (0   + rot * 10) }
  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (120 + rot * 10) }
  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (240 + rot * 10) }
}


union {
  object { HK translate  z*sin(rot*pi) rotate z*(rot*360/6+30) }
  object { HK rotate x*180 translate -z*sin(rot*pi) rotate z*rot*360/6 }
  box { <-1,-1,-1> <1,1,1> rotate z*rot*90 texture { T3 } }
  object { cyl translate x * (sin(rot*3.1415926))  rotate x * rot * 120 rotate z * (rot * 90) texture { T1 } }
  object { cyl translate x * (sin(rot*3.1415926))  rotate x * rot * 120 rotate z * (rot * 90+90) texture { T1 } }
  object { cyl translate x * (sin(rot*3.1415926))  rotate x * rot * 120 rotate z * (rot * 90+180) texture { T1 } }
  object { cyl translate x * (sin(rot*3.1415926))  rotate x * rot * 120 rotate z * (rot * 90+270) texture { T1 } }
  scale 0.9
}
