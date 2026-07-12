#include "cornball/intro_scene.h"

#include <math.h>
#include <string.h>

static const float kBipyramidBaseRotateZ = 90.0f;
static const float kOverlayHalfExtent = 2.0f;
static const float kOverlayDepth = -2.0f;
static const float kSoftQuadDepth = -2.0f;
static const float kSoftQuadInsetTexcoord = 0.01f;
static const float kSoftQuadMaxUTexcoord = 0.99f;
static const float kSoftQuadTopLeftVTexcoord = 0.99f;
static const float kSoftQuadTopRightVTexcoord = 1.0f;
static const uint32_t kOverlayReseedMask = 1u;
static const double kBipyramidRotateYRate = 2.2;
static const double kBipyramidRotateXRate = 10.0;
static const double kBipyramidColorRandScale = 0.21;
static const double kBipyramidColorRedBase = 0.3;
static const double kBipyramidColorGreenBase = 0.31;
static const double kBipyramidColorBlueBase = 0.1;
static const double kTxt2LastInclusiveSeconds = 5.0;
static const double kLogotausHalfWidthBase = 3.2;
static const double kLogotausHalfWidthRate = 0.02;
static const double kLogotausHalfHeightScale = 0.8;
static const double kLogotausMinHalfWidth = 1.0;
static const double kLogotausRotateRate = 9.0;
static const double kLogoIntensityBias = 3.0;
static const double kLogoIntensityDenominatorRate = 0.5;
static const double kLogoRotateBase = 180.0;
static const double kLogoRotateRate = 3.0;
static const double kLogoPhaseBase = 0.3;
static const double kLogoPhaseRate = 0.06;
static const double kLogoPhaseWrap = 2.0;
static const double kLogoHalfWidthScale = 2.0;
static const double kLogoHalfHeightScale = 0.5;
static const double kOverlayIntensityBias = 1.0;
static const double kOverlayIntensityDenominatorRate = 0.8;
static const double kLayerVisibilityThreshold = 0.01;

static void set_texcoord(CornballIntroLayerQuad *quad, size_t index, float u, float v)
{
    quad->texcoords[index].u = u;
    quad->texcoords[index].v = v;
}

static void fill_layer_quad(
    CornballIntroLayerQuad *quad,
    CornballIntroTextureSlot texture_slot,
    float half_width,
    float half_height,
    float depth_z,
    float translate_x,
    float translate_y,
    float color_r,
    float color_g,
    float color_b,
    float color_a,
    float rotation_degrees
)
{
    quad->enabled = 1u;
    quad->texture_slot = texture_slot;
    quad->half_width = half_width;
    quad->half_height = half_height;
    quad->depth_z = depth_z;
    quad->translate_x = translate_x;
    quad->translate_y = translate_y;
    quad->color_r = color_r;
    quad->color_g = color_g;
    quad->color_b = color_b;
    quad->color_a = color_a;
    quad->rotation_degrees = rotation_degrees;
}

static void disable_layer_quad(CornballIntroLayerQuad *quad)
{
    memset(quad, 0, sizeof(*quad));
}

static void set_soft_blended_quad_texcoords(CornballIntroLayerQuad *quad)
{
    set_texcoord(quad, 0u, kSoftQuadMaxUTexcoord, kSoftQuadInsetTexcoord);
    set_texcoord(quad, 1u, kSoftQuadMaxUTexcoord, kSoftQuadTopRightVTexcoord);
    set_texcoord(quad, 2u, kSoftQuadInsetTexcoord, kSoftQuadTopLeftVTexcoord);
    set_texcoord(quad, 3u, kSoftQuadInsetTexcoord, kSoftQuadInsetTexcoord);
}

static void set_overlay_quad_texcoords(CornballIntroLayerQuad *quad, float jitter_x, float jitter_y)
{
    set_texcoord(quad, 0u, jitter_x + 1.0f, jitter_y);
    set_texcoord(quad, 1u, jitter_x + 1.0f, jitter_y + 1.0f);
    set_texcoord(quad, 2u, jitter_x, jitter_y + 1.0f);
    set_texcoord(quad, 3u, jitter_x, jitter_y);
}

static float compute_logo_intensity(double scene_elapsed_seconds, float rand_unit)
{
    return (float)(((double)rand_unit + kLogoIntensityBias) / (1.0 + scene_elapsed_seconds * kLogoIntensityDenominatorRate));
}

static float compute_overlay_intensity(double scene_elapsed_seconds, float rand_unit)
{
    return (float)(((double)rand_unit + kOverlayIntensityBias) / (1.0 + scene_elapsed_seconds * kOverlayIntensityDenominatorRate));
}

static void build_logotaus_soft_quad(double scene_elapsed_seconds, CornballIntroLayerQuad *quad)
{
    float half_width = (float)(kLogotausHalfWidthBase - scene_elapsed_seconds * kLogotausHalfWidthRate);

    if (half_width < (float)kLogotausMinHalfWidth) {
        disable_layer_quad(quad);
        return;
    }

    fill_layer_quad(
        quad,
        CORNBALL_INTRO_TEXTURE_LOGOTAUS,
        half_width,
        (float)(half_width * kLogotausHalfHeightScale),
        kSoftQuadDepth,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        (float)(scene_elapsed_seconds * kLogotausRotateRate)
    );
    set_soft_blended_quad_texcoords(quad);
}

static void build_bipyramid_pass(
    float rand_unit,
    double scene_elapsed_seconds,
    CornballIntroBipyramidPass *pass
)
{
    memset(pass, 0, sizeof(*pass));
    pass->enabled = 1u;
    pass->front_texture_slot = CORNBALL_INTRO_TEXTURE_V1;
    pass->back_texture_slot = CORNBALL_INTRO_TEXTURE_V2;
    pass->rotate_x_degrees = (float)(scene_elapsed_seconds * kBipyramidRotateXRate);
    pass->rotate_y_degrees = (float)(scene_elapsed_seconds * kBipyramidRotateYRate);
    pass->rotate_z_degrees = kBipyramidBaseRotateZ;
    pass->color_r = (float)(kBipyramidColorRedBase + (double)rand_unit * kBipyramidColorRandScale);
    pass->color_g = (float)(kBipyramidColorGreenBase - (double)rand_unit * kBipyramidColorRandScale);
    pass->color_b = (float)(kBipyramidColorBlueBase + (double)rand_unit * kBipyramidColorRandScale);
    pass->color_a = 1.0f;
}

static void build_logo_soft_quad(double scene_elapsed_seconds, float rand_unit, CornballIntroLayerQuad *quad)
{
    double phase;
    float intensity = compute_logo_intensity(scene_elapsed_seconds, rand_unit);

    if (intensity <= (float)kLayerVisibilityThreshold) {
        disable_layer_quad(quad);
        return;
    }

    phase = kLogoPhaseBase + scene_elapsed_seconds * kLogoPhaseRate;
    if (phase <= 0.0) {
        phase = 0.0;
    }
    phase = fmod(phase, kLogoPhaseWrap);

    fill_layer_quad(
        quad,
        CORNBALL_INTRO_TEXTURE_LOGO,
        (float)(phase * kLogoHalfWidthScale),
        (float)(phase * kLogoHalfHeightScale),
        kSoftQuadDepth,
        0.0f,
        0.0f,
        intensity,
        intensity,
        intensity,
        0.0f,
        (float)(kLogoRotateBase + scene_elapsed_seconds * kLogoRotateRate)
    );
    set_soft_blended_quad_texcoords(quad);
}

static void build_overlay_pass(
    CornballIntroScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    float rand_unit,
    CornballIntroLayerQuad *quad
)
{
    float intensity = compute_overlay_intensity(scene_elapsed_seconds, rand_unit);
    CornballIntroTextureSlot texture_slot;

    if (intensity <= (float)kLayerVisibilityThreshold) {
        disable_layer_quad(quad);
        return;
    }

    if ((scene->overlay_state.call_counter & kOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;

    texture_slot = (scene_elapsed_seconds <= kTxt2LastInclusiveSeconds)
        ? CORNBALL_INTRO_TEXTURE_TXT2
        : CORNBALL_INTRO_TEXTURE_TXT1;

    fill_layer_quad(
        quad,
        texture_slot,
        kOverlayHalfExtent,
        kOverlayHalfExtent,
        kOverlayDepth,
        0.0f,
        0.0f,
        intensity,
        intensity,
        intensity,
        intensity,
        0.0f
    );
    set_overlay_quad_texcoords(quad, scene->overlay_state.jitter_x, scene->overlay_state.jitter_y);
}

void cornball_intro_scene_clear(CornballIntroScene *scene)
{
    memset(scene, 0, sizeof(*scene));
}

void cornball_intro_scene_loader_pass(CornballIntroScene *scene)
{
    if (scene->texture_group_loaded != 0u) {
        return;
    }

    scene->texture_group_loaded = 1u;
}

void cornball_intro_scene_step_frame(
    CornballIntroScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballIntroFrame *frame
)
{
    float rand_unit;

    memset(frame, 0, sizeof(*frame));

    rand_unit = cornball_rand15_unit(random);

    build_bipyramid_pass(rand_unit, scene_elapsed_seconds, &frame->bipyramid);
    build_logotaus_soft_quad(scene_elapsed_seconds, &frame->logotaus_soft_quad);
    build_logo_soft_quad(scene_elapsed_seconds, rand_unit, &frame->logo_soft_quad);
    build_overlay_pass(scene, random, scene_elapsed_seconds, rand_unit, &frame->overlay_quad);
}
