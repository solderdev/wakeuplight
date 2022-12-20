include <BOSL2/std.scad>

$fn=100;

PCB_X = 48.25;
PCB_X_antenna = 6;  // antenna overhang
PCB_antenna_y = 17.9;  // antenna width
PCB_Y = 28.2;
PCB_Z = 1.65;

PCB_z_space = 4; // extra space for top components

PIN_dist = 25.4;
PIN_spacing = 2.54;
PIN_num = 19;  // on each side
PIN_dim = [0.64, 0.64, 6+2.7];
PIN_socket_height = PIN_dim[2]/2;

// centered on top of PCB
USB_width = 7.8;
USB_height = 2.9;
USB_con_width = 11;
USB_con_height = 8.4;

LED_strip_width = 8.0 + 0.5;
LED_strip_height = 1.5;

WALL = 1.2;
WALL_tol = 0.25;

BOT_height = WALL + PIN_dim[2] + PCB_Z;
BOT_x_outside = WALL*2 + WALL_tol*2 + PCB_X + PCB_X_antenna;
BOT_y_outside = WALL*2 + WALL_tol*2 + PCB_Y;

TOP_height = WALL + PCB_z_space + PCB_Z + PIN_dim[2] + 1;
TOP_x_outside = WALL*2 + WALL_tol*2 + BOT_x_outside;
TOP_y_outside = WALL*2 + WALL_tol*2 + BOT_y_outside;

LID_ridge_d = 1.6;
LID_ridge_d_hole = 2;
LID_ridge_z_off = -PIN_dim[2] + 1;  //BOT_height/2 - WALL - PIN_dim[2];
LID_ridge_cutout = 20;

module dummy()
{
  // PCB
  cube([PCB_X, PCB_Y, PCB_Z], anchor=LEFT+BOTTOM);
  // antenna
  translate([PCB_X, 0, PCB_Z])
    cube([PCB_X_antenna, PCB_antenna_y, 1], anchor=LEFT+BOTTOM);
  // USB connector
  translate([-1, 0, PCB_Z])
    cube([5, USB_width, USB_height], anchor=LEFT+BOTTOM);
  
  for (y = [-1, 1])
  {
    translate([PCB_X/2, y*PIN_dist/2, 0])
      xcopies(spacing=PIN_spacing, n=PIN_num) //, sp=[,0,0])
        cube(PIN_dim, anchor=TOP);
  }
}

module usb_cutout()
{
  // USB cutout
  translate([0, 0, PCB_Z + USB_height/2])
    cuboid([20, USB_con_width, USB_con_height], anchor=RIGHT, rounding=1, edges="X", except=TOP);
}

module pin_holes()
{
  // hole for pins
  translate([0, 0, PIN_socket_height/2 - PIN_dim[2]])
    xcopies(PIN_spacing, n=PIN_num) //PCB_X/2 + PIN_spacing * (PIN_num/2-1)
      translate([PCB_X/2, 0, PIN_socket_height/2-0.5]) // - PIN_spacing * floor(PIN_num/2)
        ycopies(PIN_dist)
          cube([PIN_dim[0] + 0.2, PIN_dim[1] + 0.2, PIN_socket_height*2], center=true);
}

module case_bottom()
{
  x_off = BOT_x_outside/2 - WALL - WALL_tol;
  z_off = BOT_height/2 - WALL - PIN_dim[2];
  
  difference()
  {
    union()
    {
      difference()
      {
        // outer shell
        translate([x_off, 0, z_off])
          cuboid([BOT_x_outside,
                  BOT_y_outside,
                  BOT_height], rounding=2-WALL-WALL_tol, edges="Z");
          
        // inner space
        translate([x_off, 0, z_off])
          translate([0, 0, WALL])
            cuboid([PCB_X + PCB_X_antenna + WALL_tol*2, PCB_Y + WALL_tol*2, BOT_height], rounding=1.5, edges=BOT);
      }
      
      // bottom mounts for pins
      xcopies(PCB_X/2 + PIN_spacing * (PIN_num/2-1) + 0.5)
        translate([PCB_X/2, 0, PIN_socket_height/2 - PIN_dim[2]])
          ycopies(PIN_dist + 1)
            cube([3, 3, PIN_socket_height], center=true);
    }
    
    usb_cutout();
    
    // led strip cutout
    led_strip_h_oversize = LED_strip_height + 0.5;
    translate([-WALL_tol - WALL/2 + BOT_x_outside - WALL, 0, led_strip_h_oversize/2 + z_off - BOT_height/2 + WALL]) 
      cube([10, LED_strip_width, led_strip_h_oversize], center=true);
    
    // little rdiges for case top snap-on
    translate([x_off, 0, LID_ridge_z_off])
      ycopies(BOT_y_outside)
        difference()
        {
          cylinder(h=BOT_x_outside * 0.85, d=LID_ridge_d_hole, orient=RIGHT, center=true);
          cube([LID_ridge_cutout, LID_ridge_d_hole*2, LID_ridge_d_hole*2], center=true);
        }
        
    pin_holes();
  }
}

module case_top()
{
  x_off = BOT_x_outside/2 - WALL - WALL_tol;
  z_off = -TOP_height/2 + WALL + PCB_Z + PCB_z_space;
  
  difference()
  {
    translate([x_off, 0, z_off])
    cuboid([TOP_x_outside, TOP_y_outside, TOP_height], rounding=2, except=BOT);
    
    translate([x_off, 0, -WALL + z_off])
      cuboid([TOP_x_outside - WALL*2, TOP_y_outside - WALL*2, TOP_height], rounding=2-WALL, except=BOT);
    
    usb_cutout();
    
    // led strip cutout
    led_strip_h_oversize = LED_strip_height + 20;
    translate([-WALL_tol - WALL/2 + BOT_x_outside - WALL, 
               0, 
               -led_strip_h_oversize/2 + LED_strip_height + BOT_height/2 - WALL - PIN_dim[2] - BOT_height/2 + WALL]) 
      cube([10, LED_strip_width, led_strip_h_oversize], center=true);
    
    // top graphic
    depth = 0.8;
    translate([BOT_x_outside/2, 0, -depth/2 + WALL + PCB_Z + PCB_z_space]) rotate([0, 0, 270])
      linear_extrude(height=depth + 0.01, center=true)
        scale(0.5) offset(0.01)
          import(file = "alarm_clock.svg", center=true);
  }
  
  // positive snap-on ridges
  difference()
  {
    translate([x_off, 0, LID_ridge_z_off])
      ycopies(TOP_y_outside - WALL*2)
        scale([1, 0.8, 1.3])
        difference()
        {
          cylinder(h=BOT_x_outside * 0.8, d=LID_ridge_d, orient=RIGHT, center=true);
          cube([LID_ridge_cutout * 1.1, LID_ridge_d*2, LID_ridge_d*2], center=true);
        }
  }
}


%dummy();


difference()
{
  case_top();
//  translate([90, 0, 0])
//    cube([100, 100, 100], center=true);
}

difference()
{
  case_bottom();
//  translate([90, 0, 0])
//    cube([100, 100, 100], center=true);
}
