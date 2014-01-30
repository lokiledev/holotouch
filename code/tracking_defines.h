#ifndef TRACKING_DEFINES_H
#define TRACKING_DEFINES_H

typedef struct {
    float x;
    float y;
    float z;
}head_t;


// Settings for rendering engine
// ==================================================================================
#define DEFAULTWIDTH 1920
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

// screen dimensions in cm
#define SCREENHEIGHT 30.80
#define SCREENWIDTH 40.64

// distance from origin (center of screen) to vertical screen edges, horizontal screen edges (in cm)
#define V_SCREENEDGE SCREENHEIGHT/2
#define H_SCREENEDGE SCREENWIDTH/2

#endif // TRACKING_DEFINES_H
