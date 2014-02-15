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

// Head tracking settings
// =====================================

// screen dimensions in mm
#define SCREENHEIGHT 308.0
#define SCREENWIDTH 406.4

// distance from origin (center of screen) to vertical screen edges, horizontal screen edges (in cm)
#define V_SCREENEDGE SCREENHEIGHT/2
#define H_SCREENEDGE SCREENWIDTH/2

#define WEBCAM_FOV (M_PI_4) // 45° of field of view for the webcam

#define DEPTH_ADJUST 7.0f //offset in distance
#define MOVE_SCALE 1.5f // mult

//Parameters initially from wiimote configuration of
//Johny Chung Lee, adapted for the webcam here.

#define AVG_HEAD_MM 150 //head width to change
#define VERTICAL_ANGLE 0// allows to compensate if webcam is not parallel
                        // with the screen
#define WIIMOTE_ADJUST 0 // head height between -100 and 100
#define CAMERA_ABOVE true // camera above the screen generally

#define NB_SAMPLE_FILTER 10 //weighted average of N previous values

#endif // TRACKING_DEFINES_H
