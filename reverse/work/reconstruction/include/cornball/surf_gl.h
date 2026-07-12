#ifndef CORNBALL_SURF_GL_H
#define CORNBALL_SURF_GL_H

#include <stddef.h>
#include <stdint.h>

#include "cornball/surf_scene.h"

enum {
    CORNBALL_SURF_TUBE_SEGMENT_COUNT = 64
};

typedef struct CornballSurfGlRenderer {
    unsigned int surf128_texture;
    unsigned int fla_texture;
    unsigned int txt1_texture;
    int viewport_width;
    int viewport_height;
    uint32_t tube_cache_valid;
    float tube_ring_vertices_neg_z[CORNBALL_SURF_TUBE_SEGMENT_COUNT][3];
    float tube_ring_vertices_pos_z[CORNBALL_SURF_TUBE_SEGMENT_COUNT][3];
} CornballSurfGlRenderer;

int cornball_surf_gl_init(
    CornballSurfGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
);

void cornball_surf_gl_shutdown(CornballSurfGlRenderer *renderer);
void cornball_surf_gl_resize(CornballSurfGlRenderer *renderer, int width, int height);
void cornball_surf_gl_render(const CornballSurfGlRenderer *renderer, const CornballSurfFrame *frame);

#endif
