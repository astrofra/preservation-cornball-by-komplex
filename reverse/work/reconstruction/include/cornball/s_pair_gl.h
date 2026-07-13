#ifndef CORNBALL_S_PAIR_GL_H
#define CORNBALL_S_PAIR_GL_H

#include <stddef.h>

#include "cornball/s_pair_scene.h"

typedef struct CornballSPairGlRenderer {
    unsigned int s1_texture;
    unsigned int s2_texture;
    unsigned int txt2_texture;
    int viewport_width;
    int viewport_height;
} CornballSPairGlRenderer;

int cornball_s_pair_gl_init(
    CornballSPairGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
);

void cornball_s_pair_gl_shutdown(CornballSPairGlRenderer *renderer);
void cornball_s_pair_gl_resize(CornballSPairGlRenderer *renderer, int width, int height);
void cornball_s_pair_gl_render(const CornballSPairGlRenderer *renderer, const CornballSPairFrame *frame);

#endif
