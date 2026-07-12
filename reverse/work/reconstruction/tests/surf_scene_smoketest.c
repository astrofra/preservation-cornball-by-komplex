#include "cornball/random.h"
#include "cornball/surf_scene.h"

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
    CornballSurfScene scene;
    CornballSurfFrame frame;

    cornball_random_seed(&random, 1u);
    cornball_surf_scene_clear(&scene);
    cornball_surf_scene_loader_pass(&scene);

    assert_equal_u32("texture_group_loaded", scene.texture_group_loaded, 1u);

    cornball_surf_scene_step_frame(&scene, &random, 0.0, &frame);

    assert_close("step0.burned_rand_unit", frame.burned_rand_unit, 0.001251221f, 0.000001f);
    assert_equal_u32("step0.fog.enabled", frame.fog.enabled, 1u);
    assert_close("step0.fog.density", frame.fog.density, 0.2f, 0.000001f);
    assert_close("step0.fog.end", frame.fog.end_distance, 65.0f, 0.000001f);
    assert_equal_u32("step0.tube.enabled", frame.tube_shell.enabled, 1u);
    assert_close("step0.translate_x", frame.tube_shell.translate_x, 3.0f, 0.000001f);
    assert_close("step0.translate_y", frame.tube_shell.translate_y, 3.0f, 0.000001f);
    assert_close("step0.rotate_z_pre", frame.tube_shell.rotate_z_pre_degrees, 0.0f, 0.000001f);
    assert_close("step0.rotate_x", frame.tube_shell.rotate_x_degrees, 0.0f, 0.000001f);
    assert_close("step0.rotate_z_post", frame.tube_shell.rotate_z_post_degrees, 0.0f, 0.000001f);
    assert_close("step0.phase", (float)frame.tube_shell.phase, 0.0f, 0.000001f);
    assert_equal_u32("step0.fla.enabled", frame.fla_quad_stack.enabled, 1u);
    assert_equal_u32("step0.fla.count", frame.fla_quad_stack.repeat_count, 32u);
    assert_close("step0.fla.base_z", frame.fla_quad_stack.base_translate_z, 0.0f, 0.000001f);
    assert_close("step0.fla.step_z", frame.fla_quad_stack.step_translate_z, -10.0f, 0.000001f);
    assert_close("step0.foreground.depth", frame.surf_foreground_quad.depth_z, -1.0f, 0.000001f);
    assert_close("step0.foreground.max_u", frame.surf_foreground_quad.texcoord_max_u, 2.0f, 0.000001f);
    assert_close("step0.foreground.alpha", frame.surf_foreground_quad.color_a, 0.8f, 0.000001f);
    assert_close("step0.foreground.rotation", frame.surf_foreground_rotation_degrees, 0.0f, 0.000001f);
    assert_equal_u32("step0.overlay.enabled", frame.jitter_overlay_quad.enabled, 1u);
    assert_equal_u32("step0.overlay.call_counter", scene.overlay_state.call_counter, 1u);
    assert_close("step0.overlay.jitter_x", scene.overlay_state.jitter_x, 0.563568115f, 0.000001f);
    assert_close("step0.overlay.jitter_y", scene.overlay_state.jitter_y, 0.193298340f, 0.000001f);
    assert_close("step0.overlay.min_u", frame.jitter_overlay_quad.texcoord_min_u, 0.563568115f, 0.000001f);
    assert_close("step0.overlay.max_u", frame.jitter_overlay_quad.texcoord_max_u, 1.563568115f, 0.000001f);
    assert_close("step0.overlay.min_v", frame.jitter_overlay_quad.texcoord_min_v, 0.193298340f, 0.000001f);
    assert_close("step0.overlay.max_v", frame.jitter_overlay_quad.texcoord_max_v, 1.193298340f, 0.000001f);
    assert_close("step0.overlay.red", frame.jitter_overlay_quad.color_r, 0.4f, 0.000001f);
    assert_close("step0.overlay.blue", frame.jitter_overlay_quad.color_b, 0.2f, 0.000001f);

    cornball_surf_scene_step_frame(&scene, &random, 1.0, &frame);

    assert_close("step1.burned_rand_unit", frame.burned_rand_unit, 0.808715820f, 0.000001f);
    assert_equal_u32("step1.overlay.call_counter", scene.overlay_state.call_counter, 2u);
    assert_close("step1.translate_x", frame.tube_shell.translate_x, 2.940199852f, 0.000001f);
    assert_close("step1.translate_y", frame.tube_shell.translate_y, 2.866009474f, 0.000001f);
    assert_close("step1.rotate_z_pre", frame.tube_shell.rotate_z_pre_degrees, 3.0f, 0.000001f);
    assert_close("step1.rotate_x", frame.tube_shell.rotate_x_degrees, 14.382766724f, 0.000001f);
    assert_close("step1.rotate_z_post", frame.tube_shell.rotate_z_post_degrees, 32.0f, 0.000001f);
    assert_close("step1.phase", (float)frame.tube_shell.phase, -0.3f, 0.000001f);
    assert_close("step1.fla.base_z", frame.fla_quad_stack.base_translate_z, 8.0f, 0.000001f);
    assert_close("step1.foreground.rotation", frame.surf_foreground_rotation_degrees, 11.0f, 0.000001f);
    assert_close("step1.overlay.jitter_x", scene.overlay_state.jitter_x, 0.584991455f, 0.000001f);
    assert_close("step1.overlay.jitter_y", scene.overlay_state.jitter_y, 0.479858398f, 0.000001f);
    assert_close("step1.overlay.min_u", frame.jitter_overlay_quad.texcoord_min_u, 0.584991455f, 0.000001f);
    assert_close("step1.overlay.max_v", frame.jitter_overlay_quad.texcoord_max_v, 1.479858398f, 0.000001f);

    return 0;
}
