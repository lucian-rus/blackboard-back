#ifndef SERVERTYPES_H
#define SERVERTYPES_H

/* defines the actions that the user can take. will only use some for now */
typedef enum {
    // drawing actions
    Action_UpdateCoordinates = 0,
    Action_UpdateColor,
    Action_UpdateThickness,

    // moving actions
    Action_ZoomIn = 100,
    Action_ZoomOut, // might consider factor
    Action_ZoomPan, // might consider dir + amount
} serverType_Action_t;

// define enum for each action defined above:
// *update color
// *update coordinates

#endif