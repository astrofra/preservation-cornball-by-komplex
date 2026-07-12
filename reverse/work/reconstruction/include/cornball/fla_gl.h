#ifndef CORNBALL_FLA_GL_H
#define CORNBALL_FLA_GL_H

#include <stddef.h>

#include "cornball/fla_scene.h"

typedef struct CornballFlaGlRenderer {
    unsigned int fla_texture;
    unsigned int logotaus_texture;
    unsigned int txt1_texture;
    int viewport_width;
    int viewport_height;
} CornballFlaGlRenderer;

int cornball_fla_gl_init(
    CornballFlaGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
);

void cornball_fla_gl_shutdown(CornballFlaGlRenderer *renderer);
void cornball_fla_gl_resize(CornballFlaGlRenderer *renderer, int width, int height);
void cornball_fla_gl_render(const CornballFlaGlRenderer *renderer, const CornballFlaFrame *frame);

#endif
