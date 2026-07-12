#include "cornball/kaar_scene.h"

#include <math.h>
#include <string.h>

static const float kKaarRandScale = 3.05185094759972e-05f;
static const float kKaarGateTintScale = 0.1f;
static const float kKaarPreludeThreshold = 0.25f;
static const float kHelperQuadHalfExtent = 2.0f;
static const float kHelperQuadDepth = -2.0f;
static const float kCenteredQuadHalfExtent = 3.0f;
static const float kCenteredQuadDepth = -2.0f;
static const float kInsetTexcoord = 0.01f;
static const float kCenteredQuadColorR = 0.5f;
static const float kCenteredQuadColorB = 0.2f;
static const float kCenteredQuadColorA = 1.0f;
static const float kCenteredQuadGreenBias = 0.3f;
static const float kFogColorG = 1.0f;
static const float kFogColorB = 1.0f;
static const float kFogColorA = 1.0f;
static const float kFogDensity = 0.2f;
static const float kFogStartDistance = 1.0f;
static const float kFogEndDistance = 65.0f;
static const double kOrbitXRate = 0.2;
static const double kOrbitXAmp = 190.0;
static const double kOrbitYRate = 0.3;
static const double kOrbitYAmp = 3.0;
static const double kRotateXRate = 0.3;
static const double kRotateXAmp = 190.0;
static const double kRotateYRate = 2.0;
static const double kRotateZRate = 1.0;
static const double kTubePhaseRate = 0.1;
static const double kTxt1RotationRate = 11.0;
static const uint32_t kOverlayReseedMask = 1u;

static void fill_layer_quad(
    CornballKaarLayerQuad *quad,
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

static void disable_layer_quad(CornballKaarLayerQuad *quad)
{
    memset(quad, 0, sizeof(*quad));
}

static void disable_fog(CornballKaarFogState *fog)
{
    memset(fog, 0, sizeof(*fog));
}

static void disable_tube_shell(CornballKaarTubeShellPass *tube_shell)
{
    memset(tube_shell, 0, sizeof(*tube_shell));
}

static void build_center_quad(
    CornballKaarFrame *frame,
    float gate_tint,
    double scene_elapsed_seconds
)
{
    fill_layer_quad(
        &frame->txt1_center_quad,
        kCenteredQuadHalfExtent,
        kCenteredQuadHalfExtent,
        kCenteredQuadDepth,
        kCenteredQuadColorR,
        gate_tint + kCenteredQuadGreenBias,
        kCenteredQuadColorB,
        kCenteredQuadColorA,
        kInsetTexcoord,
        1.0f - kInsetTexcoord,
        kInsetTexcoord,
        1.0f - kInsetTexcoord
    );

    frame->txt1_rotation_degrees = (float)(scene_elapsed_seconds * kTxt1RotationRate);
}

static void reseed_overlay_jitter(CornballKaarScene *scene, CornballRandom *random)
{
    if ((scene->overlay_state.call_counter & kOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;
}

static void build_jitter_overlay(
    CornballKaarScene *scene,
    CornballRandom *random,
    float gate_unit,
    CornballKaarFrame *frame
)
{
    reseed_overlay_jitter(scene, random);

    fill_layer_quad(
        &frame->jitter_overlay_quad,
        kHelperQuadHalfExtent,
        kHelperQuadHalfExtent,
        kHelperQuadDepth,
        gate_unit,
        0.0f,
        0.0f,
        0.0f,
        scene->overlay_state.jitter_x - 1.0f,
        scene->overlay_state.jitter_x,
        scene->overlay_state.jitter_y - 1.0f,
        scene->overlay_state.jitter_y
    );
}

static void build_main_tube_shell(double scene_elapsed_seconds, CornballKaarFrame *frame)
{
    frame->fog.enabled = 1u;
    frame->fog.color_r = 0.0f;
    frame->fog.color_g = kFogColorG;
    frame->fog.color_b = kFogColorB;
    frame->fog.color_a = kFogColorA;
    frame->fog.density = kFogDensity;
    frame->fog.start_distance = kFogStartDistance;
    frame->fog.end_distance = kFogEndDistance;

    frame->tube_shell.enabled = 1u;
    frame->tube_shell.translate_x = (float)(cos(scene_elapsed_seconds * kOrbitXRate) * kOrbitXAmp);
    frame->tube_shell.translate_y = (float)(cos(scene_elapsed_seconds * kOrbitYRate) * kOrbitYAmp);
    frame->tube_shell.translate_z = 0.0f;
    frame->tube_shell.rotate_x_degrees = (float)(sin(scene_elapsed_seconds * kRotateXRate) * kRotateXAmp);
    frame->tube_shell.rotate_y_degrees = (float)(scene_elapsed_seconds * kRotateYRate);
    frame->tube_shell.rotate_z_degrees = (float)(scene_elapsed_seconds * kRotateZRate);
    frame->tube_shell.phase = scene_elapsed_seconds * kTubePhaseRate;
}

void cornball_kaar_scene_clear(CornballKaarScene *scene)
{
    memset(scene, 0, sizeof(*scene));
}

void cornball_kaar_scene_loader_pass(CornballKaarScene *scene)
{
    if (scene->texture_group_loaded != 0u) {
        return;
    }

    scene->texture_group_loaded = 1u;
}

void cornball_kaar_scene_step_frame(
    CornballKaarScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballKaarFrame *frame
)
{
    unsigned gate_rand15;

    memset(frame, 0, sizeof(*frame));

    gate_rand15 = cornball_lcg_rand15(random);
    frame->gate_unit = (float)gate_rand15 * kKaarRandScale;
    frame->gate_tint = frame->gate_unit * kKaarGateTintScale;

    disable_layer_quad(&frame->jitter_overlay_quad);
    disable_fog(&frame->fog);
    disable_tube_shell(&frame->tube_shell);

    if (frame->gate_unit > kKaarPreludeThreshold) {
        build_jitter_overlay(scene, random, frame->gate_unit, frame);
    } else {
        build_main_tube_shell(scene_elapsed_seconds, frame);
    }

    build_center_quad(frame, frame->gate_tint, scene_elapsed_seconds);
}
