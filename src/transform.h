#pragma once

#include "cglm/types-struct.h"

// Unified transform for both 2D and 3D sprites
// 2D: translation.xy = screen position, translation.z = 0, rotation = zero
// 3D: translation.xyz = world position, rotation.xyz = Euler angles
typedef struct {
    vec3s translation;  // x,y for screen pos (2D) or world pos (3D)
    vec3s rotation;     // all zeros for 2D sprites
    vec3s scale;
} transform_t;

mat4s transform_to_matrix(transform_t);
mat4s transform_to_matrix_around_center(transform_t, vec3s);
