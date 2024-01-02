#declare klink=box { <40,-9,0>, <48,9,7.5> }

/*
global_settings{
  assumed_gamma 1.0
  radiosity{
    pretrace_start 0.04
    pretrace_end 0.01
    count 200
    recursion_limit 3
    nearest_count 10
    error_bound 0.5
  }
}
*/

#declare scheibe = union {
  #local klinkcount = 0;
  #while (klinkcount < 16)
     object { klink rotate klinkcount*(360/16)*z }
     #declare klinkcount = klinkcount + 1;
  #end
}




#declare zinne = difference {
  box { <61, -14, 0 >, <72, 14, 24> }
  box { <60, -15, 12>, <73, -7, 25> }
  box { <60,   7, 12>, <73, 15, 25> }
}

#declare zinnen = union {
  #local zinnecount = 0;
  #while (zinnecount < 16)
    object { zinne rotate zinnecount*z*(360/16) }
    #declare zinnecount = zinnecount + 1;
  #end
  cylinder { <0,0,0> <0,0,12> 65 }
}

#declare stufe=cylinder { <58,0,0>, <58,0,7>, 10 texture { pigment { color rgb 0.7 } } }
#declare stutz=cylinder { <58,0,0>, <58,0,7>, 4  texture { pigment { color rgb 0.7 } } }
#declare eleva=cylinder { <58,0,0>, <58,0,7>, 8  texture { pigment { color rgb 0.7 } } }

#macro brick_tex()
  texture {
    pigment { farb }
    normal { bumps 0.3 scale 0.5 }
  }
#end

#declare moertel = texture {
  pigment { color rgb <0.7,0.7,0.7> }
  normal { bumps 0.2 scale 0.4 }
}

#declare current_layer = 0;

#macro battlement()
  object { zinnen translate z*8*current_layer brick_tex() }
#end /* battlement */

/*
 * macros the towers must use
 */
#macro step(dx)
  object { stufe rotate z*(360/16)*dx translate z*8*(current_layer-1) }
#end /* step */

#macro elev(dx)
  object { eleva rotate z*(360/16)*dx translate z*8*(current_layer-1) }
#end /* eleva */

#macro pillar(dx)
  object { stutz rotate z*(360/16)*dx translate z*8*(current_layer-1) }
#end /* pillar */

#macro door(dx)
  box { <-50,-10, -0.001>, <50,10,8.002> rotate z*(360/16)*dx translate z*8*(current_layer-1) brick_tex() }
#end /* door */

#macro boxx(dx)
  object {
      cylinder { <58,0,0>, <58,0,7>, 4
	    texture {
		pigment {
		    color red 1 green 0.5 blue 0.0
		}
		finish {
		    phong 1
		    ambient 0.4
		}
	    }
      }
      rotate z*(360/16)*dx
      translate z*8*(current_layer-1)
  }
#end /* boxx */

/* add a layer to the tower */
#macro layer()
  union {
    object { scheibe rotate z*11.25*mod((current_layer+1), 2) translate z*8*current_layer brick_tex() }
    cylinder { <0, 0, 0>, <0,0,8.001>, 48 open translate z*8*current_layer texture { moertel } }
  }
  #declare current_layer = current_layer + 1;
#end /* layer */

/*
 * Include a tower from file.
 *
 * In the file you must #declare 2 variables: "farb" and "turm".
 * farb is of type color, and turm should be a union, where you
 * use only the above declared macroes.
 * eg.
 * #declare farb = color rgb <0.5, 0, 1>;
 * #declare turm = union { layer() step(0) step(1) layer() layer() };
 *
 */
#macro load_tower(towerfname)
  object {
    #include towerfname
    union {
      object { turm }
      object { battlement() }
    }
//    bounded_by { cylinder { <0, 0, 0>, <0, 0, 8*(current_layer+3)+32>, 80 } }
    #declare current_layer = 0;
  }
#end /* load_tower */


camera {
  location <700,-300,300>
  up z
  right 8/3*x
  sky z
  look_at <0,500,300>
}

light_source { <1300,1700,300> color rgb <1,1,1> }

plane {
  z, 0
  texture {
    pigment { color rgb <0,0,0.4> }
    finish {
      reflection 0.8
    }

    normal { waves scale 10  }
    normal { wrinkles scale 3  }
  }
}

union {
  sphere { <0,0,0>,50000
     pigment { color rgb 1 }
     normal { bumps 200 scale 4500 }
     finish { ambient 0.8 }
  }
  intersection {
    sphere { <0,0,0>,50001 }
    box { <-50001,-50001,-50001>, <0,50001,50001>}
    pigment { color rgb <0,0,0.15> }
    normal { bumps 200 scale 4500 }
    finish { ambient 0.4 }
  }
  rotate z*50
  rotate -x*35
  translate <0,500000,60000>
  rotate z*14
  pigment { color rgb 1 }
  finish {
    diffuse 0
    phong 0
  }
}

#declare num_towers = 8;

#declare current_tower = 0;
#while (current_tower < num_towers)
  #declare towerfname = concat("turm", str(current_tower+1, 0, 0), ".inc");
  object {
    load_tower(towerfname)
    translate <0, 200*current_tower, 0>
  }
  #declare current_tower = current_tower + 1;
#end

sky_sphere { pigment { color rgb <0, 0, 0.05> } }
