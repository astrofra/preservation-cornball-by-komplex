#include "cornball/s_pair_scene.h"

#include <math.h>
#include <string.h>

static const float kRandScale = 3.05185094759972e-05f;
static const float kOverlayHalfExtent = 2.0f;
static const float kOverlayDepth = -2.0f;
static const float kOverlayVisibilityThreshold = 0.005f;
static const float kBipyramidBlueBase = 0.5f;
static const float kBipyramidBlueScale = 0.3f;
static const float kOverlayRedBias = 0.04f;
static const double kRotateYRate = 0.5;
static const double kRotateYAmp = 37.0;
static const double kRotateXRate = 0.3;
static const double kRotateXAmp = 97.0;
static const double kRotateZLinearRate = 16.2;
static const double kRotateZWobbleRate = 0.1;
static const double kRotateZWobbleAmp = 20.0;
static const double kOverlayDenominatorRate = 1.2;
static const double kOverlayIntensityBias = 2.0;
static const uint32_t kOverlayReseedMask = 1u;

static void fill_layer_quad(
    CornballSPairLayerQuad *quad,
    float half_width,
    float half_height,
    float depth_z,
    float color_r,
    float color_g,
    float color_b,
    float color_a,
    float texcoord_min_u,
    float texcoord_max_u,
    float texcoord_min_v,
    float texcoord_max_v
)
{
    quad->enabled = 1u;
    quad->half_width = half_width;
    quad->half_height = half_height;
    quad->depth_z = depth_z;
    quad->color_r = color_r;
    quad->color_g = color_g;
    quad->color_b = color_b;
    quad->color_a = color_a;
    quad->texcoord_min_u = texcoord_min_u;
    quad->texcoord_max_u = texcoord_max_u;
    quad->texcoord_min_v = texcoord_min_v;
    quad->texcoord_max_v = texcoord_max_v;
}

static void disable_layer_quad(CornballSPairLayerQuad *quad)
{
    memset(quad, 0, sizeof(*quad));
}

static float build_overlay_intensity(float rand_unit, double scene_elapsed_seconds)
{
    return (float)((rand_unit + kOverlayIntensityBias) / (1.0 + scene_elapsed_seconds * kOverlayDenominatorRate));
}

static void build_bipyramid_pass(float rand_unit, double scene_elapsed_seconds, CornballSPairBipyramidPass *pass)
{
    memset(pass, 0, sizeof(*pass));
    pass->enabled = 1u;
    pass->rotate_z_pre_degrees = -90.0f;
    pass->rotate_y_degrees = (float)(sin(scene_elapsed_seconds * kRotateYRate) * kRotateYAmp);
    pass->rotate_x_degrees = (float)(cos(scene_elapsed_seconds * kRotateXRate) * kRotateXAmp);
    pass->rotate_z_post_degrees = (float)(
        scene_elapsed_seconds * kRotateZLinearRate +
        sin(scene_elapsed_seconds * kRotateZWobbleRate) * kRotateZWobbleAmp
    );
    pass->color_r = 1.0f;
    pass->color_g = 1.0f;
    pass->color_b = kBipyramidBlueBase + rand_unit * kBipyramidBlueScale;
    pass->color_a = 1.0f;
}

static void build_overlay_pass(
    CornballSPairScene *scene,
    CornballRandom *random,
    float overlay_intensity,
    CornballSPairLayerQuad *quad
)
{
    if (overlay_intensity <= kOverlayVisibilityThreshold) {
        disable_layer_quad(quad);
        return;
    }

    if ((scene->overlay_state.call_counter & kOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;

    fill_layer_quad(
        quad,
        kOverlayHalfExtent,
        kOverlayHalfExtent,
        kOverlayDepth,
        overlay_intensity + kOverlayRedBias,
        overlay_intensity,
        overlay_intensity,
        overlay_intensity,
        scene->overlay_state.jitter_x,
        scene->overlay_state.jitter_x + 1.0f,
        scene->overlay_state.jitter_y,
        scene->overlay_state.jitter_y + 1.0f
    );
}

void cornball_s_pair_scene_clear(CornballSPairScene *scene)
{
    memset(scene, 0, sizeof(*scene));
}

void cornball_s_pair_scene_loader_pass(CornballSPairScene *scene)
{
    if (scene->texture_group_loaded != 0u) {
        return;
    }

    scene->texture_group_loaded = 1u;
}

void cornball_s_pair_scene_step_frame(
    CornballSPairScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballSPairFrame *frame
)
{
    unsigned int rand15;
    float overlay_intensity;

    memset(frame, 0, sizeof(*frame));

    rand15 = cornball_lcg_rand15(random);
    frame->rand_unit = (float)rand15 * kRandScale;

    build_bipyramid_pass(frame->rand_unit, scene_elapsed_seconds, &frame->bipyramid);

    overlay_intensity = build_overlay_intensity(frame->rand_unit, scene_elapsed_seconds);
    build_overlay_pass(scene, random, overlay_intensity, &frame->overlay_quad);
}
