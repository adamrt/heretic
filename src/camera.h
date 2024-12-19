#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "defines.h"

#define CAMERA_DIST_MIN    (0.1f)
#define CAMERA_DIST_MAX    (1000.0f)
#define CAMERA_ZOOM_MIN    (0.1f)
#define CAMERA_ZOOM_MAX    (3.0f)
#define CAMERA_FRUSTUM_MIN (0.1f)
#define CAMERA_FRUSTUM_MAX (512.0f)

typedef struct {
    vec3s position;
    f32 yaw_rad;
    f32 pitch_rad;
    f32 zoom;
    f32 frustum;
    bool use_perspective;
} camera_t;

typedef struct {
    f32 theta_rad;
    f32 phi_rad;
    f32 radius;
} spherical_t;

typedef struct {
    f32 forward;
    f32 right;
    f32 up;

    f32 yaw_deg;
    f32 pitch_deg;
} freefly_motion_t;

typedef struct {
    f32 theta_deg;
    f32 phi_deg;
    f32 dolly;
} orbit_motion_t;

void camera_init(void);
void camera_reset(void);
mat4s camera_get_view(void);
mat4s camera_get_proj(void);

void camera_freefly_motion(freefly_motion_t);
void camera_orbit_motion(orbit_motion_t);

void camera_set_orbit(vec3s, f32, f32, f32);
void camera_set_freefly(vec3s, f32, f32, f32);

camera_t* camera_get_internals(void);
spherical_t camera_get_spherical(void);
