#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "bin.h"
#include "defines.h"

#define LIGHTING_MAX_LIGHTS (3)

typedef struct {
    vec3s direction;
    vec4s color;

    bool valid;
} light_t;

// Lighting is a collection of lights, ambient color, and background colors.
// This is because of hose the data is stored on the PSX bin.
typedef struct {
    light_t lights[LIGHTING_MAX_LIGHTS];

    vec4s ambient_color;
    f32 ambient_strength;

    vec4s bg_top;
    vec4s bg_bottom;

    bool valid;
} lighting_t;

lighting_t read_lighting(span_t*);
