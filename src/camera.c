#include "cglm/struct/vec3.h"

#include "sokol_app.h"

#include "camera.h"
#include "game.h"

#define SENSITIVITY (2.0f)

#define MIN_ELEVATION -1.56079632679489661923132169163975144 /* -M_PI_2 + 0.01 */
#define MAX_ELEVATION 1.56079632679489661923132169163975144  /*  M_PI_2 - 0.01 */

#define CAM_SE 45.0f
#define CAM_NE 135.0f
#define CAM_NW 225.0f
#define CAM_SW 315.0f

void camera_init(void)
{
    g.cam.target = glms_vec3_zero();

    g.cam.pos_degrees = CAM_SE;
    g.cam.azimuth = glm_rad(g.cam.pos_degrees);
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

    g.cam.pos_degrees = glm_deg(g.cam.azimuth);
    if (g.cam.pos_degrees < 0.0f) {
        g.cam.pos_degrees += 360.0f;
    }
    if (g.cam.pos_degrees >= 360.0f) {
        g.cam.pos_degrees -= 360.0f;
    }

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

void camera_left(void)
{
    g.cam.pos_degrees -= 90.0f;
    g.cam.azimuth = glm_rad(g.cam.pos_degrees);
}

void camera_right(void)
{
    g.cam.pos_degrees += 90.0f;
    g.cam.azimuth = glm_rad(g.cam.pos_degrees);
}
