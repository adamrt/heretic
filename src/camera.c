#include "sokol_app.h"

#include "camera.h"

typedef struct {
    mat4s proj;
    mat4s view;

    vec3s eye;
    vec3s center;
    vec3s up;

    float azimuth;
    float elevation;
    float distance;
} camera_t;

static camera_t cam;

void camera_init(void)
{
    cam.center = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    cam.distance = 1.5f;
    cam.azimuth = 0.6f;
    cam.elevation = 0.4f;
    cam.up = (vec3s) { { 0.0f, 1.0f, 0.0f } };

    camera_update();
}

static void camera_update_proj(void)
{
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    cam.proj = glms_perspective(glm_rad(60.0f), w / h, 0.01f, 100.0f);
}

void camera_update(void)
{
    // Convert azimuth and elevation (in radians) to spherical coordinates
    float x = cam.distance * cosf(cam.elevation) * sinf(cam.azimuth);
    float y = cam.distance * sinf(cam.elevation);
    float z = cam.distance * cosf(cam.elevation) * cosf(cam.azimuth);

    // center is typically (0, 0, 0), so the addition doesn't matter, but as we
    // add transitions it could become important.
    cam.eye = (vec3s) { { cam.center.x + x, cam.center.y + y, cam.center.z + z } };
    cam.view = glms_lookat(cam.eye, cam.center, cam.up);

    camera_update_proj();
}

void camera_rotate(float delta_azimuth, float delta_elevation)
{
    cam.azimuth -= delta_azimuth;
    cam.elevation += delta_elevation;

    float max_elevation = M_PI_2 - 0.01f; // Near 90 degrees
    float min_elevation = -max_elevation;
    cam.elevation = glm_clamp(cam.elevation, min_elevation, max_elevation);
}

void camera_zoom(float delta)
{
    cam.distance -= delta;
    cam.distance = glm_clamp(cam.distance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
}

mat4s camera_proj(void) { return cam.proj; }
mat4s camera_view(void) { return cam.view; }
