#include "cornball/fla_scene.h"

#include <math.h>
#include <string.h>

static const float kBrightnessThreshold = 0.01f;
static const float kBrightnessDecay = 0.9f;
static const float kVelocityXBias = 0.5f;
static const float kVelocityXScale = 0.7f;
static const float kVelocityYBase = 0.7f;
static const float kVelocityYScale = 0.3f;
static const float kAccelYBias = -0.01f;
static const float kAccelYScale = -0.15f;
static const float kDrawQuadHalfExtent = 1.0f;
static const float kDrawBrightnessScale = 3.0f;
static const float kHelperQuadHalfExtent = 2.0f;
static const float kHelperQuadDepth = -2.0f;
static const float kSoftQuadBlue = 0.5f;
static const float kOverlayBlueScale = 0.8f;
static const float kInsetTexcoord = 0.01f;
static const double kSceneRotationRate = 0.72;
static const double kSceneRotationAmplitude = -19.0;
static const double kSceneRotationBase = 180.0;
static const uint32_t kFlaOverlayReseedMask = 1u;

static void cornball_fla_fill_layer_quad(
    CornballFlaLayerQuad *quad,
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

static void cornball_fla_disable_layer_quad(CornballFlaLayerQuad *quad)
{
    memset(quad, 0, sizeof(*quad));
}

static void cornball_fla_scene_build_particle_quads(
    const CornballFlaScene *scene,
    CornballFlaFrame *frame
)
{
    size_t i;

    frame->quad_count = CORNBALL_FLA_PARTICLE_COUNT;

    for (i = 0; i < CORNBALL_FLA_PARTICLE_COUNT; ++i) {
        const CornballFlaParticleState *state = &scene->state_block[i];
        CornballFlaDrawQuad *quad = &frame->quads[i];

        quad->translate_x = state->pos_x;
        quad->translate_y = state->pos_y;
        quad->translate_z = 0.0f;
        quad->half_width = kDrawQuadHalfExtent;
        quad->half_height = kDrawQuadHalfExtent;
        quad->grayscale = state->brightness * kDrawBrightnessScale;
    }
}

static void cornball_fla_scene_build_helper_layers(
    const CornballFlaScene *scene,
    CornballFlaFrame *frame
)
{
    cornball_fla_fill_layer_quad(
        &frame->logotaus_quad,
        kHelperQuadHalfExtent,
        kHelperQuadHalfExtent,
        kHelperQuadDepth,
        1.0f,
        1.0f,
        kSoftQuadBlue,
        1.0f,
        kInsetTexcoord,
        1.0f - kInsetTexcoord,
        kInsetTexcoord,
        1.0f - kInsetTexcoord
    );

    if (scene->overlay_state.call_counter == 0u) {
        cornball_fla_disable_layer_quad(&frame->txt1_overlay_quad);
        return;
    }

    cornball_fla_fill_layer_quad(
        &frame->txt1_overlay_quad,
        kHelperQuadHalfExtent,
        kHelperQuadHalfExtent,
        kHelperQuadDepth,
        scene->overlay_state.tint,
        scene->overlay_state.tint,
        scene->overlay_state.tint * kOverlayBlueScale,
        1.0f,
        scene->overlay_state.jitter_x - 1.0f,
        scene->overlay_state.jitter_x,
        scene->overlay_state.jitter_y - 1.0f,
        scene->overlay_state.jitter_y
    );
}

static void cornball_fla_scene_respawn_particle(
    CornballFlaScene *scene,
    size_t index,
    CornballRandom *random
)
{
    CornballFlaParticleState *state = &scene->state_block[index];
    CornballFlaParticleVelocity *velocity = &scene->velocity_block[index];
    CornballFlaParticleAccel *accel = &scene->accel_block[index];

    state->brightness = cornball_rand15_unit(random);
    state->pos_x = 0.0f;
    state->pos_y = 0.0f;
    state->pos_z = 0.0f;

    velocity->unused0 = 0.0f;
    velocity->vel_x = (cornball_rand15_unit(random) - kVelocityXBias) * kVelocityXScale;
    velocity->vel_y = kVelocityYBase + cornball_rand15_unit(random) * kVelocityYScale;
    velocity->vel_z = 0.0f;

    accel->unused0 = 0.0f;
    accel->accel_x = 0.0f;
    accel->accel_y = cornball_rand15_unit(random) * kAccelYScale + kAccelYBias;
    accel->accel_z = 0.0f;
}

void cornball_fla_scene_clear(CornballFlaScene *scene)
{
    memset(scene, 0, sizeof(*scene));
}

void cornball_fla_scene_loader_pass(CornballFlaScene *scene)
{
    if (scene->texture_group_loaded != 0u) {
        return;
    }

    memset(scene->velocity_block, 0, sizeof(scene->velocity_block));
    memset(scene->accel_block, 0, sizeof(scene->accel_block));
    memset(scene->state_block, 0, sizeof(scene->state_block));

    scene->texture_group_loaded = 1u;
}

void cornball_fla_scene_update(CornballFlaScene *scene, CornballRandom *random)
{
    size_t i;

    for (i = 0; i < CORNBALL_FLA_PARTICLE_COUNT; ++i) {
        CornballFlaParticleState *state = &scene->state_block[i];
        CornballFlaParticleVelocity *velocity = &scene->velocity_block[i];
        CornballFlaParticleAccel *accel = &scene->accel_block[i];

        if (state->brightness > kBrightnessThreshold) {
            state->brightness *= kBrightnessDecay;

            state->pos_x += velocity->vel_x;
            state->pos_y += velocity->vel_y;
            state->pos_z += velocity->vel_z;

            velocity->vel_x += accel->accel_x;
            velocity->vel_y += accel->accel_y;
            velocity->vel_z += accel->accel_z;
        } else {
            cornball_fla_scene_respawn_particle(scene, i, random);
        }
    }
}

void cornball_fla_scene_step_frame(
    CornballFlaScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballFlaFrame *frame
)
{
    scene->overlay_state.tint = cornball_rand15_unit(random);
    cornball_fla_scene_update(scene, random);

    if ((scene->overlay_state.call_counter & kFlaOverlayReseedMask) == 0u) {
        scene->overlay_state.jitter_x = cornball_rand15_unit(random);
        scene->overlay_state.jitter_y = cornball_rand15_unit(random);
    }

    scene->overlay_state.call_counter += 1u;
    cornball_fla_scene_build_frame(scene, scene_elapsed_seconds, frame);
}

float cornball_fla_scene_rotation_degrees(double scene_elapsed_seconds)
{
    return (float)(
        kSceneRotationBase +
        sin(scene_elapsed_seconds * kSceneRotationRate) * kSceneRotationAmplitude
    );
}

void cornball_fla_scene_build_frame(
    const CornballFlaScene *scene,
    double scene_elapsed_seconds,
    CornballFlaFrame *frame
)
{
    frame->rotation_degrees = cornball_fla_scene_rotation_degrees(scene_elapsed_seconds);
    cornball_fla_scene_build_helper_layers(scene, frame);
    cornball_fla_scene_build_particle_quads(scene, frame);
}
