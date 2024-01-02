#include "../sprites_pov/environment.pov"

#if (white = 1)
  #declare TC1 = texture { white_text }
  #declare TC2 = texture { white_text }
#else
  #declare TC1 = texture { pigment { color rgb <0,0,1> } finish { phong 1 ambient 0.3 } }
  #declare TC2 = texture { pigment { color rgb <1,0,0> } finish { phong 1 ambient 0.3 } }
#end

#declare cyl = union {
  cone { 0, 0.3, -x*2, 0 }
  cone { 0, 0.3, x*2, 0 }

  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (0   + rot * 10) }
  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (120 + rot * 10) }
  cone { x, 0.15, 2*x-y*0.7, 0 rotate x * (240 + rot * 10) }

  cone { -x, 0.15, -2*x-y*0.7, 0 rotate x * (0   + rot * 10) }
  cone { -x, 0.15, -2*x-y*0.7, 0 rotate x * (120 + rot * 10) }
  cone { -x, 0.15, -2*x-y*0.7, 0 rotate x * (240 + rot * 10) }
}

union {

  sphere { 0  0.7  texture { TC2 } }

  union {
    object { cyl }
    object { cyl rotate z * 90 }
    object { cyl rotate z * 45}
    object { cyl rotate -z * 45}

    object { cyl rotate y * 90 }
    object { cyl rotate y * 45}
    object { cyl rotate -y * 45}

    object { cyl rotate z * 90  rotate x * 45}
    object { cyl rotate z * 90  rotate -x * 45}

    texture { TC1 }
  }

  rotate x*(260*1.9*rot+0.5)
  rotate y*(260*1.7*rot+0.5)
  rotate z*(260*1.4*rot+0.2)
  
  scale 1.5
}
