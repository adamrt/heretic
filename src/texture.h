#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cglm/types-struct.h"
#include "defines.h"
#include "map_record.h"
#include "model.h"
#include "span.h"

typedef struct {
    map_state_t map_state; // only useful for actual map/level textures.
    int width;
    int height;
    u8* data;
    usize size;
    bool valid;
} texture_t;

void texture_destroy(texture_t);

texture_t read_texture(span_t*);
texture_t read_palette(span_t*);
vec4s read_rgb15(span_t* span);

sg_image texture_to_sg_image(texture_t);
