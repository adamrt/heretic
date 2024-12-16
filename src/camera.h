#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "defines.h"

#define CAMERA_DIST_MIN (0.01f)
#define CAMERA_DIST_MAX (1000.0f)

typedef struct {
    vec3s position;
    vec3s target;
    bool use_perspective;
} camera_t;

typedef struct {
    f32 theta;
    f32 phi;
    f32 radius;
} spherical_t;

typedef struct {
    vec2s oribit;
    f32 dolly;
} motion_t;

void camera_init(void);
void camera_update(void);
mat4s camera_get_view(void);
mat4s camera_get_proj(void);

void camera_update_transform(motion_t);

camera_t* camera_get_internals(void);
spherical_t camera_get_spherical(void);
