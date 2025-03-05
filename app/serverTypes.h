#ifndef SERVERTYPES_H
#define SERVERTYPES_H

/* defines the actions that the user can take. will only use some for now */
typedef enum {
    // drawing actions
    Action_UpdateCoordinates = 0u,
    Action_UpdateColor,
    Action_UpdateThickness,
    Action_SetBackground,
    // reserve more draw actions

    // general actions
    Action_Undo = 30u, // <- doing this server side cause it seems fun
    Action_Redo,       // <- doing this server side cause it seems fun
    Action_SetTheme,
    // moving actions
    Action_ZoomIn,
    Action_ZoomOut, // might consider factor
    Action_ZoomPan, // might consider dir + amount
    // reserve for future actions

    Action_Exit = 255u,
} serverType_Action_t;

#endif
