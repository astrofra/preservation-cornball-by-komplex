#ifndef CORNBALL_KAAR_GL_H
#define CORNBALL_KAAR_GL_H

#include <stddef.h>
#include <stdint.h>

#include "cornball/kaar_scene.h"

enum {
    CORNBALL_KAAR_TUBE_SEGMENT_COUNT = 64
};

typedef struct CornballKaarGlRenderer {
    unsigned int kaar128_texture;
    unsigned int txt1_texture;
    unsigned int txt2_texture;
    int viewport_width;
    int viewport_height;
    uint32_t tube_cache_valid;
    float tube_ring_vertices_neg_z[CORNBALL_KAAR_TUBE_SEGMENT_COUNT][3];
    float tube_ring_vertices_pos_z[CORNBALL_KAAR_TUBE_SEGMENT_COUNT][3];
} CornballKaarGlRenderer;

int cornball_kaar_gl_init(
    CornballKaarGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
);

void cornball_kaar_gl_shutdown(CornballKaarGlRenderer *renderer);
void cornball_kaar_gl_resize(CornballKaarGlRenderer *renderer, int width, int height);
void cornball_kaar_gl_render(const CornballKaarGlRenderer *renderer, const CornballKaarFrame *frame);

#endif
