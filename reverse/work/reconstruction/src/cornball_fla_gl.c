#include "cornball/fla_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>

static const float kWorldHalfWidth = 20.0f;
static const float kWorldBottom = -6.0f;
static const float kWorldTop = 30.0f;

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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

static void apply_projection(const CornballFlaGlRenderer *renderer)
{
    float aspect;
    float half_width;

    aspect = 1.0f;
    if ((renderer->viewport_width > 0) && (renderer->viewport_height > 0)) {
        aspect = (float)renderer->viewport_width / (float)renderer->viewport_height;
    }

    half_width = kWorldHalfWidth * aspect;

    glViewport(0, 0, renderer->viewport_width, renderer->viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-half_width, half_width, kWorldBottom, kWorldTop, -1.0, 1.0);
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

static void draw_mirrored_textured_quad(float center_x, float center_y, float half_width, float half_height)
{
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(center_x + half_width, center_y - half_height, 0.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(center_x + half_width, center_y + half_height, 0.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(center_x - half_width, center_y + half_height, 0.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(center_x - half_width, center_y - half_height, 0.0f);
}

static void draw_layer_quad(const CornballFlaLayerQuad *quad)
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

int cornball_fla_gl_init(
    CornballFlaGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
)
{
    char fla_path[MAX_PATH];
    char logotaus_path[MAX_PATH];
    char txt1_path[MAX_PATH];

    memset(renderer, 0, sizeof(*renderer));

    join_asset_path(fla_path, sizeof(fla_path), asset_root, "FLA.TGA");
    join_asset_path(logotaus_path, sizeof(logotaus_path), asset_root, "LOGOTAUS.TGA");
    join_asset_path(txt1_path, sizeof(txt1_path), asset_root, "TXT1.TGA");

    if (!load_gl_texture_from_tga(fla_path, &renderer->fla_texture, error_message, error_message_size)) {
        return 0;
    }

    if (!load_gl_texture_from_tga(logotaus_path, &renderer->logotaus_texture, error_message, error_message_size)) {
        glDeleteTextures(1, &renderer->fla_texture);
        renderer->fla_texture = 0u;
        return 0;
    }

    if (!load_gl_texture_from_tga(txt1_path, &renderer->txt1_texture, error_message, error_message_size)) {
        glDeleteTextures(1, &renderer->logotaus_texture);
        renderer->logotaus_texture = 0u;
        glDeleteTextures(1, &renderer->fla_texture);
        renderer->fla_texture = 0u;
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

void cornball_fla_gl_shutdown(CornballFlaGlRenderer *renderer)
{
    if (renderer->fla_texture != 0u) {
        glDeleteTextures(1, &renderer->fla_texture);
        renderer->fla_texture = 0u;
    }

    if (renderer->logotaus_texture != 0u) {
        glDeleteTextures(1, &renderer->logotaus_texture);
        renderer->logotaus_texture = 0u;
    }

    if (renderer->txt1_texture != 0u) {
        glDeleteTextures(1, &renderer->txt1_texture);
        renderer->txt1_texture = 0u;
    }
}

void cornball_fla_gl_resize(CornballFlaGlRenderer *renderer, int width, int height)
{
    renderer->viewport_width = (width > 0) ? width : 1;
    renderer->viewport_height = (height > 0) ? height : 1;
    apply_projection(renderer);
}

void cornball_fla_gl_render(const CornballFlaGlRenderer *renderer, const CornballFlaFrame *frame)
{
    size_t i;

    glClear(GL_COLOR_BUFFER_BIT);
    apply_projection(renderer);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if ((renderer->logotaus_texture != 0u) && (frame->logotaus_quad.enabled != 0u)) {
        glBindTexture(GL_TEXTURE_2D, renderer->logotaus_texture);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glPushMatrix();
        glRotatef(frame->rotation_degrees, 0.0f, 0.0f, 1.0f);
        draw_layer_quad(&frame->logotaus_quad);
        glPopMatrix();
    }

    glBindTexture(GL_TEXTURE_2D, renderer->fla_texture);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

    glBegin(GL_QUADS);
    for (i = 0; i < frame->quad_count; ++i) {
        const CornballFlaDrawQuad *quad = &frame->quads[i];
        float shade = quad->grayscale;

        glColor4f(shade, shade, shade, 1.0f);
        draw_mirrored_textured_quad(
            quad->translate_x,
            quad->translate_y,
            quad->half_width,
            quad->half_height
        );
    }
    glEnd();

    if ((renderer->txt1_texture != 0u) && (frame->txt1_overlay_quad.enabled != 0u)) {
        glBindTexture(GL_TEXTURE_2D, renderer->txt1_texture);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        draw_layer_quad(&frame->txt1_overlay_quad);
    }
}
