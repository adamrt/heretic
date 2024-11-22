#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/struct.h"

#define CAM_DIST_MIN  (0.01f)
#define CAM_DIST_MAX  (1000.0f)
#define CAM_ZNEAR_MIN (0.01f)
#define CAM_ZFAR_MAX  (1000.0f)

typedef struct {
    mat4s proj_mat, view_mat;
    vec3s eye, target;
    float azimuth, elevation; // In degrees
    float znear, zfar;
    float distance;
    bool use_perspective;
} camera_t;

void camera_init(void);
void camera_update(void);
void camera_orbit(float, float);
void camera_zoom(float);

void camera_left(void);
void camera_right(void);
void camera_up(void);
void camera_down(void);

#endif // CAMERA_H_
