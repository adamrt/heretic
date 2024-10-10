#include "sokol_app.h"

#include "camera.h"

#define CAMERA_MIN_DIST (0.5f)
#define CAMERA_MAX_DIST (5.0f)

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
    float near_plane = -10.0f;
    float far_plane = 10.0f;
    float aspect = sapp_widthf() / sapp_heightf();

    float ortho_width = cam.zoom_factor;
    float ortho_height = ortho_width / aspect;

    cam.proj = glms_ortho(-ortho_width, ortho_width, -ortho_height, ortho_height, near_plane, far_plane);
}

void camera_update(void)
{
    // Convert azimuth and elevation (in radians) to spherical coordinates
    float x = cosf(cam.elevation) * sinf(cam.azimuth);
    float y = sinf(cam.elevation);
    float z = cosf(cam.elevation) * cosf(cam.azimuth);

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
    float min_elevation = -0.15f;
    cam.elevation = glm_clamp(cam.elevation, min_elevation, max_elevation);
}

void camera_zoom(float delta)
{
    cam.zoom_factor -= delta;
    cam.zoom_factor = glm_clamp(cam.zoom_factor, 0.1f, 5.0f);
}

mat4s camera_get_proj(void) { return cam.proj; }
mat4s camera_get_view(void) { return cam.view; }
