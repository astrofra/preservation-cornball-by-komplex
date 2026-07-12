#ifndef CORNBALL_INTRO_SCENE_H
#define CORNBALL_INTRO_SCENE_H

#include <stdint.h>

#include "cornball/random.h"

typedef enum CornballIntroTextureSlot {
    CORNBALL_INTRO_TEXTURE_NONE = 0,
    CORNBALL_INTRO_TEXTURE_V1 = 1,
    CORNBALL_INTRO_TEXTURE_V2 = 2,
    CORNBALL_INTRO_TEXTURE_TXT1 = 3,
    CORNBALL_INTRO_TEXTURE_TXT2 = 4,
    CORNBALL_INTRO_TEXTURE_LOGO = 5,
    CORNBALL_INTRO_TEXTURE_LOGOTAUS = 6
} CornballIntroTextureSlot;

typedef struct CornballIntroTexcoord {
    float u;
    float v;
} CornballIntroTexcoord;

typedef struct CornballIntroLayerQuad {
    uint32_t enabled;
    CornballIntroTextureSlot texture_slot;
    float half_width;
    float half_height;
    float depth_z;
    float translate_x;
    float translate_y;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    CornballIntroTexcoord texcoords[4];
    float rotation_degrees;
} CornballIntroLayerQuad;

typedef struct CornballIntroOverlayState {
    float jitter_x;
    float jitter_y;
    uint32_t call_counter;
} CornballIntroOverlayState;

typedef struct CornballIntroBipyramidPass {
    uint32_t enabled;
    CornballIntroTextureSlot front_texture_slot;
    CornballIntroTextureSlot back_texture_slot;
    float rotate_x_degrees;
    float rotate_y_degrees;
    float rotate_z_degrees;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
} CornballIntroBipyramidPass;

typedef struct CornballIntroFrame {
    CornballIntroBipyramidPass bipyramid;
    CornballIntroLayerQuad logotaus_soft_quad;
    CornballIntroLayerQuad logo_soft_quad;
    CornballIntroLayerQuad overlay_quad;
} CornballIntroFrame;

typedef struct CornballIntroScene {
    CornballIntroOverlayState overlay_state;
    uint32_t texture_group_loaded;
} CornballIntroScene;

void cornball_intro_scene_clear(CornballIntroScene *scene);
void cornball_intro_scene_loader_pass(CornballIntroScene *scene);
void cornball_intro_scene_step_frame(
    CornballIntroScene *scene,
    CornballRandom *random,
    double scene_elapsed_seconds,
    CornballIntroFrame *frame
);

#endif
