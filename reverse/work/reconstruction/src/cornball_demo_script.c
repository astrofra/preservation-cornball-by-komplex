#include "cornball/demo_script.h"

#include <limits.h>
#include <string.h>

static const CornballDemoSceneSlot kSceneSlots[] = {
    {0u, 0x0cu, CORNBALL_DEMO_FAMILY_INTRO},
    {1u, 0x12u, CORNBALL_DEMO_FAMILY_KAAR},
    {2u, 0x16u, CORNBALL_DEMO_FAMILY_S_PAIR},
    {3u, 0x19u, CORNBALL_DEMO_FAMILY_SURF},
    {4u, 0x1bu, CORNBALL_DEMO_FAMILY_S_PAIR},
    {5u, 0x1eu, CORNBALL_DEMO_FAMILY_FLA},
    {6u, 0x22u, CORNBALL_DEMO_FAMILY_SURF},
    {7u, 0x23u, CORNBALL_DEMO_FAMILY_INTRO},
    {8u, 0x26u, CORNBALL_DEMO_FAMILY_KAAR},
    {9u, 0x29u, CORNBALL_DEMO_FAMILY_FINALE}
};

static CornballDemoSceneSelection make_invalid_selection(void)
{
    CornballDemoSceneSelection selection;

    memset(&selection, 0, sizeof(selection));
    selection.scene_index = UINT_MAX;
    selection.family = CORNBALL_DEMO_FAMILY_INVALID;
    selection.valid = 0;
    return selection;
}

size_t cornball_demo_script_scene_count(void)
{
    return sizeof(kSceneSlots) / sizeof(kSceneSlots[0]);
}

const CornballDemoSceneSlot *cornball_demo_script_scene_slot(size_t index)
{
    if (index >= cornball_demo_script_scene_count()) {
        return NULL;
    }

    return &kSceneSlots[index];
}

double cornball_demo_script_scene_start_position(unsigned int scene_index)
{
    if (scene_index == 0u) {
        return 0.0;
    }

    if ((size_t)scene_index >= cornball_demo_script_scene_count()) {
        return cornball_demo_script_total_positions();
    }

    return (double)kSceneSlots[scene_index - 1u].threshold_position;
}

double cornball_demo_script_scene_duration_positions(unsigned int scene_index)
{
    if ((size_t)scene_index >= cornball_demo_script_scene_count()) {
        return 0.0;
    }

    return (double)kSceneSlots[scene_index].threshold_position -
        cornball_demo_script_scene_start_position(scene_index);
}

double cornball_demo_script_total_positions(void)
{
    return (double)kSceneSlots[cornball_demo_script_scene_count() - 1u].threshold_position;
}

double cornball_demo_script_total_seconds(double position_seconds)
{
    return cornball_demo_script_total_positions() * position_seconds;
}

const char *cornball_demo_family_name(CornballDemoFamily family)
{
    switch (family) {
    case CORNBALL_DEMO_FAMILY_INTRO:
        return "intro";

    case CORNBALL_DEMO_FAMILY_KAAR:
        return "kaar";

    case CORNBALL_DEMO_FAMILY_S_PAIR:
        return "s-pair";

    case CORNBALL_DEMO_FAMILY_SURF:
        return "surf";

    case CORNBALL_DEMO_FAMILY_FLA:
        return "fla";

    case CORNBALL_DEMO_FAMILY_FINALE:
        return "finale";

    case CORNBALL_DEMO_FAMILY_INVALID:
    default:
        return "invalid";
    }
}

CornballDemoSceneSelection cornball_demo_script_select_from_position_units(double position_units)
{
    size_t index;

    if (position_units < 0.0) {
        return make_invalid_selection();
    }

    for (index = 0u; index < cornball_demo_script_scene_count(); ++index) {
        if (position_units < (double)kSceneSlots[index].threshold_position) {
            CornballDemoSceneSelection selection;

            selection.scene_index = kSceneSlots[index].scene_index;
            selection.family = kSceneSlots[index].family;
            selection.valid = 1;
            return selection;
        }
    }

    return make_invalid_selection();
}

CornballDemoSceneSelection cornball_demo_script_select_from_music_position(unsigned int position)
{
    return cornball_demo_script_select_from_position_units((double)position);
}
