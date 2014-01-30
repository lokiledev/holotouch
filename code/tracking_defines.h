#ifndef TRACKING_DEFINES_H
#define TRACKING_DEFINES_H

typedef struct {
    float x;
    float y;
    float z;
}head_t;


// Settings for rendering engine
// ==================================================================================
#define DEFAULTWIDTH 1920 //screen info
#define DEFAULTHEIGHT 1080

// maximum view distance
#define VIEWFARPLANE 500.0


// Head tracking settings
// =====================================

// this defines the units we will be using for the coordinate system.
// use #define UNITS xxx where xxx is the number of your units in a real meter.
// for example, if you want to use centimeters, you would write #define UNITS 100.0
#define UNITS 100.0
// from here on, the units will be referred to as cm, since this is the default.

// screen dimensions in mm
#define SCREENHEIGHT 308.0
#define SCREENWIDTH 406.4

// distance from origin (center of screen) to vertical screen edges, horizontal screen edges (in cm)
#define V_SCREENEDGE SCREENHEIGHT/2
#define H_SCREENEDGE SCREENWIDTH/2

#define WEBCAM_FOV (M_PI_4) // 45° of field of view for the webcam

#define DEPTH_ADJUST 5.0 //affects distance from scene

//Parameters initially from wiimote configuration of
//Johny Chung Lee, adapted for the webcam here.

#define AVG_HEAD_MM 150 //head width to change
#define VERTICAL_ANGLE 0// allows to compensate if webcam is not parallel
                        // with the screen
#define WIIMOTE_ADJUST 0 // head height between -100 and 100
#define CAMERA_ABOVE true // camera above the screen generally

#define ANTI_FLICKER_TRESHOLD 20 // head must move more than 20 pixels to update

#endif // TRACKING_DEFINES_H
