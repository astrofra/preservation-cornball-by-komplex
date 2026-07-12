#ifndef CORNBALL_FLA_SCENE_H
#define CORNBALL_FLA_SCENE_H

#include <stddef.h>
#include <stdint.h>

#include "cornball/random.h"

enum {
    CORNBALL_FLA_PARTICLE_COUNT = 500
};

typedef struct CornballFlaParticleState {
    float brightness;
    float pos_x;
    float pos_y;
    float pos_z;
} CornballFlaParticleState;

typedef struct CornballFlaParticleVelocity {
    float unused0;
    float vel_x;
    float vel_y;
    float vel_z;
} CornballFlaParticleVelocity;

typedef struct CornballFlaParticleAccel {
    float unused0;
    float accel_x;
    float accel_y;
    float accel_z;
} CornballFlaParticleAccel;

typedef struct CornballFlaDrawQuad {
    float translate_x;
    float translate_y;
    float translate_z;
    float half_width;
    float half_height;
    float grayscale;
} CornballFlaDrawQuad;

typedef struct CornballFlaFrame {
    float rotation_degrees;
    size_t quad_count;
    CornballFlaDrawQuad quads[CORNBALL_FLA_PARTICLE_COUNT];
} CornballFlaFrame;

typedef struct CornballFlaScene {
    CornballFlaParticleVelocity velocity_block[CORNBALL_FLA_PARTICLE_COUNT];
    CornballFlaParticleAccel accel_block[CORNBALL_FLA_PARTICLE_COUNT];
    CornballFlaParticleState state_block[CORNBALL_FLA_PARTICLE_COUNT];
    uint32_t texture_group_loaded;
} CornballFlaScene;

void cornball_fla_scene_clear(CornballFlaScene *scene);
void cornball_fla_scene_loader_pass(CornballFlaScene *scene);
void cornball_fla_scene_update(CornballFlaScene *scene, CornballRandom *random);
float cornball_fla_scene_rotation_degrees(double scene_elapsed_seconds);
void cornball_fla_scene_build_frame(
    const CornballFlaScene *scene,
    double scene_elapsed_seconds,
    CornballFlaFrame *frame
);

#endif
