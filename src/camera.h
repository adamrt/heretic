#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/struct.h"

void camera_init(void);
void camera_update(void);
void camera_rotate(float azimuth, float elevation);
void camera_zoom(float distance);

mat4s camera_get_proj(void);
mat4s camera_get_view(void);

#endif // CAMERA_H_
