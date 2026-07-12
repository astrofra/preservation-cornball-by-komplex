#include "cornball/kaar_gl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>

static const float kWorldHalfHeight = 192.0f;
static const float kWorldNear = -256.0f;
static const float kWorldFar = 256.0f;
static const float kCenteredTexcoordInset = 0.01f;
static const float kTubeRingZBias = 0.5f;
static const float kTubeRingZScale = 140.0f;
static const float kTubeAngleStep = 0.1f;
static const float kTubeRadius = 6.0f;
static const float kTubeTexcoordStep = 0.0625f;
static const float kTubePhaseOffset = 3.0f;

static void copy_error_message(char *dst, size_t dst_size, const char *src)
{
    if ((dst == NULL) || (dst_size == 0u)) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    snprintf(dst, dst_size, "%s", src);
}

static int load_file_tga_rgba(
    const char *path,
    unsigned char **pixels_out,
    int *width_out,
    int *height_out,
    char *error_message,
    size_t error_message_size
)
{
    FILE *file;
    unsigned char header[18];
    size_t pixel_size;
    size_t pixel_count;
    unsigned char *source_pixels;
    unsigned char *rgba_pixels;
    size_t i;
    unsigned char id_length;
    unsigned char color_map_type;
    unsigned char image_type;
    unsigned short width;
    unsigned short height;
    unsigned char pixel_depth;

    *pixels_out = NULL;
    *width_out = 0;
    *height_out = 0;

    file = fopen(path, "rb");
    if (file == NULL) {
        copy_error_message(error_message, error_message_size, "Failed to open TGA file.");
        return 0;
    }

    if (fread(header, sizeof(header), 1u, file) != 1u) {
        fclose(file);
        copy_error_message(error_message, error_message_size, "Failed to read TGA header.");
        return 0;
    }

    id_length = header[0];
    color_map_type = header[1];
    image_type = header[2];
    width = (unsigned short)(header[12] | (header[13] << 8));
    height = (unsigned short)(header[14] | (header[15] << 8));
    pixel_depth = header[16];

    if ((image_type != 2u) || (color_map_type != 0u)) {
        fclose(file);
        snprintf(
            error_message,
            error_message_size,
            "Unsupported TGA format for %s: image_type=%u color_map_type=%u.",
            path,
            (unsigned)image_type,
            (unsigned)color_map_type
        );
        return 0;
    }

    if ((pixel_depth != 24u) && (pixel_depth != 32u)) {
        fclose(file);
        snprintf(
            error_message,
            error_message_size,
            "Unsupported TGA depth for %s: pixel_depth=%u.",
            path,
            (unsigned)pixel_depth
        );
        return 0;
    }

    if (fseek(file, (long)id_length, SEEK_CUR) != 0) {
        fclose(file);
        copy_error_message(error_message, error_message_size, "Failed to seek past TGA image ID.");
        return 0;
    }

    *width_out = (int)width;
    *height_out = (int)height;

    pixel_size = (size_t)pixel_depth / 8u;
    pixel_count = (size_t)(*width_out) * (size_t)(*height_out);

    source_pixels = (unsigned char *)malloc(pixel_count * pixel_size);
    rgba_pixels = (unsigned char *)malloc(pixel_count * 4u);

    if ((source_pixels == NULL) || (rgba_pixels == NULL)) {
        free(source_pixels);
        free(rgba_pixels);
        fclose(file);
        copy_error_message(error_message, error_message_size, "Out of memory while loading TGA.");
        return 0;
    }

    if (fread(source_pixels, pixel_size, pixel_count, file) != pixel_count) {
        free(source_pixels);
        free(rgba_pixels);
        fclose(file);
        copy_error_message(error_message, error_message_size, "Failed to read TGA pixel data.");
        return 0;
    }

    fclose(file);

    for (i = 0; i < pixel_count; ++i) {
        size_t src = i * pixel_size;
        size_t dst = i * 4u;

        rgba_pixels[dst + 0u] = source_pixels[src + 2u];
        rgba_pixels[dst + 1u] = source_pixels[src + 1u];
        rgba_pixels[dst + 2u] = source_pixels[src + 0u];
        rgba_pixels[dst + 3u] = (pixel_size == 4u) ? source_pixels[src + 3u] : 255u;
    }

    free(source_pixels);

    *pixels_out = rgba_pixels;
    copy_error_message(error_message, error_message_size, "");
    return 1;
}

static int load_gl_texture_from_tga(
    const char *path,
    unsigned int *texture_out,
    char *error_message,
    size_t error_message_size
)
{
    unsigned char *pixels;
    int width;
    int height;

    if (!load_file_tga_rgba(path, &pixels, &width, &height, error_message, error_message_size)) {
        return 0;
    }

    glGenTextures(1, texture_out);
    glBindTexture(GL_TEXTURE_2D, *texture_out);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );

    free(pixels);
    copy_error_message(error_message, error_message_size, "");
    return 1;
}

static void join_asset_path(char *dst, size_t dst_size, const char *root, const char *filename)
{
    snprintf(dst, dst_size, "%s\\%s", root, filename);
}

static void apply_projection(const CornballKaarGlRenderer *renderer)
{
    float aspect;
    float half_width;

    aspect = 1.0f;
    if ((renderer->viewport_width > 0) && (renderer->viewport_height > 0)) {
        aspect = (float)renderer->viewport_width / (float)renderer->viewport_height;
    }

    half_width = kWorldHalfHeight * aspect;

    glViewport(0, 0, renderer->viewport_width, renderer->viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-half_width, half_width, -kWorldHalfHeight, kWorldHalfHeight, kWorldNear, kWorldFar);
    glMatrixMode(GL_MODELVIEW);
}

static void draw_textured_quad_uv(
    float half_width,
    float half_height,
    float depth_z,
    float texcoord_min_u,
    float texcoord_max_u,
    float texcoord_min_v,
    float texcoord_max_v
)
{
    glTexCoord2f(texcoord_max_u, texcoord_min_v);
    glVertex3f(half_width, -half_height, depth_z);

    glTexCoord2f(texcoord_max_u, texcoord_max_v);
    glVertex3f(half_width, half_height, depth_z);

    glTexCoord2f(texcoord_min_u, texcoord_max_v);
    glVertex3f(-half_width, half_height, depth_z);

    glTexCoord2f(texcoord_min_u, texcoord_min_v);
    glVertex3f(-half_width, -half_height, depth_z);
}

static void draw_layer_quad(const CornballKaarLayerQuad *quad)
{
    glColor4f(quad->color_r, quad->color_g, quad->color_b, quad->color_a);
    glBegin(GL_QUADS);
    draw_textured_quad_uv(
        quad->half_width,
        quad->half_height,
        quad->depth_z,
        quad->texcoord_min_u,
        quad->texcoord_max_u,
        quad->texcoord_min_v,
        quad->texcoord_max_v
    );
    glEnd();
}

static void draw_centered_textured_quad(float half_width, float half_height)
{
    glBegin(GL_QUADS);
    draw_textured_quad_uv(
        half_width,
        half_height,
        -2.0f,
        kCenteredTexcoordInset,
        1.0f - kCenteredTexcoordInset,
        kCenteredTexcoordInset,
        1.0f
    );
    glEnd();
}

static void build_tube_cache(CornballKaarGlRenderer *renderer)
{
    unsigned int ring_index;
    unsigned int segment_index;

    if (renderer->tube_cache_valid != 0u) {
        return;
    }

    for (ring_index = 0u; ring_index < 2u; ++ring_index) {
        float (*ring_vertices)[3];
        float depth_z = ((float)ring_index - kTubeRingZBias) * kTubeRingZScale;

        ring_vertices = (ring_index == 0u)
            ? renderer->tube_ring_vertices_neg_z
            : renderer->tube_ring_vertices_pos_z;

        for (segment_index = 0u; segment_index < CORNBALL_KAAR_TUBE_SEGMENT_COUNT; ++segment_index) {
            float angle = (float)segment_index * kTubeAngleStep;
            ring_vertices[segment_index][0] = cosf(angle) * kTubeRadius;
            ring_vertices[segment_index][1] = sinf(angle) * kTubeRadius;
            ring_vertices[segment_index][2] = depth_z;
        }
    }

    renderer->tube_cache_valid = 1u;
}

static void draw_cached_tube_shell(CornballKaarGlRenderer *renderer, double phase)
{
    unsigned int segment_index;
    float phase_neg_z;
    float phase_pos_z;

    build_tube_cache(renderer);

    phase_neg_z = (float)phase;
    phase_pos_z = (float)(phase + kTubePhaseOffset);

    glBegin(GL_QUADS);
    for (segment_index = 0u; segment_index < CORNBALL_KAAR_TUBE_SEGMENT_COUNT; ++segment_index) {
        unsigned int next_index = (segment_index + 1u) & (CORNBALL_KAAR_TUBE_SEGMENT_COUNT - 1u);
        float texcoord_v = (float)segment_index * kTubeTexcoordStep;
        float next_texcoord_v = (float)(segment_index + 1u) * kTubeTexcoordStep;

        glTexCoord2f(phase_neg_z, texcoord_v);
        glVertex3fv(renderer->tube_ring_vertices_neg_z[segment_index]);

        glTexCoord2f(phase_pos_z, texcoord_v);
        glVertex3fv(renderer->tube_ring_vertices_pos_z[segment_index]);

        glTexCoord2f(phase_pos_z, next_texcoord_v);
        glVertex3fv(renderer->tube_ring_vertices_pos_z[next_index]);

        glTexCoord2f(phase_neg_z, next_texcoord_v);
        glVertex3fv(renderer->tube_ring_vertices_neg_z[next_index]);
    }
    glEnd();
}

int cornball_kaar_gl_init(
    CornballKaarGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
)
{
    char kaar128_path[MAX_PATH];
    char txt1_path[MAX_PATH];
    char txt2_path[MAX_PATH];

    memset(renderer, 0, sizeof(*renderer));

    join_asset_path(kaar128_path, sizeof(kaar128_path), asset_root, "KAAR128.TGA");
    join_asset_path(txt1_path, sizeof(txt1_path), asset_root, "TXT1.TGA");
    join_asset_path(txt2_path, sizeof(txt2_path), asset_root, "TXT2.TGA");

    if (!load_gl_texture_from_tga(kaar128_path, &renderer->kaar128_texture, error_message, error_message_size)) {
        return 0;
    }

    if (!load_gl_texture_from_tga(txt1_path, &renderer->txt1_texture, error_message, error_message_size)) {
        glDeleteTextures(1, &renderer->kaar128_texture);
        renderer->kaar128_texture = 0u;
        return 0;
    }

    if (!load_gl_texture_from_tga(txt2_path, &renderer->txt2_texture, error_message, error_message_size)) {
        glDeleteTextures(1, &renderer->txt1_texture);
        renderer->txt1_texture = 0u;
        glDeleteTextures(1, &renderer->kaar128_texture);
        renderer->kaar128_texture = 0u;
        return 0;
    }

    renderer->viewport_width = 1;
    renderer->viewport_height = 1;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    copy_error_message(error_message, error_message_size, "");
    return 1;
}

void cornball_kaar_gl_shutdown(CornballKaarGlRenderer *renderer)
{
    if (renderer->kaar128_texture != 0u) {
        glDeleteTextures(1, &renderer->kaar128_texture);
        renderer->kaar128_texture = 0u;
    }

    if (renderer->txt1_texture != 0u) {
        glDeleteTextures(1, &renderer->txt1_texture);
        renderer->txt1_texture = 0u;
    }

    if (renderer->txt2_texture != 0u) {
        glDeleteTextures(1, &renderer->txt2_texture);
        renderer->txt2_texture = 0u;
    }
}

void cornball_kaar_gl_resize(CornballKaarGlRenderer *renderer, int width, int height)
{
    renderer->viewport_width = (width > 0) ? width : 1;
    renderer->viewport_height = (height > 0) ? height : 1;
    apply_projection(renderer);
}

void cornball_kaar_gl_render(const CornballKaarGlRenderer *renderer, const CornballKaarFrame *frame)
{
    float fog_color[4];
    CornballKaarGlRenderer *mutable_renderer;

    mutable_renderer = (CornballKaarGlRenderer *)renderer;

    glClear(GL_COLOR_BUFFER_BIT);
    apply_projection(renderer);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if ((renderer->txt1_texture != 0u) && (frame->jitter_overlay_quad.enabled != 0u)) {
        glBindTexture(GL_TEXTURE_2D, renderer->txt1_texture);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        draw_layer_quad(&frame->jitter_overlay_quad);
    }

    if ((renderer->kaar128_texture != 0u) && (frame->tube_shell.enabled != 0u)) {
        if (frame->fog.enabled != 0u) {
            fog_color[0] = frame->fog.color_r;
            fog_color[1] = frame->fog.color_g;
            fog_color[2] = frame->fog.color_b;
            fog_color[3] = frame->fog.color_a;

            glFogf(GL_FOG_DENSITY, frame->fog.density);
            glFogfv(GL_FOG_COLOR, fog_color);
            glFogf(GL_FOG_START, frame->fog.start_distance);
            glFogf(GL_FOG_END, frame->fog.end_distance);
            glHint(GL_FOG_HINT, GL_NICEST);
            glEnable(GL_FOG);
        }

        glBindTexture(GL_TEXTURE_2D, renderer->kaar128_texture);
        glEnable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBlendFunc(GL_ONE, GL_ONE);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(
            frame->tube_shell.translate_x,
            frame->tube_shell.translate_y,
            frame->tube_shell.translate_z
        );
        glRotatef(frame->tube_shell.rotate_x_degrees, 1.0f, 0.0f, 0.0f);
        glRotatef(frame->tube_shell.rotate_y_degrees, 0.0f, 1.0f, 0.0f);
        glRotatef(frame->tube_shell.rotate_z_degrees, 0.0f, 0.0f, 1.0f);
        draw_cached_tube_shell(mutable_renderer, frame->tube_shell.phase);
        glPopMatrix();

        glDisable(GL_FOG);
    }

    if ((renderer->txt1_texture != 0u) && (frame->txt1_center_quad.enabled != 0u)) {
        glBindTexture(GL_TEXTURE_2D, renderer->txt1_texture);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
        glColor4f(
            frame->txt1_center_quad.color_r,
            frame->txt1_center_quad.color_g,
            frame->txt1_center_quad.color_b,
            frame->txt1_center_quad.color_a
        );

        glPushMatrix();
        glLoadIdentity();
        glRotatef(frame->txt1_rotation_degrees, 0.0f, 0.0f, 1.0f);
        draw_centered_textured_quad(
            frame->txt1_center_quad.half_width,
            frame->txt1_center_quad.half_height
        );
        glPopMatrix();
    }
}
