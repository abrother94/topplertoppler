#include "../environment.pov"

declare pyramide = intersection {
  box {<-3,-3,0> <3,3,3>}
  plane { z+y ,sqrt(4.5) rotate z*0 }
  plane { z+y ,sqrt(4.5) rotate z*72 }
  plane { z+y ,sqrt(4.5) rotate z*144 }
  plane { z+y ,sqrt(4.5) rotate z*216 }
  plane { z+y ,sqrt(4.5) rotate z*288 }
  scale (x+y)*0.3
  scale sin(rot*pi)*0.5+0.5

  texture { T2 }
}


union {
  sphere { <0,0,0> 1.5
    texture { T3 }
  }

  object { pyramide }

  object { pyramide rotate x*45 rotate z*72*0 }
  object { pyramide rotate x*45 rotate z*72*1 }
  object { pyramide rotate x*45 rotate z*72*2 }
  object { pyramide rotate x*45 rotate z*72*3 }
  object { pyramide rotate x*45 rotate z*72*4 }

  object { pyramide rotate x*90 rotate z*72*0   }
  object { pyramide rotate x*90 rotate z*72*0.5 }
  object { pyramide rotate x*90 rotate z*72*1   }
  object { pyramide rotate x*90 rotate z*72*1.5 }
  object { pyramide rotate x*90 rotate z*72*2   }
  object { pyramide rotate x*90 rotate z*72*2.5 }
  object { pyramide rotate x*90 rotate z*72*3   }
  object { pyramide rotate x*90 rotate z*72*3.5 }
  object { pyramide rotate x*90 rotate z*72*4   }
  object { pyramide rotate x*90 rotate z*72*4.5 }

  object { pyramide rotate x*135 rotate z*72*0 }
  object { pyramide rotate x*135 rotate z*72*1 }
  object { pyramide rotate x*135 rotate z*72*2 }
  object { pyramide rotate x*135 rotate z*72*3 }
  object { pyramide rotate x*135 rotate z*72*4 }

  object { pyramide rotate x*180 }

  rotate z*rot*72

  scale 0.9
}

