#include "cornball/intro_scene.h"
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
    CornballIntroScene scene;
    CornballIntroFrame frame;

    cornball_random_seed(&random, 1u);
    cornball_intro_scene_clear(&scene);
    cornball_intro_scene_loader_pass(&scene);

    assert_equal_u32("texture_group_loaded", scene.texture_group_loaded, 1u);

    cornball_intro_scene_step_frame(&scene, &random, 0.0, &frame);

    assert_equal_u32("bipyramid.enabled", frame.bipyramid.enabled, 1u);
    assert_equal_u32("bipyramid.front_slot", frame.bipyramid.front_texture_slot, CORNBALL_INTRO_TEXTURE_V1);
    assert_equal_u32("bipyramid.back_slot", frame.bipyramid.back_texture_slot, CORNBALL_INTRO_TEXTURE_V2);
    assert_close("bipyramid.rotate_x", frame.bipyramid.rotate_x_degrees, 0.0f, 0.000001f);
    assert_close("bipyramid.rotate_y", frame.bipyramid.rotate_y_degrees, 0.0f, 0.000001f);
    assert_close("bipyramid.rotate_z", frame.bipyramid.rotate_z_degrees, 90.0f, 0.000001f);
    assert_close("bipyramid.color_r", frame.bipyramid.color_r, 1.0f, 0.000001f);
    assert_close("bipyramid.color_g", frame.bipyramid.color_g, 1.0f, 0.000001f);
    assert_close("bipyramid.color_b", frame.bipyramid.color_b, 1.0f, 0.000001f);
    assert_equal_u32("logotaus.enabled", frame.logotaus_soft_quad.enabled, 0u);
    assert_equal_u32("logo.enabled", frame.logo_soft_quad.enabled, 0u);

    assert_equal_u32("overlay.enabled", frame.overlay_quad.enabled, 1u);
    assert_equal_u32("overlay.texture_slot", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT2);
    assert_equal_u32("overlay.call_counter", scene.overlay_state.call_counter, 1u);
    assert_close("overlay.jitter_x", scene.overlay_state.jitter_x, 0.563568115f, 0.000001f);
    assert_close("overlay.jitter_y", scene.overlay_state.jitter_y, 0.193298340f, 0.000001f);
    assert_close("overlay.min_u", frame.overlay_quad.texcoord_min_u, -0.436431885f, 0.000001f);
    assert_close("overlay.max_u", frame.overlay_quad.texcoord_max_u, 0.563568115f, 0.000001f);
    assert_close("overlay.min_v", frame.overlay_quad.texcoord_min_v, -0.806701660f, 0.000001f);
    assert_close("overlay.max_v", frame.overlay_quad.texcoord_max_v, 0.193298340f, 0.000001f);
    assert_close("overlay.tint", frame.overlay_quad.color_r, 0.85f, 0.000001f);

    cornball_intro_scene_step_frame(&scene, &random, 2.5, &frame);

    assert_equal_u32("logotaus.enabled@2.5s", frame.logotaus_soft_quad.enabled, 1u);
    assert_equal_u32("logotaus.texture_slot@2.5s", frame.logotaus_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGOTAUS);
    assert_close("logotaus.half_width@2.5s", frame.logotaus_soft_quad.half_width, 1.45f, 0.000001f);
    assert_close("logotaus.alpha@2.5s", frame.logotaus_soft_quad.color_a, 0.55f, 0.000001f);
    assert_equal_u32("logo.enabled@2.5s", frame.logo_soft_quad.enabled, 1u);
    assert_equal_u32("logo.texture_slot@2.5s", frame.logo_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGO);
    assert_close("logo.half_width@2.5s", frame.logo_soft_quad.half_width, 1.05f, 0.000001f);
    assert_close("logo.alpha@2.5s", frame.logo_soft_quad.color_a, 0.65f, 0.000001f);
    assert_close("logo.translate_y@2.5s", frame.logo_soft_quad.translate_y, 0.2f, 0.000001f);
    assert_equal_u32("overlay.texture_slot@2.5s", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT2);
    assert_equal_u32("overlay.call_counter@2.5s", scene.overlay_state.call_counter, 2u);

    cornball_intro_scene_step_frame(&scene, &random, 5.0, &frame);

    assert_equal_u32("overlay.texture_slot@5s", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT1);
    assert_equal_u32("overlay.call_counter@5s", scene.overlay_state.call_counter, 3u);
    assert_close("overlay.jitter_x@5s", scene.overlay_state.jitter_x, 0.479858398f, 0.000001f);
    assert_close("overlay.rotate_y@5s", frame.bipyramid.rotate_y_degrees, 11.0f, 0.000001f);
    assert_close("overlay.rotate_x@5s", frame.bipyramid.rotate_x_degrees, 50.0f, 0.000001f);
    assert_equal_u32("logotaus.enabled@5s", frame.logotaus_soft_quad.enabled, 0u);
    assert_equal_u32("logo.enabled@5s", frame.logo_soft_quad.enabled, 0u);

    return 0;
}
