#include "../environment.pov"

#declare spiess = union {
  cylinder { <0,0,0> <2.5,5/3,0> 0.2 }
  cone { <2.5,5/3,0> 0.2 <3,2,0> 0 }
  texture { T2 }
  rotate x*45*sin(rot*2*3.1415926)
}

union {
  cylinder { <-0.2,0,0> <0.2,0,0> 3 texture { T1 } }

  union {
    object { spiess translate y rotate x*360/5*0 }
    object { spiess translate y rotate x*360/5*1 }
    object { spiess translate y rotate x*360/5*2 }
    object { spiess translate y rotate x*360/5*3 }
    object { spiess translate y rotate x*360/5*4 }

    object { spiess translate -2.8*y rotate x*360/5*0 }
    object { spiess translate -2.8*y rotate x*360/5*1 }
    object { spiess translate -2.8*y rotate x*360/5*2 }
    object { spiess translate -2.8*y rotate x*360/5*3 }
    object { spiess translate -2.8*y rotate x*360/5*4 }
  }

  union {
    object { spiess translate y rotate x*360/5*0 }
    object { spiess translate y rotate x*360/5*1 }
    object { spiess translate y rotate x*360/5*2 }
    object { spiess translate y rotate x*360/5*3 }
    object { spiess translate y rotate x*360/5*4 }

    object { spiess translate -2.8*y rotate x*360/5*0 }
    object { spiess translate -2.8*y rotate x*360/5*1 }
    object { spiess translate -2.8*y rotate x*360/5*2 }
    object { spiess translate -2.8*y rotate x*360/5*3 }
    object { spiess translate -2.8*y rotate x*360/5*4 }

    rotate z*180
    rotate x*180
  }

  rotate y*rot*180

  scale 0.9
}
