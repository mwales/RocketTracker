WALL_THICK = 2;
FEATHER_LENGTH = 54;
FEATHER_HEIGHT = 24;
FEATHER_WIDTH = 26.5;

EXTREME_SIZE = 100;

module tube(length)
{
	rotate(90,[0,1,0])
	cylinder(h=length, r=39/2);
}

module featherFrameKeepout()
{
	//difference()
	//{
		

		translate([0,-FEATHER_WIDTH/2, -FEATHER_HEIGHT/2])
		cube([FEATHER_LENGTH, FEATHER_WIDTH, FEATHER_HEIGHT]);
	//}

	translate([-10,WALL_THICK / 2, FEATHER_HEIGHT/2+WALL_THICK])
	cube([EXTREME_SIZE, EXTREME_SIZE, EXTREME_SIZE]);
	
	translate([-10,-EXTREME_SIZE - WALL_THICK / 2, FEATHER_HEIGHT/2+WALL_THICK])
	cube([EXTREME_SIZE, EXTREME_SIZE, EXTREME_SIZE]);

}

module feather_holder()
{
    difference()
    {
        translate([-WALL_THICK, -WALL_THICK, -WALL_THICK])
        cube([FEATHER_LENGTH + 2 * WALL_THICK, 
              FEATHER_WIDTH + 2 * WALL_THICK, 
              FEATHER_HEIGHT + 2 * WALL_THICK]);
        
        cube([FEATHER_LENGTH, FEATHER_WIDTH, FEATHER_HEIGHT]);
    }
}

module feather_frame_area(length)
{
    difference()
    {
        tube(length);
        
        translate([-1, 
               -(FEATHER_WIDTH + 2 * WALL_THICK)/2 - 0.05,
               -(FEATHER_HEIGHT + 2 * WALL_THICK)/2 - 0.05])
            cube([FEATHER_LENGTH + 2 * WALL_THICK+2, 
                  FEATHER_WIDTH + 2 * WALL_THICK-.1, 
                  FEATHER_HEIGHT + 2 * WALL_THICK-.1]);
        
        translate([5.5, 0, -FEATHER_HEIGHT/2-.5]) 
        cube([12,100,12.5]);
    }
}

module antenna_plate()
{
    difference()
    {
        rotate(90,[0,1,0])
        cylinder(h=6, d=39);
    
        // skirt
        translate([2,0,0])
        rotate(90,[0,1,0])
        cylinder(h=8, d=(39 - 2*WALL_THICK));
        
        // antenna mont
        translate([-2,0,0])
        rotate(90,[0,1,0])
        cylinder(h=6, d=6.5);
    }
}

module front_collar()
{
    rotate(90,[0,1,0])
    rotate_extrude($fn=200)
    polygon( points=[[39/2, 0],[39/2, 7],[39/2 - 10, 7],[39/2 - 10, 5],[39/2-2, 0],[39/2,0]] );
}


module top_assembly()
{
    antenna_plate();
    
    translate([5.9,0,0])
    front_collar();
    
    translate([12.8,0,0])
    feather_frame_area(15);
}

module bottom_plate()
{
    difference()
    {
        rotate(90,[0,1,0])
            cylinder(h=3, d=39);
        
        
        translate([-1,-7.5,-12])
        cube([5, 15,15]);
        
        
        
        
    }
}

module bottom_assembly()
{
    bottom_plate();
    
    translate([2.9,0,0])
    feather_frame_area(15);
}


//top_assembly();
bottom_assembly();

$fn=200;
