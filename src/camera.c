#include "sokol_app.h"

#include "camera.h"

#define MAX_ELEVATION (M_PI_2 - 0.01f)
#define MIN_ELEVATION (-M_PI_2 - 0.01f)
#define MIN_DIST      (0.5f)
#define MAX_DIST      (5.0f)
#define SENSITIVITY   (0.02f)
#define ZNEAR         (-10.0f)
#define ZFAR          (10.0f)

typedef struct {
    mat4s proj;
    mat4s view;

    vec3s eye;
    vec3s center;
    vec3s up;

    float azimuth;
    float elevation;
    float zoom_factor;
} camera_t;

static camera_t cam;

void camera_init(void)
{
    cam.center = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    cam.azimuth = 0.6f;
    cam.elevation = 0.4f;
    cam.zoom_factor = 1.0f;
    cam.up = (vec3s) { { 0.0f, 1.0f, 0.0f } };

    camera_update();
}

static void camera_update_proj(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = cam.zoom_factor;
    float h = w / aspect;

    cam.proj = glms_ortho(-w, w, -h, h, ZNEAR, ZFAR);
}

void camera_update(void)
{
    // Convert azimuth and elevation (in radians) to spherical coordinates
    float x = cosf(cam.elevation) * sinf(cam.azimuth);
    float y = sinf(cam.elevation);
    float z = cosf(cam.elevation) * cosf(cam.azimuth);

    cam.eye = (vec3s) { { cam.center.x + x, cam.center.y + y, cam.center.z + z } };
    cam.view = glms_lookat(cam.eye, cam.center, cam.up);

    camera_update_proj();
}

void camera_rotate(float delta_azimuth, float delta_elevation)
{
    cam.azimuth = cam.azimuth - (delta_azimuth * SENSITIVITY);
    cam.elevation = cam.elevation + (delta_elevation * SENSITIVITY);
    cam.elevation = glm_clamp(cam.elevation, MIN_ELEVATION, MAX_ELEVATION);
}

void camera_zoom(float delta)
{
    cam.zoom_factor -= delta * SENSITIVITY;
    cam.zoom_factor = glm_clamp(cam.zoom_factor, MIN_DIST, MAX_DIST);
}

mat4s camera_get_proj(void) { return cam.proj; }
mat4s camera_get_view(void) { return cam.view; }
