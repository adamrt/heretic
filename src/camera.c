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

typedef enum {
    LEFT,
    RIGHT,
} movment_t;

typedef struct {
    movment_t movment;

    float start_degrees;
    float end_degrees;

    float current_frame;
    float total_frames;

    bool valid;
} transition_t;

static transition_t transition;

static void process_transitions(void);

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

    process_transitions();

    // Transitions work with negative degrees sometimes so don't keep camera
    // position degrees positive mid-transition.
    if (!transition.valid) {
        if (g.cam.pos_degrees >= 360.0f) {
            g.cam.pos_degrees -= 360.0f;
        }
        if (g.cam.pos_degrees < 0.0f) {
            g.cam.pos_degrees += 360.0f;
        }
    }

    g.cam.azimuth = glm_rad(g.cam.pos_degrees);

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

    g.cam.pos_degrees = glm_deg(g.cam.azimuth);
}

void camera_zoom(float delta)
{
    g.cam.distance -= delta * SENSITIVITY;
    g.cam.distance = glm_clamp(g.cam.distance, CAM_MIN_DIST, CAM_MAX_DIST);
}

void camera_left(void)
{
    if (transition.valid) {
        return;
    }

    float pos = g.cam.pos_degrees;
    float end_degrees = pos - 90.0f;

    // Handle the camera when not in the intercardinal directions
    if (pos > CAM_SE && pos < CAM_NE) {
        end_degrees = CAM_SE;
    } else if (pos > CAM_NE && pos < CAM_NW) {
        end_degrees = CAM_NE;
    } else if (pos > CAM_NW && pos < CAM_SW) {
        end_degrees = CAM_NW;
    } else if ((pos > CAM_SW && pos <= 360.0f) || (pos < CAM_SE && pos >= 0.0f)) {
        // Handle 360 wrap around
        end_degrees = CAM_SW - 360.0f;
    }

    transition = (transition_t) {
        .movment = LEFT,
        .start_degrees = g.cam.pos_degrees,
        .end_degrees = end_degrees,
        .total_frames = 60.0f,
        .current_frame = 0.0f,
        .valid = true,
    };
}

void camera_right(void)
{
    if (transition.valid) {
        return;
    }

    float pos = g.cam.pos_degrees;
    float end_degrees = pos + 90.0f;

    // Handle the camera when not in the intercardinal directions
    if (pos > CAM_SE && pos < CAM_NE) {
        end_degrees = CAM_NE;
    } else if (pos > CAM_NE && pos < CAM_NW) {
        end_degrees = CAM_NW;
    } else if (pos > CAM_NW && pos < CAM_SW) {
        end_degrees = CAM_SW;
    } else if ((pos > CAM_SW && pos <= 360.0f) || (pos < CAM_SE && pos >= 0.0f)) {
        // Handle 360 wrap around
        end_degrees = CAM_SE + 360.0f;
    }

    transition = (transition_t) {
        .movment = RIGHT,
        .start_degrees = g.cam.pos_degrees,
        .end_degrees = end_degrees,
        .total_frames = 60.0f,
        .current_frame = 0.0f,
        .valid = true,
    };
}

static void process_transitions(void)
{
    if (!transition.valid) {
        return;
    }

    float t = transition.current_frame / transition.total_frames;
    float new_pos = glm_lerp(transition.start_degrees, transition.end_degrees, t);

    g.cam.pos_degrees = new_pos;

    if (transition.current_frame >= transition.total_frames) {
        g.cam.pos_degrees = transition.end_degrees;
        transition.valid = false;
    }

    transition.current_frame++;
}
