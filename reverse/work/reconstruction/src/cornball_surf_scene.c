#include "cornball/surf_scene.h"

#include <math.h>
#include <string.h>

static const float kFogDensity = 0.2f;
static const float kFogStartDistance = 1.0f;
static const float kFogEndDistance = 65.0f;
static const float kFogColorA = 1.0f;
static const float kTrailQuadHalfExtent = 2.0f;
static const float kForegroundQuadHalfExtent = 1.0f;
static const float kForegroundQuadDepth = -1.0f;
static const float kOverlayQuadHalfExtent = 2.0f;
static const float kOverlayQuadDepth = -2.0f;
static const float kForegroundQuadAlpha = 0.8f;
static const float kOverlayColorR = 0.4f;
static const float kOverlayColorG = 0.4f;
static const float kOverlayColorB = 0.2f;
static const float kOverlayColorA = 1.0f;
static const uint32_t kOverlayReseedMask = 0u;
static const uint32_t kTrailQuadCount = 32u;
static const double kTubeTranslateXRate = 0.2;
static const double kTubeTranslateYRate = 0.3;
static const double kTubeTranslateAmp = 3.0;
static const double kTubeRotateZPreRate = 3.0;
static const double kTubeRotateXRate = 0.5;
static const double kTubeRotateXAmp = 30.0;
static const double kTubeRotateZPostRate = 32.0;
static const double kTubePhaseRate = -0.3;
static const double kTrailBaseTranslateZRate = 8.0;
static const double kTrailStepTranslateZ = -10.0;
static const double kForegroundRotationRate = 11.0;

static void fill_layer_quad(
    CornballSurfLayerQuad *quad,
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

static void build_overlay_quad(
    CornballSurfScene *scene,
    CornballRandom *random,
    CornballSurfFrame *frame
)
{
    if ((scene->overlay_state.call_counter & kOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;

    fill_layer_quad(
        &frame->jitter_overlay_quad,
        kOverlayQuadHalfExtent,
        kOverlayQuadHalfExtent,
        kOverlayQuadDepth,
        kOverlayColorR,
        kOverlayColorG,
        kOverlayColorB,
        kOverlayColorA,
        scene->overlay_state.jitter_x,
        scene->overlay_state.jitter_x + 1.0f,
        scene->overlay_state.jitter_y,
        scene->overlay_state.jitter_y + 1.0f
    );
}

static void build_fog_state(CornballSurfFogState *fog)
{
    fog->enabled = 1u;
    fog->color_r = 0.0f;
    fog->color_g = 0.0f;
    fog->color_b = 0.0f;
    fog->color_a = kFogColorA;
    fog->density = kFogDensity;
    fog->start_distance = kFogStartDistance;
    fog->end_distance = kFogEndDistance;
}

static void build_tube_shell(double scene_elapsed_seconds, CornballSurfTubeShellPass *tube_shell)
{
    tube_shell->enabled = 1u;
    tube_shell->translate_x = (float)(cos(scene_elapsed_seconds * kTubeTranslateYRate) * kTubeTranslateAmp);
    tube_shell->translate_y = (float)(cos(scene_elapsed_seconds * kTubeTranslateXRate) * kTubeTranslateAmp);
    tube_shell->translate_z = 0.0f;
    tube_shell->rotate_z_pre_degrees = (float)(scene_elapsed_seconds * kTubeRotateZPreRate);
    tube_shell->rotate_x_degrees = (float)(sin(scene_elapsed_seconds * kTubeRotateXRate) * kTubeRotateXAmp);
    tube_shell->rotate_z_post_degrees = (float)(scene_elapsed_seconds * kTubeRotateZPostRate);
    tube_shell->phase = scene_elapsed_seconds * kTubePhaseRate;
}

static void build_fla_quad_stack(double scene_elapsed_seconds, CornballSurfQuadStackPass *quad_stack)
{
    quad_stack->enabled = 1u;
    quad_stack->repeat_count = kTrailQuadCount;
    quad_stack->base_translate_z = (float)(scene_elapsed_seconds * kTrailBaseTranslateZRate);
    quad_stack->step_translate_z = (float)kTrailStepTranslateZ;
    quad_stack->half_width = kTrailQuadHalfExtent;
    quad_stack->half_height = kTrailQuadHalfExtent;
    quad_stack->depth_z = 0.0f;
    quad_stack->color_r = 1.0f;
    quad_stack->color_g = 1.0f;
    quad_stack->color_b = 1.0f;
    quad_stack->color_a = 0.0f;
    quad_stack->texcoord_min_u = 0.0f;
    quad_stack->texcoord_max_u = 1.0f;
    quad_stack->texcoord_min_v = 0.0f;
    quad_stack->texcoord_max_v = 1.0f;
}

static void build_foreground_quad(double scene_elapsed_seconds, CornballSurfFrame *frame)
{
    fill_layer_quad(
        &frame->surf_foreground_quad,
        kForegroundQuadHalfExtent,
        kForegroundQuadHalfExtent,
        kForegroundQuadDepth,
        1.0f,
        1.0f,
        1.0f,
        kForegroundQuadAlpha,
        0.0f,
        2.0f,
        0.0f,
        2.0f
    );

    frame->surf_foreground_rotation_degrees = (float)(scene_elapsed_seconds * kForegroundRotationRate);
}

void cornball_surf_scene_clear(CornballSurfScene *scene)
{
    memset(scene, 0, sizeof(*scene));
}

void cornball_surf_scene_loader_pass(CornballSurfScene *scene)
{
    if (scene->texture_group_loaded != 0u) {
        return;
    }

    scene->texture_group_loaded = 1u;
}

void cornball_surf_scene_step_frame(
    CornballSurfScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballSurfFrame *frame
)
{
    memset(frame, 0, sizeof(*frame));

    frame->burned_rand_unit = cornball_rand15_unit(random);
    build_fog_state(&frame->fog);
    build_tube_shell(scene_elapsed_seconds, &frame->tube_shell);
    build_fla_quad_stack(scene_elapsed_seconds, &frame->fla_quad_stack);
    build_foreground_quad(scene_elapsed_seconds, frame);
    build_overlay_quad(scene, random, frame);
}
