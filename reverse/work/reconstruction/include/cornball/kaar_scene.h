#ifndef CORNBALL_KAAR_SCENE_H
#define CORNBALL_KAAR_SCENE_H

#include <stdint.h>

#include "cornball/random.h"

typedef struct CornballKaarLayerQuad {
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
} CornballKaarLayerQuad;

typedef struct CornballKaarOverlayState {
    float jitter_x;
    float jitter_y;
    uint32_t call_counter;
} CornballKaarOverlayState;

typedef struct CornballKaarFogState {
    uint32_t enabled;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    float density;
    float start_distance;
    float end_distance;
} CornballKaarFogState;

typedef struct CornballKaarTubeShellPass {
    uint32_t enabled;
    float translate_x;
    float translate_y;
    float translate_z;
    float rotate_x_degrees;
    float rotate_y_degrees;
    float rotate_z_degrees;
    double phase;
} CornballKaarTubeShellPass;

typedef struct CornballKaarFrame {
    float gate_unit;
    float gate_tint;
    CornballKaarLayerQuad jitter_overlay_quad;
    CornballKaarFogState fog;
    CornballKaarTubeShellPass tube_shell;
    CornballKaarLayerQuad txt1_center_quad;
    float txt1_rotation_degrees;
} CornballKaarFrame;

typedef struct CornballKaarScene {
    CornballKaarOverlayState overlay_state;
    uint32_t texture_group_loaded;
} CornballKaarScene;

void cornball_kaar_scene_clear(CornballKaarScene *scene);
void cornball_kaar_scene_loader_pass(CornballKaarScene *scene);
void cornball_kaar_scene_step_frame(
    CornballKaarScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballKaarFrame *frame
);

#endif
