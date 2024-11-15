#include "sokol_app.h"

#include "camera.h"

#define SENSITIVITY (0.02f)

#define ZNEAR (-10.0f)
#define ZFAR  (10.0f)

#define MIN_DIST (0.5f)
#define MAX_DIST (5.0f)

#define MIN_ELEVATION (-M_PI_2 + 0.01f)
#define MAX_ELEVATION (M_PI_2 - 0.01f)

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

static void camera_update_proj(void);

void camera_init(void)
{
    cam.center = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    cam.azimuth = glm_rad(30.0f);
    cam.elevation = glm_rad(20.0f);
    cam.zoom_factor = 1.0f;
    cam.up = (vec3s) { { 0.0f, 1.0f, 0.0f } };

    camera_update();
    camera_update_proj();
}

void camera_update(void)
{
    float x = cosf(cam.elevation) * sinf(cam.azimuth);
    float y = sinf(cam.elevation);
    float z = cosf(cam.elevation) * cosf(cam.azimuth);

    cam.eye = (vec3s) { { cam.center.x + x, cam.center.y + y, cam.center.z + z } };
    cam.view = glms_lookat(cam.eye, cam.center, cam.up);
}

static void camera_update_proj(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = cam.zoom_factor;
    float h = w / aspect;
    cam.proj = glms_ortho(-w, w, -h, h, ZNEAR, ZFAR);
}

void camera_orbit(float dx_deg, float dy_deg)
{
    float dx_rad = glm_rad(dx_deg);
    float dy_rad = glm_rad(dy_deg);

#ifdef CGLM_FORCE_LEFT_HANDED
    cam.azimuth += dx_rad;
#else
    cam.azimuth -= dx_rad;
#endif

    cam.elevation += dy_rad;
    cam.elevation = glm_clamp(cam.elevation, MIN_ELEVATION, MAX_ELEVATION);
}

void camera_zoom(float delta)
{
    cam.zoom_factor -= delta * SENSITIVITY;
    cam.zoom_factor = glm_clamp(cam.zoom_factor, MIN_DIST, MAX_DIST);

    camera_update_proj(); // Zoom affects the projection matrix
}

mat4s camera_get_proj(void) { return cam.proj; }
mat4s camera_get_view(void) { return cam.view; }
