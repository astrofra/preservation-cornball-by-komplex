#include "cornball/kaar_scene.h"
#include "cornball/random.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void assert_close(const char *label, float actual, float expected, float epsilon)
{
    if (fabsf(actual - expected) > epsilon) {
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

static void assert_equal_u32(const char *label, unsigned actual, unsigned expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s mismatch: actual=%u expected=%u\n", label, actual, expected);
        exit(1);
    }
}

int main(void)
{
    CornballRandom random;
    CornballRandom forced_random;
    CornballRandom isolated_random;
    CornballKaarScene scene;
    CornballKaarScene forced_scene;
    CornballKaarScene isolated_scene;
    CornballKaarFrame frame;
    CornballKaarFrame forced_frame;
    CornballKaarFrame isolated_frame;

    cornball_random_seed(&random, 1u);
    cornball_random_seed(&forced_random, 1u);
    cornball_random_seed(&isolated_random, 1u);
    cornball_kaar_scene_clear(&scene);
    cornball_kaar_scene_loader_pass(&scene);
    cornball_kaar_scene_clear(&forced_scene);
    cornball_kaar_scene_loader_pass(&forced_scene);
    cornball_kaar_scene_set_force_main_branch(&forced_scene, 1u);
    cornball_kaar_scene_clear(&isolated_scene);
    cornball_kaar_scene_loader_pass(&isolated_scene);
    cornball_kaar_scene_set_isolate_tube_pass(&isolated_scene, 1u);

    assert_equal_u32("texture_group_loaded", scene.texture_group_loaded, 1u);
    assert_equal_u32("forced.texture_group_loaded", forced_scene.texture_group_loaded, 1u);
    assert_equal_u32("forced.force_main_branch", forced_scene.force_main_branch, 1u);
    assert_equal_u32("isolated.isolate_tube_pass", isolated_scene.isolate_tube_pass, 1u);

    cornball_kaar_scene_step_frame(&scene, &random, 1.0, &frame);

    assert_close("step1.gate_unit", frame.gate_unit, 0.001251259f, 0.000001f);
    assert_close("step1.gate_tint", frame.gate_tint, 0.000125126f, 0.000001f);
    assert_equal_u32("step1.jitter.enabled", frame.jitter_overlay_quad.enabled, 0u);
    assert_equal_u32("step1.fog.enabled", frame.fog.enabled, 1u);
    assert_equal_u32("step1.tube.enabled", frame.tube_shell.enabled, 1u);
    assert_close("step1.fog.density", frame.fog.density, 0.2f, 0.000001f);
    assert_close("step1.fog.end", frame.fog.end_distance, 65.0f, 0.000001f);
    assert_close("step1.translate_x", frame.tube_shell.translate_x, 2.940199852f, 0.000001f);
    assert_close("step1.translate_y", frame.tube_shell.translate_y, 2.866009474f, 0.000001f);
    assert_close("step1.rotate_x", frame.tube_shell.rotate_x_degrees, 56.148838043f, 0.000100f);
    assert_close("step1.rotate_y", frame.tube_shell.rotate_y_degrees, 2.0f, 0.000001f);
    assert_close("step1.rotate_z", frame.tube_shell.rotate_z_degrees, 1.0f, 0.000001f);
    assert_close("step1.txt1.green", frame.txt1_center_quad.color_g, 0.300125122f, 0.000001f);
    assert_close("step1.txt1.rotation", frame.txt1_rotation_degrees, 11.0f, 0.000001f);

    cornball_kaar_scene_step_frame(&scene, &random, 2.0, &frame);

    assert_close("step2.gate_unit", frame.gate_unit, 0.563585341f, 0.000001f);
    assert_close("step2.gate_tint", frame.gate_tint, 0.056358535f, 0.000001f);
    assert_equal_u32("step2.fog.enabled", frame.fog.enabled, 0u);
    assert_equal_u32("step2.tube.enabled", frame.tube_shell.enabled, 0u);
    assert_equal_u32("step2.jitter.enabled", frame.jitter_overlay_quad.enabled, 1u);
    assert_equal_u32("step2.overlay.call_counter", scene.overlay_state.call_counter, 1u);
    assert_close("step2.overlay.red", frame.jitter_overlay_quad.color_r, 0.563585341f, 0.000001f);
    assert_close("step2.overlay.jitter_x", scene.overlay_state.jitter_x, 0.193298340f, 0.000001f);
    assert_close("step2.overlay.jitter_y", scene.overlay_state.jitter_y, 0.808715820f, 0.000001f);
    assert_close("step2.overlay.min_u", frame.jitter_overlay_quad.texcoord_min_u, 0.193298340f, 0.000001f);
    assert_close("step2.overlay.max_u", frame.jitter_overlay_quad.texcoord_max_u, 1.193298340f, 0.000001f);
    assert_close("step2.overlay.min_v", frame.jitter_overlay_quad.texcoord_min_v, 0.808715820f, 0.000001f);
    assert_close("step2.overlay.max_v", frame.jitter_overlay_quad.texcoord_max_v, 1.808715820f, 0.000001f);
    assert_close("step2.txt1.green", frame.txt1_center_quad.color_g, 0.356358528f, 0.000001f);
    assert_close("step2.txt1.rotation", frame.txt1_rotation_degrees, 22.0f, 0.000001f);

    cornball_kaar_scene_step_frame(&forced_scene, &forced_random, 1.0, &forced_frame);
    cornball_kaar_scene_step_frame(&forced_scene, &forced_random, 2.0, &forced_frame);

    assert_close("forced.step2.gate_unit", forced_frame.gate_unit, 0.563585341f, 0.000001f);
    assert_equal_u32("forced.step2.fog.enabled", forced_frame.fog.enabled, 1u);
    assert_equal_u32("forced.step2.tube.enabled", forced_frame.tube_shell.enabled, 1u);
    assert_equal_u32("forced.step2.jitter.enabled", forced_frame.jitter_overlay_quad.enabled, 0u);
    assert_equal_u32("forced.step2.overlay.call_counter", forced_scene.overlay_state.call_counter, 1u);
    assert_close("forced.step2.overlay.jitter_x", forced_scene.overlay_state.jitter_x, 0.193298340f, 0.000001f);
    assert_close("forced.step2.overlay.jitter_y", forced_scene.overlay_state.jitter_y, 0.808715820f, 0.000001f);
    assert_close("forced.step2.translate_x", forced_frame.tube_shell.translate_x, 2.763182879f, 0.000001f);
    assert_close("forced.step2.translate_y", forced_frame.tube_shell.translate_y, 2.476006746f, 0.000001f);
    assert_close("forced.step2.rotate_y", forced_frame.tube_shell.rotate_y_degrees, 4.0f, 0.000001f);
    assert_close("forced.step2.txt1.rotation", forced_frame.txt1_rotation_degrees, 22.0f, 0.000001f);

    cornball_kaar_scene_step_frame(&isolated_scene, &isolated_random, 1.0, &isolated_frame);
    cornball_kaar_scene_step_frame(&isolated_scene, &isolated_random, 2.0, &isolated_frame);

    assert_equal_u32("isolated.step2.fog.enabled", isolated_frame.fog.enabled, 1u);
    assert_equal_u32("isolated.step2.tube.enabled", isolated_frame.tube_shell.enabled, 1u);
    assert_equal_u32("isolated.step2.jitter.enabled", isolated_frame.jitter_overlay_quad.enabled, 0u);
    assert_equal_u32("isolated.step2.txt1.enabled", isolated_frame.txt1_center_quad.enabled, 0u);
    assert_equal_u32("isolated.step2.overlay.call_counter", isolated_scene.overlay_state.call_counter, 1u);
    assert_close("isolated.step2.overlay.jitter_x", isolated_scene.overlay_state.jitter_x, 0.193298340f, 0.000001f);
    assert_close("isolated.step2.overlay.jitter_y", isolated_scene.overlay_state.jitter_y, 0.808715820f, 0.000001f);
    assert_close("isolated.step2.txt1.rotation", isolated_frame.txt1_rotation_degrees, 0.0f, 0.000001f);

    return 0;
}
