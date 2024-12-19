#include "cglm/util.h"

#include "defines.h"
#include "parse.h"

f32 parse_coord(i16 value) {
    return (f32)value / 4.0f;
}

f32 parse_rad(i16 value) {
    f32 deg = (f32)value / 1024.0f * 90.0f;
    return -glm_rad(deg);
}

f32 parse_zoom(i16 value) {
    return (f32)value / 4096.0f;
}
