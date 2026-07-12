#include "cornball/intro_scene.h"

#include <math.h>
#include <string.h>

static const float kBipyramidBaseRotateZ = 90.0f;
static const float kOverlayHalfExtent = 2.0f;
static const float kOverlayDepth = -2.0f;
static const float kOverlayTint = 0.85f;
static const float kSoftQuadDepth = -2.0f;
static const float kSoftQuadInsetTexcoord = 0.01f;
static const float kLogotausHalfExtent = 1.45f;
static const float kLogotausColor = 0.45f;
static const float kLogotausAlphaScale = 0.55f;
static const float kLogoHalfExtent = 1.05f;
static const float kLogoAlphaScale = 0.65f;
static const uint32_t kOverlayReseedMask = 1u;
static const double kBipyramidRotateYRate = 2.2;
static const double kBipyramidRotateXRate = 10.0;
static const double kTxt2OpeningPhaseSeconds = 5.0;
static const double kLogotausFadeInStartSeconds = 1.0;
static const double kLogotausFadeInEndSeconds = 1.5;
static const double kLogotausFadeOutStartSeconds = 4.0;
static const double kLogotausFadeOutEndSeconds = 5.0;
static const double kLogoFadeInStartSeconds = 2.0;
static const double kLogoFadeInEndSeconds = 2.5;
static const double kLogoFadeOutStartSeconds = 4.25;
static const double kLogoFadeOutEndSeconds = 5.0;

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
    float texcoord_min_u,
    float texcoord_max_u,
    float texcoord_min_v,
    float texcoord_max_v,
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
    quad->texcoord_min_u = texcoord_min_u;
    quad->texcoord_max_u = texcoord_max_u;
    quad->texcoord_min_v = texcoord_min_v;
    quad->texcoord_max_v = texcoord_max_v;
    quad->rotation_degrees = rotation_degrees;
}

static void disable_layer_quad(CornballIntroLayerQuad *quad)
{
    memset(quad, 0, sizeof(*quad));
}

static float linear_unit_ramp(double scene_elapsed_seconds, double start_seconds, double end_seconds)
{
    if (scene_elapsed_seconds <= start_seconds) {
        return 0.0f;
    }

    if (scene_elapsed_seconds >= end_seconds) {
        return 1.0f;
    }

    return (float)((scene_elapsed_seconds - start_seconds) / (end_seconds - start_seconds));
}

static float layer_envelope(
    double scene_elapsed_seconds,
    double fade_in_start_seconds,
    double fade_in_end_seconds,
    double fade_out_start_seconds,
    double fade_out_end_seconds
)
{
    float fade_in = linear_unit_ramp(scene_elapsed_seconds, fade_in_start_seconds, fade_in_end_seconds);
    float fade_out = 1.0f - linear_unit_ramp(scene_elapsed_seconds, fade_out_start_seconds, fade_out_end_seconds);

    return fminf(fade_in, fade_out);
}

static void build_soft_quad(
    CornballIntroTextureSlot texture_slot,
    float half_extent,
    float translate_x,
    float translate_y,
    float color_r,
    float color_g,
    float color_b,
    float alpha,
    CornballIntroLayerQuad *quad
)
{
    if (alpha <= 0.0f) {
        disable_layer_quad(quad);
        return;
    }

    fill_layer_quad(
        quad,
        texture_slot,
        half_extent,
        half_extent,
        kSoftQuadDepth,
        translate_x,
        translate_y,
        color_r,
        color_g,
        color_b,
        alpha,
        kSoftQuadInsetTexcoord,
        1.0f - kSoftQuadInsetTexcoord,
        kSoftQuadInsetTexcoord,
        1.0f - kSoftQuadInsetTexcoord,
        0.0f
    );
}

static void build_logotaus_soft_quad(double scene_elapsed_seconds, CornballIntroLayerQuad *quad)
{
    float intensity = layer_envelope(
        scene_elapsed_seconds,
        kLogotausFadeInStartSeconds,
        kLogotausFadeInEndSeconds,
        kLogotausFadeOutStartSeconds,
        kLogotausFadeOutEndSeconds
    );

    build_soft_quad(
        CORNBALL_INTRO_TEXTURE_LOGOTAUS,
        kLogotausHalfExtent,
        0.0f,
        0.0f,
        kLogotausColor,
        kLogotausColor,
        kLogotausColor,
        intensity * kLogotausAlphaScale,
        quad
    );
}

static void build_logo_soft_quad(double scene_elapsed_seconds, CornballIntroLayerQuad *quad)
{
    float intensity = layer_envelope(
        scene_elapsed_seconds,
        kLogoFadeInStartSeconds,
        kLogoFadeInEndSeconds,
        kLogoFadeOutStartSeconds,
        kLogoFadeOutEndSeconds
    );

    build_soft_quad(
        CORNBALL_INTRO_TEXTURE_LOGO,
        kLogoHalfExtent,
        0.0f,
        0.2f,
        1.0f,
        1.0f,
        1.0f,
        intensity * kLogoAlphaScale,
        quad
    );
}

static void build_bipyramid_pass(
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballIntroBipyramidPass *pass
)
{
    (void)cornball_rand15_unit(random);

    memset(pass, 0, sizeof(*pass));
    pass->enabled = 1u;
    pass->front_texture_slot = CORNBALL_INTRO_TEXTURE_V1;
    pass->back_texture_slot = CORNBALL_INTRO_TEXTURE_V2;
    pass->rotate_x_degrees = (float)(scene_elapsed_seconds * kBipyramidRotateXRate);
    pass->rotate_y_degrees = (float)(scene_elapsed_seconds * kBipyramidRotateYRate);
    pass->rotate_z_degrees = kBipyramidBaseRotateZ;
    pass->color_r = 1.0f;
    pass->color_g = 1.0f;
    pass->color_b = 1.0f;
    pass->color_a = 1.0f;
}

static void build_overlay_pass(
    CornballIntroScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballIntroLayerQuad *quad
)
{
    CornballIntroTextureSlot texture_slot;

    if ((scene->overlay_state.call_counter & kOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;

    texture_slot = (scene_elapsed_seconds < kTxt2OpeningPhaseSeconds)
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
        kOverlayTint,
        kOverlayTint,
        kOverlayTint,
        1.0f,
        scene->overlay_state.jitter_x - 1.0f,
        scene->overlay_state.jitter_x,
        scene->overlay_state.jitter_y - 1.0f,
        scene->overlay_state.jitter_y,
        0.0f
    );
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
    memset(frame, 0, sizeof(*frame));

    build_bipyramid_pass(random, scene_elapsed_seconds, &frame->bipyramid);
    build_logotaus_soft_quad(scene_elapsed_seconds, &frame->logotaus_soft_quad);
    build_logo_soft_quad(scene_elapsed_seconds, &frame->logo_soft_quad);
    build_overlay_pass(scene, random, scene_elapsed_seconds, &frame->overlay_quad);
}
