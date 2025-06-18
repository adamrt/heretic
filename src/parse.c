#include "cglm/util.h"

#include "defines.h"
#include "parse.h"

// The original data is stored as a fixed-point number.
f32 parse_coord(coord_t coord, i16 value) {
    f32 rv = (f32)value;
    if (coord == COORD_Y || coord == COORD_Z) {
        // rv = -rv;
    }
    rv = rv / 4.0f;
    return rv;
}

// The original data is in degrees and stored as a fixed-point number.
// 4096 == 1 full rotation (360 degrees).
f32 parse_deg(i16 value) {
    return (f32)value / 4096.0f * 360.0f;
}

// The original data is stored as a fixed-point number.
f32 parse_zoom(i16 value) {
    return (f32)value / 4096.0f;
}
