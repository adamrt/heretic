#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "map_record.h"
#include "span.h"

typedef struct {
    map_state_t map_state; // only useful for actual map/level textures.

    int width;
    int height;
    u8* data;
    usize size;

    bool valid;
} image_t;

void image_destroy(image_t);
image_t image_read_4bpp_image(span_t*, int, int);
image_t image_read_rgb15_image(span_t*, int, int);
