#ifndef CORNBALL_INTRO_GL_H
#define CORNBALL_INTRO_GL_H

#include <stddef.h>

#include "cornball/intro_scene.h"

typedef struct CornballIntroGlRenderer {
    unsigned int v1_texture;
    unsigned int v2_texture;
    unsigned int txt1_texture;
    unsigned int txt2_texture;
    unsigned int logo_texture;
    unsigned int logotaus_texture;
    int viewport_width;
    int viewport_height;
} CornballIntroGlRenderer;

int cornball_intro_gl_init(
    CornballIntroGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
);

void cornball_intro_gl_shutdown(CornballIntroGlRenderer *renderer);
void cornball_intro_gl_resize(CornballIntroGlRenderer *renderer, int width, int height);
void cornball_intro_gl_render(const CornballIntroGlRenderer *renderer, const CornballIntroFrame *frame);

#endif
