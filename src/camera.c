#include "cglm/struct/vec3.h"
#include "game.h"
#include "sokol_app.h"

#include "camera.h"

#define SENSITIVITY (2.0f)

#define MIN_DIST (0.01f)
#define MAX_DIST (1000.0f)

#define MIN_ELEVATION (-M_PI_2 + 0.01f)
#define MAX_ELEVATION (M_PI_2 - 0.01f)

void camera_init(void)
{
    g.cam.target = glms_vec3_zero();
    g.cam.azimuth = glm_rad(30.0f);
    g.cam.elevation = glm_rad(20.0f);
    g.cam.znear = 0.001f;
    g.cam.zfar = 1000.0f;
    g.cam.zoom_factor = 256.0f;

    camera_update();
}

void camera_update(void)
{
    vec3s offset = { {
        cosf(g.cam.elevation) * sinf(g.cam.azimuth),
        sinf(g.cam.elevation),
        cosf(g.cam.elevation) * cosf(g.cam.azimuth),
    } };

    g.cam.eye = glms_vec3_add(g.cam.target, offset);
    g.cam.view = glms_lookat(g.cam.eye, g.cam.target, GLMS_YUP);

    float aspect = sapp_widthf() / sapp_heightf();
    float w = g.cam.zoom_factor;
    float h = w / aspect;
    g.cam.proj = glms_ortho(-w, w, -h, h, g.cam.znear, g.cam.zfar);
}

void camera_orbit(float dx_deg, float dy_deg)
{
    float dx_rad = glm_rad(dx_deg);
    float dy_rad = glm_rad(dy_deg);

#ifdef CGLM_FORCE_LEFT_HANDED
    g.cam.azimuth += dx_rad;
#else
    g.cam.azimuth -= dx_rad;
#endif

    g.cam.elevation += dy_rad;
    g.cam.elevation = glm_clamp(g.cam.elevation, MIN_ELEVATION, MAX_ELEVATION);
}

void camera_zoom(float delta)
{
    g.cam.zoom_factor -= delta * SENSITIVITY;
    g.cam.zoom_factor = glm_clamp(g.cam.zoom_factor, MIN_DIST, MAX_DIST);
}
