#include "cornball/capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int cornball_capture_write_tga24(
    const char *output_path,
    int width,
    int height,
    const unsigned char *rgb_pixels,
    char *error_message,
    size_t error_message_size
)
{
    FILE *file;
    unsigned char header[18];
    unsigned char *bgr_row;
    size_t row_size;
    int y;

    if ((output_path == NULL) || (rgb_pixels == NULL) || (width <= 0) || (height <= 0)) {
        copy_error_message(error_message, error_message_size, "Invalid TGA capture arguments.");
        return 0;
    }

    file = fopen(output_path, "wb");
    if (file == NULL) {
        copy_error_message(error_message, error_message_size, "Failed to open TGA output path.");
        return 0;
    }

    row_size = (size_t)width * 3u;
    bgr_row = (unsigned char *)malloc(row_size);
    if (bgr_row == NULL) {
        fclose(file);
        copy_error_message(error_message, error_message_size, "Out of memory while writing TGA.");
        return 0;
    }

    memset(header, 0, sizeof(header));
    header[2] = 2u;
    header[12] = (unsigned char)(width & 0xff);
    header[13] = (unsigned char)((width >> 8) & 0xff);
    header[14] = (unsigned char)(height & 0xff);
    header[15] = (unsigned char)((height >> 8) & 0xff);
    header[16] = 24u;

    if (fwrite(header, sizeof(header), 1u, file) != 1u) {
        free(bgr_row);
        fclose(file);
        copy_error_message(error_message, error_message_size, "Failed to write TGA header.");
        return 0;
    }

    for (y = 0; y < height; ++y) {
        const unsigned char *src_row = rgb_pixels + ((size_t)y * row_size);
        int x;

        for (x = 0; x < width; ++x) {
            size_t src = (size_t)x * 3u;
            size_t dst = (size_t)x * 3u;

            bgr_row[dst + 0u] = src_row[src + 2u];
            bgr_row[dst + 1u] = src_row[src + 1u];
            bgr_row[dst + 2u] = src_row[src + 0u];
        }

        if (fwrite(bgr_row, row_size, 1u, file) != 1u) {
            free(bgr_row);
            fclose(file);
            copy_error_message(error_message, error_message_size, "Failed to write TGA pixel data.");
            return 0;
        }
    }

    free(bgr_row);
    fclose(file);
    copy_error_message(error_message, error_message_size, "");
    return 1;
}
