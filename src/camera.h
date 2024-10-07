#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/struct.h"

#define CAMERA_MIN_DIST (0.5f)
#define CAMERA_MAX_DIST (5.0f)

void camera_init(void);
void camera_update(void);
void camera_rotate(float azimuth, float elevation);
void camera_zoom(float distance);

mat4s camera_proj(void);
mat4s camera_view(void);

#endif // CAMERA_H_
