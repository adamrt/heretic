#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "defines.h"

#define CAMERA_DIST_MIN (0.01f)
#define CAMERA_DIST_MAX (1000.0f)

typedef struct {
    vec3s position;
    f32 yaw;
    f32 pitch;
    bool use_perspective;
} camera_t;

typedef struct {
    f32 theta;
    f32 phi;
    f32 radius;
} spherical_t;

typedef struct {
    float forward; // W/S
    float right;   // A/D
    float up;      // R/F
    float dx;      // mouse dx
    float dy;      // mouse dy
} freefly_motion_t;

typedef struct {
    vec2s orbit;
    f32 dolly;
} orbit_motion_t;

void camera_init(void);
void camera_update(void);
mat4s camera_get_view(void);
mat4s camera_get_proj(void);

void camera_freefly_motion(freefly_motion_t);
void camera_orbit_motion(orbit_motion_t);

void camera_set_orbit(vec3s center, f32 theta, f32 phi, f32 radius);
void camera_set_freefly(vec3s position, f32 yaw, f32 pitch);

camera_t* camera_get_internals(void);
spherical_t camera_get_spherical(void);
