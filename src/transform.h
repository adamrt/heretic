#pragma once

#include "cglm/types-struct.h"

typedef struct {
    vec3s scale;
    vec2s screen_pos;
} transform2d_t;

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform3d_t;

mat4s transform_to_matrix(transform3d_t);
mat4s transform_to_matrix_around_center(transform3d_t, vec3s);
