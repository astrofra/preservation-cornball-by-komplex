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
static const double kSceneRotationRate = 0.72;
static const double kSceneRotationAmplitude = 19.0;
static const double kSceneRotationBase = 180.0;

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
    size_t i;

    frame->rotation_degrees = cornball_fla_scene_rotation_degrees(scene_elapsed_seconds);
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
