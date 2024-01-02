#include "../environment.pov"

#local sphere_size = 15*((sin(rot*2*3.141592)+1)/2+0.5);
#local torus_thickness = 4;

#local a1 = 90*(1-cos(rot*3.1415926))/2;

union {
    union {
	sphere {
	    <0, 0, -sphere_size>, sphere_size
	    texture { T2 }
	}
	sphere {
	    <0, 0, sphere_size>, sphere_size
	    texture { T2 }
	}
	sphere {
	    <0, sphere_size, 0>, sphere_size
	    texture { T1 }
	}
	sphere {
	    <0, -sphere_size, 0>, sphere_size
	    texture { T1 }
	}
    }
    union {
	torus {
	    (100-torus_thickness*2)/2, torus_thickness
	    texture { T3 }
	}
	torus {
	    (100-torus_thickness*2)/2, torus_thickness
	    texture { T3 }
	    rotate z*90
	}
	torus {
	    (100-torus_thickness*2)/2, torus_thickness
	    texture { T3 }
	    rotate x*90
	}
        rotate <0, 0, a1>
        rotate <a1, 0, 0>
    }

    scale 0.06
}
