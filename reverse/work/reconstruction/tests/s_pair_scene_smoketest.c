#include "cornball/random.h"
#include "cornball/s_pair_scene.h"

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
    CornballSPairScene scene;
    CornballSPairFrame frame;

    cornball_random_seed(&random, 1u);
    cornball_s_pair_scene_clear(&scene);
    cornball_s_pair_scene_loader_pass(&scene);

    assert_equal_u32("texture_group_loaded", scene.texture_group_loaded, 1u);

    cornball_s_pair_scene_step_frame(&scene, &random, 0.0, &frame);

    assert_close("step1.rand_unit", frame.rand_unit, 0.001251221f, 0.000001f);
    assert_equal_u32("step1.bipyramid.enabled", frame.bipyramid.enabled, 1u);
    assert_close("step1.rotate_z_pre", frame.bipyramid.rotate_z_pre_degrees, -90.0f, 0.000001f);
    assert_close("step1.rotate_y", frame.bipyramid.rotate_y_degrees, 0.0f, 0.000001f);
    assert_close("step1.rotate_x", frame.bipyramid.rotate_x_degrees, 97.0f, 0.000001f);
    assert_close("step1.rotate_z_post", frame.bipyramid.rotate_z_post_degrees, 0.0f, 0.000001f);
    assert_close("step1.blue", frame.bipyramid.color_b, 0.500375390f, 0.000001f);
    assert_equal_u32("step1.overlay.enabled", frame.overlay_quad.enabled, 1u);
    assert_equal_u32("step1.overlay.call_counter", scene.overlay_state.call_counter, 1u);
    assert_close("step1.overlay.jitter_x", scene.overlay_state.jitter_x, 0.563568115f, 0.000001f);
    assert_close("step1.overlay.jitter_y", scene.overlay_state.jitter_y, 0.193298340f, 0.000001f);
    assert_close("step1.overlay.min_u", frame.overlay_quad.texcoord_min_u, 0.563568115f, 0.000001f);
    assert_close("step1.overlay.max_u", frame.overlay_quad.texcoord_max_u, 1.563568115f, 0.000001f);
    assert_close("step1.overlay.min_v", frame.overlay_quad.texcoord_min_v, 0.193298340f, 0.000001f);
    assert_close("step1.overlay.max_v", frame.overlay_quad.texcoord_max_v, 1.193298340f, 0.000001f);
    assert_close("step1.overlay.red", frame.overlay_quad.color_r, 2.041251183f, 0.000001f);
    assert_close("step1.overlay.green", frame.overlay_quad.color_g, 2.001251221f, 0.000001f);
    assert_close("step1.overlay.alpha", frame.overlay_quad.color_a, 2.001251221f, 0.000001f);

    cornball_s_pair_scene_step_frame(&scene, &random, 2.0, &frame);

    assert_close("step2.rand_unit", frame.rand_unit, 0.808740497f, 0.000001f);
    assert_close("step2.rotate_y", frame.bipyramid.rotate_y_degrees, 31.134347916f, 0.000100f);
    assert_close("step2.rotate_x", frame.bipyramid.rotate_x_degrees, 80.057556152f, 0.000100f);
    assert_close("step2.rotate_z_post", frame.bipyramid.rotate_z_post_degrees, 36.373386383f, 0.000100f);
    assert_close("step2.blue", frame.bipyramid.color_b, 0.742622137f, 0.000001f);
    assert_equal_u32("step2.overlay.enabled", frame.overlay_quad.enabled, 1u);
    assert_equal_u32("step2.overlay.call_counter", scene.overlay_state.call_counter, 2u);
    assert_close("step2.overlay.red", frame.overlay_quad.color_r, 0.866100132f, 0.000001f);
    assert_close("step2.overlay.green", frame.overlay_quad.color_g, 0.826100171f, 0.000001f);
    assert_close("step2.overlay.min_u", frame.overlay_quad.texcoord_min_u, 0.563568115f, 0.000001f);
    assert_close("step2.overlay.max_u", frame.overlay_quad.texcoord_max_u, 1.563568115f, 0.000001f);

    return 0;
}
