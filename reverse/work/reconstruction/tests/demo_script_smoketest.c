#include "cornball/demo_script.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void assert_close(const char *label, double actual, double expected, double epsilon)
{
    if (fabs(actual - expected) > epsilon) {
        fprintf(
            stderr,
            "%s mismatch: actual=%0.9f expected=%0.9f epsilon=%0.9f\n",
            label,
            actual,
            expected,
            epsilon
        );
        exit(1);
    }
}

static void assert_equal_u32(const char *label, unsigned int actual, unsigned int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s mismatch: actual=%u expected=%u\n", label, actual, expected);
        exit(1);
    }
}

static void assert_equal_int(const char *label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s mismatch: actual=%d expected=%d\n", label, actual, expected);
        exit(1);
    }
}

int main(void)
{
    const CornballDemoSceneSlot *slot0;
    const CornballDemoSceneSlot *slot9;
    CornballDemoSceneSelection selection;

    assert_equal_u32("scene_count", (unsigned int)cornball_demo_script_scene_count(), 10u);
    assert_close("total_positions", cornball_demo_script_total_positions(), 41.0, 0.000001);
    assert_close("total_seconds", cornball_demo_script_total_seconds(2.5), 102.5, 0.000001);

    slot0 = cornball_demo_script_scene_slot(0u);
    slot9 = cornball_demo_script_scene_slot(9u);

    if ((slot0 == NULL) || (slot9 == NULL)) {
        fprintf(stderr, "scene slot lookup returned NULL\n");
        return 1;
    }

    assert_equal_u32("slot0.scene", slot0->scene_index, 0u);
    assert_equal_u32("slot0.threshold", slot0->threshold_position, 0x0cu);
    assert_equal_int("slot0.family", slot0->family, CORNBALL_DEMO_FAMILY_INTRO);
    assert_equal_u32("slot9.scene", slot9->scene_index, 9u);
    assert_equal_u32("slot9.threshold", slot9->threshold_position, 0x29u);
    assert_equal_int("slot9.family", slot9->family, CORNBALL_DEMO_FAMILY_FINALE);

    if (strcmp(cornball_demo_family_name(CORNBALL_DEMO_FAMILY_FINALE), "finale") != 0) {
        fprintf(stderr, "family name mismatch for finale\n");
        return 1;
    }

    assert_close("scene0.start", cornball_demo_script_scene_start_position(0u), 0.0, 0.000001);
    assert_close("scene1.start", cornball_demo_script_scene_start_position(1u), 12.0, 0.000001);
    assert_close("scene8.start", cornball_demo_script_scene_start_position(8u), 35.0, 0.000001);
    assert_close("scene9.start", cornball_demo_script_scene_start_position(9u), 38.0, 0.000001);
    assert_close("scene9.duration", cornball_demo_script_scene_duration_positions(9u), 3.0, 0.000001);

    selection = cornball_demo_script_select_from_position_units(0.0);
    assert_equal_int("select0.valid", selection.valid, 1);
    assert_equal_u32("select0.scene", selection.scene_index, 0u);
    assert_equal_int("select0.family", selection.family, CORNBALL_DEMO_FAMILY_INTRO);

    selection = cornball_demo_script_select_from_position_units(11.999);
    assert_equal_int("select11.valid", selection.valid, 1);
    assert_equal_u32("select11.scene", selection.scene_index, 0u);

    selection = cornball_demo_script_select_from_position_units(12.0);
    assert_equal_int("select12.valid", selection.valid, 1);
    assert_equal_u32("select12.scene", selection.scene_index, 1u);
    assert_equal_int("select12.family", selection.family, CORNBALL_DEMO_FAMILY_KAAR);

    selection = cornball_demo_script_select_from_position_units(22.0);
    assert_equal_int("select22.valid", selection.valid, 1);
    assert_equal_u32("select22.scene", selection.scene_index, 3u);
    assert_equal_int("select22.family", selection.family, CORNBALL_DEMO_FAMILY_SURF);

    selection = cornball_demo_script_select_from_position_units(38.0);
    assert_equal_int("select38.valid", selection.valid, 1);
    assert_equal_u32("select38.scene", selection.scene_index, 9u);
    assert_equal_int("select38.family", selection.family, CORNBALL_DEMO_FAMILY_FINALE);

    selection = cornball_demo_script_select_from_music_position(40u);
    assert_equal_int("select40.valid", selection.valid, 1);
    assert_equal_u32("select40.scene", selection.scene_index, 9u);
    assert_equal_int("select40.family", selection.family, CORNBALL_DEMO_FAMILY_FINALE);

    selection = cornball_demo_script_select_from_position_units(41.0);
    assert_equal_int("select41.valid", selection.valid, 0);
    assert_equal_int("select41.family", selection.family, CORNBALL_DEMO_FAMILY_INVALID);

    return 0;
}
