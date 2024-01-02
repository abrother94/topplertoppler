#include "colors.inc"
#include "textures.inc"

camera {  //  Camera Camera01
  orthographic
  location  <16,0,3.5>
  sky       z
  up        <0,0,7>
  right     <8,0,0>
  look_at   <0,0,3.5>
}

light_source { <16,-10,10> color 1 }

difference {
  cylinder { <0,0,0> <0,0,7> 4 }
#if (clock < 1)
  pigment { color Orange }
  finish {
    phong 1
    ambient sin(clock*pi)
  }
#else
  pigment {color rgb 1}
  finish {
    ambient 1
    }
#end
}
