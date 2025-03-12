#pragma once

#include "defines.h"

typedef enum {
    COORD_X,
    COORD_Y,
    COORD_Z,
} coord_t;

f32 parse_coord(coord_t, i16);
f32 parse_rad(i16);
f32 parse_zoom(i16);
