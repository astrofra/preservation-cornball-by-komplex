#include "cornball/s_pair_gl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>

static const float kPerspectiveFovDegrees = 65.0f;
static const float kPerspectiveNear = 1.0f;
static const float kPerspectiveFar = 90.0f;
static const float kBipyramidBaseExtent = 60.0f;
static const float kBipyramidFrontZ = 60.0f;
static const float kBipyramidBackZ = -60.0f;

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
    unsigned char image_descriptor;
    unsigned char alpha_bits;

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
    image_descriptor = header[17];
    alpha_bits = (unsigned char)(image_descriptor & 0x0f);

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

    for (i = 0u; i < pixel_count; ++i) {
        size_t src = i * pixel_size;
        size_t dst = i * 4u;

        unsigned char red = source_pixels[src + 2u];
        unsigned char green = source_pixels[src + 1u];
        unsigned char blue = source_pixels[src + 0u];
        unsigned char alpha = 255u;

        if (pixel_size == 4u) {
            if (alpha_bits != 0u) {
                alpha = source_pixels[src + 3u];
            } else {
                alpha = red;
                if (green > alpha) {
                    alpha = green;
                }
                if (blue > alpha) {
                    alpha = blue;
                }
            }
        }

        rgba_pixels[dst + 0u] = red;
        rgba_pixels[dst + 1u] = green;
        rgba_pixels[dst + 2u] = blue;
        rgba_pixels[dst + 3u] = alpha;
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

static void apply_projection(const CornballSPairGlRenderer *renderer)
{
    float aspect = 1.0f;
    float top;
    float right;
    float radians;

    if ((renderer->viewport_width > 0) && (renderer->viewport_height > 0)) {
        aspect = (float)renderer->viewport_width / (float)renderer->viewport_height;
    }

    radians = (kPerspectiveFovDegrees * 3.14159265358979323846f) / 360.0f;
    top = kPerspectiveNear * tanf(radians);
    right = top * aspect;

    glViewport(0, 0, renderer->viewport_width, renderer->viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, kPerspectiveNear, kPerspectiveFar);
    glMatrixMode(GL_MODELVIEW);
}

static void draw_textured_quad_uv(const CornballSPairLayerQuad *quad)
{
    glTexCoord2f(quad->texcoord_max_u, quad->texcoord_min_v);
    glVertex3f(quad->half_width, -quad->half_height, quad->depth_z);

    glTexCoord2f(quad->texcoord_max_u, quad->texcoord_max_v);
    glVertex3f(quad->half_width, quad->half_height, quad->depth_z);

    glTexCoord2f(quad->texcoord_min_u, quad->texcoord_max_v);
    glVertex3f(-quad->half_width, quad->half_height, quad->depth_z);

    glTexCoord2f(quad->texcoord_min_u, quad->texcoord_min_v);
    glVertex3f(-quad->half_width, -quad->half_height, quad->depth_z);
}

static void draw_bipyramid_half(float apex_z)
{
    const float corners[4][2] = {
        {-kBipyramidBaseExtent, -kBipyramidBaseExtent},
        { kBipyramidBaseExtent, -kBipyramidBaseExtent},
        { kBipyramidBaseExtent,  kBipyramidBaseExtent},
        {-kBipyramidBaseExtent,  kBipyramidBaseExtent}
    };
    static const float corner_uvs[4][2] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    int i;

    for (i = 0; i < 4; ++i) {
        int next = (i + 1) & 3;

        glTexCoord2f(0.5f, 0.5f);
        glVertex3f(0.0f, 0.0f, apex_z);

        glTexCoord2f(corner_uvs[i][0], corner_uvs[i][1]);
        glVertex3f(corners[i][0], corners[i][1], 0.0f);

        glTexCoord2f(corner_uvs[next][0], corner_uvs[next][1]);
        glVertex3f(corners[next][0], corners[next][1], 0.0f);
    }
}

static void draw_dual_texture_bipyramid(
    const CornballSPairGlRenderer *renderer,
    const CornballSPairBipyramidPass *pass
)
{
    glColor4f(pass->color_r, pass->color_g, pass->color_b, pass->color_a);
    glPushMatrix();
    glRotatef(pass->rotate_z_pre_degrees, 0.0f, 0.0f, 1.0f);
    glRotatef(pass->rotate_y_degrees, 0.0f, 1.0f, 0.0f);
    glRotatef(pass->rotate_x_degrees, 1.0f, 0.0f, 0.0f);
    glRotatef(pass->rotate_z_post_degrees, 0.0f, 0.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, renderer->s1_texture);
    glBegin(GL_TRIANGLES);
    draw_bipyramid_half(kBipyramidFrontZ);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, renderer->s2_texture);
    glBegin(GL_TRIANGLES);
    draw_bipyramid_half(kBipyramidBackZ);
    glEnd();

    glPopMatrix();
}

static void draw_overlay_quad(const CornballSPairLayerQuad *quad)
{
    glColor4f(quad->color_r, quad->color_g, quad->color_b, quad->color_a);
    glBegin(GL_QUADS);
    draw_textured_quad_uv(quad);
    glEnd();
}

int cornball_s_pair_gl_init(
    CornballSPairGlRenderer *renderer,
    const char *asset_root,
    char *error_message,
    size_t error_message_size
)
{
    char s1_path[MAX_PATH];
    char s2_path[MAX_PATH];
    char txt2_path[MAX_PATH];

    memset(renderer, 0, sizeof(*renderer));

    join_asset_path(s1_path, sizeof(s1_path), asset_root, "S1.TGA");
    join_asset_path(s2_path, sizeof(s2_path), asset_root, "S2.TGA");
    join_asset_path(txt2_path, sizeof(txt2_path), asset_root, "TXT2.TGA");

    if (!load_gl_texture_from_tga(s1_path, &renderer->s1_texture, error_message, error_message_size)) {
        return 0;
    }

    if (!load_gl_texture_from_tga(s2_path, &renderer->s2_texture, error_message, error_message_size)) {
        cornball_s_pair_gl_shutdown(renderer);
        return 0;
    }

    if (!load_gl_texture_from_tga(txt2_path, &renderer->txt2_texture, error_message, error_message_size)) {
        cornball_s_pair_gl_shutdown(renderer);
        return 0;
    }

    renderer->viewport_width = 1;
    renderer->viewport_height = 1;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);

    copy_error_message(error_message, error_message_size, "");
    return 1;
}

void cornball_s_pair_gl_shutdown(CornballSPairGlRenderer *renderer)
{
    if (renderer->s1_texture != 0u) {
        glDeleteTextures(1, &renderer->s1_texture);
        renderer->s1_texture = 0u;
    }

    if (renderer->s2_texture != 0u) {
        glDeleteTextures(1, &renderer->s2_texture);
        renderer->s2_texture = 0u;
    }

    if (renderer->txt2_texture != 0u) {
        glDeleteTextures(1, &renderer->txt2_texture);
        renderer->txt2_texture = 0u;
    }
}

void cornball_s_pair_gl_resize(CornballSPairGlRenderer *renderer, int width, int height)
{
    renderer->viewport_width = (width > 0) ? width : 1;
    renderer->viewport_height = (height > 0) ? height : 1;
    apply_projection(renderer);
}

void cornball_s_pair_gl_render(const CornballSPairGlRenderer *renderer, const CornballSPairFrame *frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    apply_projection(renderer);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);

    if (frame->bipyramid.enabled != 0u) {
        draw_dual_texture_bipyramid(renderer, &frame->bipyramid);
    }

    glLoadIdentity();

    if ((renderer->txt2_texture != 0u) && (frame->overlay_quad.enabled != 0u)) {
        glBindTexture(GL_TEXTURE_2D, renderer->txt2_texture);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        draw_overlay_quad(&frame->overlay_quad);
    }
}
