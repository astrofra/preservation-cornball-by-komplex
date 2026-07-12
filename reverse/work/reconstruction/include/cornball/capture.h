#ifndef CORNBALL_CAPTURE_H
#define CORNBALL_CAPTURE_H

#include <stddef.h>

int cornball_capture_write_tga24(
    const char *output_path,
    int width,
    int height,
    const unsigned char *rgb_pixels,
    char *error_message,
    size_t error_message_size
);

#endif
