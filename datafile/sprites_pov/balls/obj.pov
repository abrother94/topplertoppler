#include "colors.inc"
#include "textures.inc"

global_settings { assumed_gamma 1.8 }

#declare rot = clock;
#if (clock >= 1)
#declare rot = clock - 1;
#declare white = 1;
#else
#declare white = 0;
#end


#declare weiss = texture {
  pigment { color rgb 1 }
  finish { ambient 1 }
}

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
  texture {
#if (clock = 0)
  Copper_Texture
#else
#if (clock = 0.5)
  Silver_Texture
#else
  weiss
#end
#end
  }

}

global_settings {
  ambient_light <1,1,1>
  assumed_gamma 1
}
