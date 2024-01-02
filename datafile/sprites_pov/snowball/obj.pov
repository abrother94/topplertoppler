#include "colors.inc"
#include "textures.inc"

camera {  //  Camera Camera01
  orthographic
  location  <16,0,0>
  sky       z
  up        <0,0,6>
  right     <6,0,0>
  look_at   <0,0,0>
}

light_source { <16,-10,10> color 1 }

sphere {
  0 3
  pigment {
    color White
  }
#if (clock < 1)
  finish { phong 1 ambient 0.3 }
#else
  finish { ambient 1 }
#end
}

