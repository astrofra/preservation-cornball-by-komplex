#ifndef CORNBALL_S_PAIR_SCENE_H
#define CORNBALL_S_PAIR_SCENE_H

#include <stdint.h>

#include "cornball/random.h"

typedef struct CornballSPairLayerQuad {
    uint32_t enabled;
    float half_width;
    float half_height;
    float depth_z;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    float texcoord_min_u;
    float texcoord_max_u;
    float texcoord_min_v;
    float texcoord_max_v;
} CornballSPairLayerQuad;

typedef struct CornballSPairOverlayState {
    float jitter_x;
    float jitter_y;
    uint32_t call_counter;
} CornballSPairOverlayState;

typedef struct CornballSPairBipyramidPass {
    uint32_t enabled;
    float rotate_z_pre_degrees;
    float rotate_y_degrees;
    float rotate_x_degrees;
    float rotate_z_post_degrees;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
} CornballSPairBipyramidPass;

typedef struct CornballSPairFrame {
    float rand_unit;
    CornballSPairBipyramidPass bipyramid;
    CornballSPairLayerQuad overlay_quad;
} CornballSPairFrame;

typedef struct CornballSPairScene {
    CornballSPairOverlayState overlay_state;
    uint32_t texture_group_loaded;
} CornballSPairScene;

void cornball_s_pair_scene_clear(CornballSPairScene *scene);
void cornball_s_pair_scene_loader_pass(CornballSPairScene *scene);
void cornball_s_pair_scene_step_frame(
    CornballSPairScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballSPairFrame *frame
);

#endif
