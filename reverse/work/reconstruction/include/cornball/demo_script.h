#ifndef CORNBALL_DEMO_SCRIPT_H
#define CORNBALL_DEMO_SCRIPT_H

#include <stddef.h>

typedef enum CornballDemoFamily {
    CORNBALL_DEMO_FAMILY_INVALID = -1,
    CORNBALL_DEMO_FAMILY_INTRO = 0,
    CORNBALL_DEMO_FAMILY_KAAR = 1,
    CORNBALL_DEMO_FAMILY_S_PAIR = 2,
    CORNBALL_DEMO_FAMILY_SURF = 3,
    CORNBALL_DEMO_FAMILY_FLA = 4,
    CORNBALL_DEMO_FAMILY_FINALE = 5
} CornballDemoFamily;

typedef struct CornballDemoSceneSlot {
    unsigned int scene_index;
    unsigned int threshold_position;
    CornballDemoFamily family;
} CornballDemoSceneSlot;

typedef struct CornballDemoSceneSelection {
    unsigned int scene_index;
    CornballDemoFamily family;
    int valid;
} CornballDemoSceneSelection;

size_t cornball_demo_script_scene_count(void);
const CornballDemoSceneSlot *cornball_demo_script_scene_slot(size_t index);
double cornball_demo_script_scene_start_position(unsigned int scene_index);
double cornball_demo_script_scene_duration_positions(unsigned int scene_index);
double cornball_demo_script_total_positions(void);
double cornball_demo_script_total_seconds(double position_seconds);
const char *cornball_demo_family_name(CornballDemoFamily family);
CornballDemoSceneSelection cornball_demo_script_select_from_position_units(double position_units);
CornballDemoSceneSelection cornball_demo_script_select_from_music_position(unsigned int position);

#endif
