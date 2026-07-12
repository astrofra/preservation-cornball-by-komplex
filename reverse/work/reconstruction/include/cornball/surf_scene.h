#ifndef CORNBALL_SURF_SCENE_H
#define CORNBALL_SURF_SCENE_H

#include <stdint.h>

#include "cornball/random.h"

typedef struct CornballSurfLayerQuad {
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
} CornballSurfLayerQuad;

typedef struct CornballSurfOverlayState {
    float jitter_x;
    float jitter_y;
    uint32_t call_counter;
} CornballSurfOverlayState;

typedef struct CornballSurfFogState {
    uint32_t enabled;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    float density;
    float start_distance;
    float end_distance;
} CornballSurfFogState;

typedef struct CornballSurfTubeShellPass {
    uint32_t enabled;
    float translate_x;
    float translate_y;
    float translate_z;
    float rotate_z_pre_degrees;
    float rotate_x_degrees;
    float rotate_z_post_degrees;
    double phase;
} CornballSurfTubeShellPass;

typedef struct CornballSurfQuadStackPass {
    uint32_t enabled;
    uint32_t repeat_count;
    float base_translate_z;
    float step_translate_z;
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
} CornballSurfQuadStackPass;

typedef struct CornballSurfFrame {
    float burned_rand_unit;
    CornballSurfFogState fog;
    CornballSurfTubeShellPass tube_shell;
    CornballSurfQuadStackPass fla_quad_stack;
    CornballSurfLayerQuad surf_foreground_quad;
    float surf_foreground_rotation_degrees;
    CornballSurfLayerQuad jitter_overlay_quad;
} CornballSurfFrame;

typedef struct CornballSurfScene {
    CornballSurfOverlayState overlay_state;
    uint32_t texture_group_loaded;
} CornballSurfScene;

void cornball_surf_scene_clear(CornballSurfScene *scene);
void cornball_surf_scene_loader_pass(CornballSurfScene *scene);
void cornball_surf_scene_step_frame(
    CornballSurfScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballSurfFrame *frame
);

#endif
