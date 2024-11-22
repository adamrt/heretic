#include "cglm/struct/vec3.h"

#include "sokol_app.h"

#include "camera.h"
#include "game.h"

#define SENSITIVITY (2.0f)

#define MIN_ELEVATION (-1.56079632679489661923132169163975144) /* -M_PI_2 + 0.01 */
#define MAX_ELEVATION (1.56079632679489661923132169163975144)  /*  M_PI_2 - 0.01 */

#define CAM_ELV_LOW  (30.0f)
#define CAM_ELV_HIGH (45.0f)

#define CAM_AZM_SE (45.0f)
#define CAM_AZM_NE (135.0f)
#define CAM_AZM_NW (225.0f)
#define CAM_AZM_SW (315.0f)

#define CAM_TRANS_FRAMES (45.0f)

typedef enum {
    TRANS_DIR_LEFT,
    TRANS_DIR_RIGHT,
    TRANS_DIR_UP,
    TRANS_DIR_DOWN,
} trans_dir_e;

typedef struct {
    trans_dir_e direction;

    float start_degrees;
    float end_degrees;

    float current_frame;
    float total_frames;

    bool valid;
} transition_t;

// Global camera state
static transition_t transition;

// Forward declarations
static void cam_azimuth(trans_dir_e);
static void cam_elevation(trans_dir_e);
static void cam_process_transitions(void);

void camera_init(void)
{
    g.cam.target = glms_vec3_zero();

    g.cam.azimuth_degrees = CAM_AZM_SE;
    g.cam.elevation_degrees = CAM_ELV_LOW;

    g.cam.azimuth = glm_rad(g.cam.azimuth_degrees);
    g.cam.elevation = glm_rad(g.cam.elevation_degrees);

    g.cam.distance = 256.0f;
    g.cam.znear = 0.01f;
    g.cam.zfar = 1000.0f;
}

void camera_update(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = g.cam.distance;
    float h = w / aspect;

    cam_process_transitions();

    // Transitions work with negative degrees sometimes so don't keep camera
    // position degrees positive mid-transition.
    if (!transition.valid) {
        if (g.cam.azimuth_degrees >= 360.0f) {
            g.cam.azimuth_degrees -= 360.0f;
        }
        if (g.cam.azimuth_degrees < 0.0f) {
            g.cam.azimuth_degrees += 360.0f;
        }
    }

    g.cam.azimuth = glm_rad(g.cam.azimuth_degrees);
    g.cam.elevation = glm_rad(g.cam.elevation_degrees);

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
    g.cam.azimuth_degrees = glm_deg(g.cam.azimuth);

    g.cam.elevation += dy_rad;
    g.cam.elevation = glm_clamp(g.cam.elevation, MIN_ELEVATION, MAX_ELEVATION);
    g.cam.elevation_degrees = glm_deg(g.cam.elevation);
}

void camera_zoom(float delta)
{
    g.cam.distance -= delta * SENSITIVITY;
    g.cam.distance = glm_clamp(g.cam.distance, CAM_MIN_DIST, CAM_MAX_DIST);
}

void camera_left(void) { cam_azimuth(TRANS_DIR_LEFT); }
void camera_right(void) { cam_azimuth(TRANS_DIR_RIGHT); }
void camera_up(void) { cam_elevation(TRANS_DIR_UP); }
void camera_down(void) { cam_elevation(TRANS_DIR_DOWN); }

// Move camera to the next intercardinal direction to the left.
// Compensate for the orbital camera by moving the camera to the nearest
// intercardinal direction.
static void cam_azimuth(trans_dir_e dir)
{
    if (transition.valid) {
        return;
    }

    bool move_left = dir == TRANS_DIR_LEFT;

    float pos = g.cam.azimuth_degrees;
    float end_degrees = move_left ? pos - 90.0f : pos + 90.0f;

    if (pos > CAM_AZM_SE && pos < CAM_AZM_NE) {
        end_degrees = move_left ? CAM_AZM_SE : CAM_AZM_NE;
    } else if (pos > CAM_AZM_NE && pos < CAM_AZM_NW) {
        end_degrees = move_left ? CAM_AZM_NE : CAM_AZM_NW;
    } else if (pos > CAM_AZM_NW && pos < CAM_AZM_SW) {
        end_degrees = move_left ? CAM_AZM_NW : CAM_AZM_SW;
    } else if ((pos > CAM_AZM_SW && pos <= 360.0f) || (pos < CAM_AZM_SE && pos >= 0.0f)) {
        // Handle 360 wrap around
        end_degrees = move_left ? CAM_AZM_SW - 360.0f : CAM_AZM_SE + 360.0f;
    }

    transition = (transition_t) {
        .direction = dir,
        .start_degrees = g.cam.azimuth_degrees,
        .end_degrees = end_degrees,
        .total_frames = CAM_TRANS_FRAMES,
        .current_frame = 0.0f,
        .valid = true,
    };
}

static void cam_elevation(trans_dir_e dir)
{
    if (transition.valid) {
        return;
    }

    if (dir == TRANS_DIR_DOWN) {
        if (g.cam.elevation_degrees == CAM_ELV_LOW) {
            return;
        }

        transition = (transition_t) {
            .direction = dir,
            .start_degrees = g.cam.elevation_degrees,
            .end_degrees = CAM_ELV_LOW,
            .total_frames = 30.0f,
            .current_frame = 0.0f,
            .valid = true,
        };
    } else {
        if (g.cam.elevation_degrees == CAM_ELV_HIGH) {
            return;
        }

        transition = (transition_t) {
            .direction = dir,
            .start_degrees = g.cam.elevation_degrees,
            .end_degrees = CAM_ELV_HIGH,
            .total_frames = 30.0f,
            .current_frame = 0.0f,
            .valid = true,
        };
    }
}

static void cam_process_transitions(void)
{
    if (!transition.valid) {
        return;
    }

    float t = transition.current_frame / transition.total_frames;
    float new_pos = glm_lerp(transition.start_degrees, transition.end_degrees, t);

    if (transition.direction == TRANS_DIR_LEFT || transition.direction == TRANS_DIR_RIGHT) {
        g.cam.azimuth_degrees = new_pos;
    } else if (transition.direction == TRANS_DIR_UP || transition.direction == TRANS_DIR_DOWN) {
        g.cam.elevation_degrees = new_pos;
    }

    if (transition.current_frame >= transition.total_frames) {
        if (transition.direction == TRANS_DIR_LEFT || transition.direction == TRANS_DIR_RIGHT) {
            g.cam.azimuth_degrees = transition.end_degrees;
        } else if (transition.direction == TRANS_DIR_UP || transition.direction == TRANS_DIR_DOWN) {
            g.cam.elevation_degrees = transition.end_degrees;
        }
        transition.valid = false;
    }

    transition.current_frame++;
}
