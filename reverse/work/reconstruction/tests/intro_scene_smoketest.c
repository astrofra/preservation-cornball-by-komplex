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
    assert_close("bipyramid.color_r", frame.bipyramid.color_r, 0.300262749f, 0.000001f);
    assert_close("bipyramid.color_g", frame.bipyramid.color_g, 0.309737235f, 0.000001f);
    assert_close("bipyramid.color_b", frame.bipyramid.color_b, 0.100262754f, 0.000001f);
    assert_equal_u32("logotaus.enabled", frame.logotaus_soft_quad.enabled, 1u);
    assert_equal_u32("logotaus.texture_slot", frame.logotaus_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGOTAUS);
    assert_close("logotaus.half_width", frame.logotaus_soft_quad.half_width, 3.2f, 0.000001f);
    assert_close("logotaus.half_height", frame.logotaus_soft_quad.half_height, 2.56f, 0.000001f);
    assert_close("logotaus.color_r", frame.logotaus_soft_quad.color_r, 0.5f, 0.000001f);
    assert_close("logotaus.color_a", frame.logotaus_soft_quad.color_a, 1.0f, 0.000001f);
    assert_close("logotaus.uv0.u", frame.logotaus_soft_quad.texcoords[0].u, 0.99f, 0.000001f);
    assert_close("logotaus.uv1.v", frame.logotaus_soft_quad.texcoords[1].v, 1.0f, 0.000001f);
    assert_close("logotaus.uv2.v", frame.logotaus_soft_quad.texcoords[2].v, 0.99f, 0.000001f);
    assert_equal_u32("logo.enabled", frame.logo_soft_quad.enabled, 1u);
    assert_equal_u32("logo.texture_slot", frame.logo_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGO);
    assert_close("logo.half_width", frame.logo_soft_quad.half_width, 0.6f, 0.000001f);
    assert_close("logo.half_height", frame.logo_soft_quad.half_height, 0.15f, 0.000001f);
    assert_close("logo.color_r", frame.logo_soft_quad.color_r, 3.001251221f, 0.000001f);
    assert_close("logo.color_a", frame.logo_soft_quad.color_a, 0.0f, 0.000001f);
    assert_close("logo.rotation", frame.logo_soft_quad.rotation_degrees, 180.0f, 0.000001f);

    assert_equal_u32("overlay.enabled", frame.overlay_quad.enabled, 1u);
    assert_equal_u32("overlay.texture_slot", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT2);
    assert_equal_u32("overlay.call_counter", scene.overlay_state.call_counter, 1u);
    assert_close("overlay.jitter_x", scene.overlay_state.jitter_x, 0.563568115f, 0.000001f);
    assert_close("overlay.jitter_y", scene.overlay_state.jitter_y, 0.193298340f, 0.000001f);
    assert_close("overlay.uv0.u", frame.overlay_quad.texcoords[0].u, 1.563568115f, 0.000001f);
    assert_close("overlay.uv0.v", frame.overlay_quad.texcoords[0].v, 0.193298340f, 0.000001f);
    assert_close("overlay.uv2.u", frame.overlay_quad.texcoords[2].u, 0.563568115f, 0.000001f);
    assert_close("overlay.uv2.v", frame.overlay_quad.texcoords[2].v, 1.193298340f, 0.000001f);
    assert_close("overlay.color", frame.overlay_quad.color_r, 1.001251221f, 0.000001f);
    assert_close("overlay.alpha", frame.overlay_quad.color_a, 1.001251221f, 0.000001f);

    cornball_intro_scene_step_frame(&scene, &random, 2.5, &frame);

    assert_equal_u32("logotaus.enabled@2.5s", frame.logotaus_soft_quad.enabled, 1u);
    assert_equal_u32("logotaus.texture_slot@2.5s", frame.logotaus_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGOTAUS);
    assert_close("logotaus.half_width@2.5s", frame.logotaus_soft_quad.half_width, 3.15f, 0.000001f);
    assert_close("logotaus.half_height@2.5s", frame.logotaus_soft_quad.half_height, 2.52f, 0.000001f);
    assert_close("logotaus.rotation@2.5s", frame.logotaus_soft_quad.rotation_degrees, 22.5f, 0.000001f);
    assert_equal_u32("logo.enabled@2.5s", frame.logo_soft_quad.enabled, 1u);
    assert_equal_u32("logo.texture_slot@2.5s", frame.logo_soft_quad.texture_slot, CORNBALL_INTRO_TEXTURE_LOGO);
    assert_close("bipyramid.rotate_y@2.5s", frame.bipyramid.rotate_y_degrees, 5.5f, 0.000001f);
    assert_close("bipyramid.rotate_x@2.5s", frame.bipyramid.rotate_x_degrees, 25.0f, 0.000001f);
    assert_close("bipyramid.color_r@2.5s", frame.bipyramid.color_r, 0.469830334f, 0.000001f);
    assert_close("bipyramid.color_g@2.5s", frame.bipyramid.color_g, 0.140169680f, 0.000001f);
    assert_close("bipyramid.color_b@2.5s", frame.bipyramid.color_b, 0.269830316f, 0.000001f);
    assert_close("logo.half_width@2.5s", frame.logo_soft_quad.half_width, 0.9f, 0.000001f);
    assert_close("logo.half_height@2.5s", frame.logo_soft_quad.half_height, 0.225f, 0.000001f);
    assert_close("logo.color_r@2.5s", frame.logo_soft_quad.color_r, 1.692762613f, 0.000001f);
    assert_close("logo.rotation@2.5s", frame.logo_soft_quad.rotation_degrees, 187.5f, 0.000001f);
    assert_close("logo.alpha@2.5s", frame.logo_soft_quad.color_a, 0.0f, 0.000001f);
    assert_equal_u32("overlay.texture_slot@2.5s", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT2);
    assert_equal_u32("overlay.call_counter@2.5s", scene.overlay_state.call_counter, 2u);
    assert_close("overlay.color@2.5s", frame.overlay_quad.color_r, 0.602905273f, 0.000001f);
    assert_close("overlay.jitter_x@2.5s", scene.overlay_state.jitter_x, 0.563568115f, 0.000001f);

    cornball_intro_scene_step_frame(&scene, &random, 5.0, &frame);

    assert_equal_u32("overlay.texture_slot@5s", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT2);
    assert_equal_u32("overlay.call_counter@5s", scene.overlay_state.call_counter, 3u);
    assert_close("overlay.jitter_x@5s", scene.overlay_state.jitter_x, 0.479858398f, 0.000001f);
    assert_close("overlay.jitter_y@5s", scene.overlay_state.jitter_y, 0.350280762f, 0.000001f);
    assert_close("overlay.color@5s", frame.overlay_quad.color_r, 0.316998303f, 0.000001f);
    assert_close("overlay.rotate_y@5s", frame.bipyramid.rotate_y_degrees, 11.0f, 0.000001f);
    assert_close("overlay.rotate_x@5s", frame.bipyramid.rotate_x_degrees, 50.0f, 0.000001f);
    assert_close("bipyramid.color_r@5s", frame.bipyramid.color_r, 0.422848195f, 0.000001f);
    assert_close("logotaus.half_width@5s", frame.logotaus_soft_quad.half_width, 3.1f, 0.000001f);
    assert_close("logo.half_width@5s", frame.logo_soft_quad.half_width, 1.2f, 0.000001f);
    assert_close("logo.half_height@5s", frame.logo_soft_quad.half_height, 0.3f, 0.000001f);
    assert_close("logo.color_r@5s", frame.logo_soft_quad.color_r, 1.024283290f, 0.000001f);
    assert_close("logo.rotation@5s", frame.logo_soft_quad.rotation_degrees, 195.0f, 0.000001f);

    cornball_intro_scene_step_frame(&scene, &random, 5.01, &frame);

    assert_equal_u32("overlay.texture_slot@5.01s", frame.overlay_quad.texture_slot, CORNBALL_INTRO_TEXTURE_TXT1);
    assert_equal_u32("overlay.call_counter@5.01s", scene.overlay_state.call_counter, 4u);
    assert_close("overlay.jitter_x@5.01s", scene.overlay_state.jitter_x, 0.479858398f, 0.000001f);

    return 0;
}
