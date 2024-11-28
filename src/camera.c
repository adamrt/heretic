#include "cglm/struct/vec3.h"

#include "sokol_app.h"

#include "camera.h"
#include "game.h"

#define SENSITIVITY (2.0f)

#define DIR_S   (0.0f)
#define DIR_W   (270.0f)
#define DIR_N   (180.0f)
#define DIR_E   (90.0f)
#define DIR_SW  (315.0f)
#define DIR_NW  (225.0f)
#define DIR_NE  (135.0f)
#define DIR_SE  (45.0f)
#define DIR_SSW (337.5f)
#define DIR_WSW (292.5f)
#define DIR_WNW (247.5f)
#define DIR_NNW (202.5f)
#define DIR_NNE (157.5f)
#define DIR_ENE (112.5f)
#define DIR_ESE (67.5f)
#define DIR_SSE (22.5f)

#define CAM_ELV_MIN (-89.1)
#define CAM_ELV_MAX (89.1)

#define CAM_ELV_LOW  (30.0f)
#define CAM_ELV_HIGH (45.0f)

#define CAM_TRANS_FRAMES (45.0f)

static camera_t state;

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
    state.target = glms_vec3_zero();

    state.azimuth = DIR_SE;
    state.elevation = CAM_ELV_LOW;

    state.distance = 256.0f;
    state.znear = 0.01f;
    state.zfar = 1000.0f;
}

void camera_update(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = state.distance;
    float h = w / aspect;

    cam_process_transitions();

    // Transitions work with negative degrees sometimes so don't keep camera
    // position degrees positive mid-transition.
    if (!transition.valid) {
        if (state.azimuth >= 360.0f) {
            state.azimuth -= 360.0f;
        }
        if (state.azimuth < 0.0f) {
            state.azimuth += 360.0f;
        }
    }

    float azimuth_rad = glm_rad(state.azimuth);
    float elevation_rad = glm_rad(state.elevation);

    vec3s position = { {
        cosf(elevation_rad) * sinf(azimuth_rad),
        sinf(elevation_rad),
        cosf(elevation_rad) * cosf(azimuth_rad),
    } };

    vec3s scaled_position = glms_vec3_scale(position, state.distance);

    state.eye = glms_vec3_add(state.target, scaled_position);
    state.view_mat = glms_lookat(state.eye, state.target, GLMS_YUP);

    if (state.use_perspective) {
        state.proj_mat = glms_perspective(glm_rad(60.0f), aspect, state.znear, state.zfar);
    } else {
        state.proj_mat = glms_ortho(-w, w, -h, h, state.znear, state.zfar);
    }
}

camera_t* camera_get_internals(void) { return &state; }
mat4s camera_get_view(void) { return state.view_mat; }
mat4s camera_get_proj(void) { return state.proj_mat; }

void camera_orbit(float dx_deg, float dy_deg)
{
    state.azimuth -= dx_deg;
    state.elevation += dy_deg;
    state.elevation = glm_clamp(state.elevation, CAM_ELV_MIN, CAM_ELV_MAX);
}

void camera_zoom(float delta)
{
    state.distance -= delta * SENSITIVITY;
    state.distance = glm_clamp(state.distance, CAM_DIST_MIN, CAM_DIST_MAX);
}

void camera_left(void) { cam_azimuth(TRANS_DIR_LEFT); }
void camera_right(void) { cam_azimuth(TRANS_DIR_RIGHT); }
void camera_up(void) { cam_elevation(TRANS_DIR_UP); }
void camera_down(void) { cam_elevation(TRANS_DIR_DOWN); }

cardinal_e camera_cardinal(void)
{
    float azimuth = state.azimuth;
    int corner_width = 15;

    if (azimuth > 345.0f || azimuth < 15.0f) {
        return CARDINAL_S;
    }
    if (fabs(azimuth - DIR_W) < corner_width) {
        return CARDINAL_W;
    }
    if (fabs(azimuth - DIR_N) < corner_width) {
        return CARDINAL_N;
    }
    if (fabs(azimuth - DIR_E) < corner_width) {
        return CARDINAL_E;
    }
    if (fabs(azimuth - DIR_SW) < corner_width) {
        return CARDINAL_SW;
    }
    if (fabs(azimuth - DIR_NW) < corner_width) {
        return CARDINAL_NW;
    }
    if (fabs(azimuth - DIR_NE) < corner_width) {
        return CARDINAL_NE;
    }
    if (fabs(azimuth - DIR_SE) < corner_width) {
        return CARDINAL_SE;
    }
    if (fabs(azimuth - DIR_SSW) < corner_width) {
        return CARDINAL_SSW;
    }
    if (fabs(azimuth - DIR_WSW) < corner_width) {
        return CARDINAL_WSW;
    }
    if (fabs(azimuth - DIR_WNW) < corner_width) {
        return CARDINAL_WNW;
    }
    if (fabs(azimuth - DIR_NNW) < corner_width) {
        return CARDINAL_NNW;
    }
    if (fabs(azimuth - DIR_NNE) < corner_width) {
        return CARDINAL_NNE;
    }
    if (fabs(azimuth - DIR_ENE) < corner_width) {
        return CARDINAL_ENE;
    }
    if (fabs(azimuth - DIR_ESE) < corner_width) {
        return CARDINAL_ESE;
    }
    if (fabs(azimuth - DIR_SSE) < corner_width) {
        return CARDINAL_SSE;
    }
    return CARDINAL_UNKNOWN;
}

// Move camera to the next intercardinal direction to the left.
//
// Compensate for the orbital camera free movement by moving the camera to the
// nearest intercardinal direction.
//
// Camera transitions deal with values < 0.0f and > 360.0f to simplify
// calculations. When a transition finishes, the camera_update() will bring the
// values back within 0.0f-360.0f range.
static void cam_azimuth(trans_dir_e dir)
{
    if (transition.valid) {
        return;
    }

    bool move_left = dir == TRANS_DIR_LEFT;

    float degrees = state.azimuth;
    float end_degrees = move_left ? degrees - 90.0f : degrees + 90.0f;

    if (degrees > DIR_SE && degrees < DIR_NE) {
        end_degrees = move_left ? DIR_SE : DIR_NE;
    } else if (degrees > DIR_NE && degrees < DIR_NW) {
        end_degrees = move_left ? DIR_NE : DIR_NW;
    } else if (degrees > DIR_NW && degrees < DIR_SW) {
        end_degrees = move_left ? DIR_NW : DIR_SW;
    } else if (degrees > DIR_SW || degrees < DIR_SE) {
        end_degrees = move_left ? DIR_SW : DIR_SE;

        // Handle 360 wrap around
        if (move_left && degrees < 180.0f) {
            end_degrees -= 360.0f;
        } else if (!move_left && degrees > 180.0f) {
            end_degrees += 360.0f;
        }
    }

    transition = (transition_t) {
        .direction = dir,
        .start_degrees = state.azimuth,
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
        if (state.elevation == CAM_ELV_LOW) {
            return;
        }

        transition = (transition_t) {
            .direction = dir,
            .start_degrees = state.elevation,
            .end_degrees = CAM_ELV_LOW,
            .total_frames = 30.0f,
            .current_frame = 0.0f,
            .valid = true,
        };
    } else {
        if (state.elevation == CAM_ELV_HIGH) {
            return;
        }

        transition = (transition_t) {
            .direction = dir,
            .start_degrees = state.elevation,
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
        state.azimuth = new_pos;
    } else if (transition.direction == TRANS_DIR_UP || transition.direction == TRANS_DIR_DOWN) {
        state.elevation = new_pos;
    }

    if (transition.current_frame >= transition.total_frames) {
        if (transition.direction == TRANS_DIR_LEFT || transition.direction == TRANS_DIR_RIGHT) {
            state.azimuth = transition.end_degrees;
        } else if (transition.direction == TRANS_DIR_UP || transition.direction == TRANS_DIR_DOWN) {
            state.elevation = transition.end_degrees;
        }
        transition.valid = false;
    }

    transition.current_frame++;
}

const char* camera_cardinal_str(void)
{
    cardinal_e dir = camera_cardinal();
    switch (dir) {
    case CARDINAL_S:
        return "South";
    case CARDINAL_W:
        return "West";
    case CARDINAL_N:
        return "North";
    case CARDINAL_E:
        return "East";
    case CARDINAL_SW:
        return "Southwest";
    case CARDINAL_NW:
        return "Northwest";
    case CARDINAL_NE:
        return "Northeast";
    case CARDINAL_SE:
        return "Southeast";
    case CARDINAL_SSW:
        return "South-Southwest";
    case CARDINAL_WSW:
        return "West-Southwest";
    case CARDINAL_WNW:
        return "West-Northwest";
    case CARDINAL_NNW:
        return "North-Northwest";
    case CARDINAL_NNE:
        return "North-Northeast";
    case CARDINAL_ENE:
        return "East-Northeast";
    case CARDINAL_ESE:
        return "East-Southeast";
    case CARDINAL_SSE:
        return "South-Southeast";
    default:
        return "Unknown";
    }
}
