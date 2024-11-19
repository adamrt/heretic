#include "cglm/struct/vec3.h"

#include "sokol_app.h"

#include "camera.h"
#include "game.h"

#define SENSITIVITY (2.0f)

#define MIN_ELEVATION (-M_PI_2 + 0.01)
#define MAX_ELEVATION (M_PI_2 - 0.01)

void camera_init(void)
{
    g.cam.target = glms_vec3_zero();
    g.cam.azimuth = glm_rad(30.0f);
    g.cam.elevation = glm_rad(30.0f);
    g.cam.distance = 256.0f;
    g.cam.znear = 0.01f;
    g.cam.zfar = 1000.0f;
}

void camera_update(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = g.cam.distance;
    float h = w / aspect;

    vec3s position = { {
        cosf(g.cam.elevation) * sinf(g.cam.azimuth),
        sinf(g.cam.elevation),
        cosf(g.cam.elevation) * cosf(g.cam.azimuth),
    } };

    vec3s scaled_position = glms_vec3_scale(position, g.cam.distance);

    g.cam.eye = glms_vec3_add(g.cam.target, scaled_position);
    g.cam.view_mat = glms_lookat(g.cam.eye, g.cam.target, GLMS_YUP);

    if (g.cam.use_perspective) {
        g.cam.proj_mat = glms_perspective(glm_rad(60.0f), aspect, g.cam.znear, g.cam.zfar);
    } else {
        g.cam.proj_mat = glms_ortho(-w, w, -h, h, g.cam.znear, g.cam.zfar);
    }
}

void camera_orbit(float dx_deg, float dy_deg)
{
    float dx_rad = glm_rad(dx_deg);
    float dy_rad = glm_rad(dy_deg);

    g.cam.azimuth -= dx_rad;
    g.cam.elevation += dy_rad;
    g.cam.elevation = glm_clamp(g.cam.elevation, MIN_ELEVATION, MAX_ELEVATION);
}

void camera_zoom(float delta)
{
    g.cam.distance -= delta * SENSITIVITY;
    g.cam.distance = glm_clamp(g.cam.distance, CAM_MIN_DIST, CAM_MAX_DIST);
}
