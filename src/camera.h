#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/struct.h"

typedef struct {
    mat4s proj, view;
    vec3s eye, target;
    float azimuth, elevation;
    float znear, zfar;
    float zoom_factor;
} camera_t;

void camera_init(void);
void camera_update(void);
void camera_orbit(float, float);
void camera_zoom(float);

#endif // CAMERA_H_
