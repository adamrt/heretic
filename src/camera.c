#include "cglm/struct/cam.h"
#include "cglm/struct/vec3.h"
#include "cglm/types-struct.h"

#include "sokol_app.h"

#include "camera.h"

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

#define CAMERA_ELV_MIN (-89.1)
#define CAMERA_ELV_MAX (89.1)

#define CAMERA_ELV_LOW  (30.0f)
#define CAMERA_ELV_HIGH (45.0f)

#define CAMERA_TRANS_FRAMES (45.0f)

static camera_t _state;

// Forward declarations
static void _camera_azimuth(transition_dir_e);
static void _camera_elevation(transition_dir_e);
static void _camera_process_transitions(void);

void camera_init(void)
{
    _state.target = glms_vec3_zero();

    _state.azimuth = DIR_SE;
    _state.elevation = CAMERA_ELV_LOW;

    _state.distance = 256.0f;
    _state.znear = 0.01f;
    _state.zfar = 1000.0f;
}

void camera_update(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = _state.distance;
    float h = w / aspect;

    _camera_process_transitions();

    // Transitions work with negative degrees sometimes so don't keep camera
    // position degrees positive mid-transition.
    if (!_state.transition.valid) {
        if (_state.azimuth >= 360.0f) {
            _state.azimuth -= 360.0f;
        }
        if (_state.azimuth < 0.0f) {
            _state.azimuth += 360.0f;
        }
    }

    float azimuth_rad = glm_rad(_state.azimuth);
    float elevation_rad = glm_rad(_state.elevation);

    vec3s position = { {
        cosf(elevation_rad) * sinf(azimuth_rad),
        sinf(elevation_rad),
        cosf(elevation_rad) * cosf(azimuth_rad),
    } };

    vec3s scaled_position = glms_vec3_scale(position, _state.distance);

    _state.eye = glms_vec3_add(_state.target, scaled_position);
    _state.view_mat = glms_lookat(_state.eye, _state.target, GLMS_YUP);

    if (_state.use_perspective) {
        _state.proj_mat = glms_perspective(glm_rad(60.0f), aspect, _state.znear, _state.zfar);
    } else {
        _state.proj_mat = glms_ortho(-w, w, -h, h, _state.znear, _state.zfar);
    }
}

camera_t* camera_get_internals(void) { return &_state; }
mat4s camera_get_view(void) { return _state.view_mat; }
mat4s camera_get_proj(void) { return _state.proj_mat; }

void camera_orbit(float dx_deg, float dy_deg)
{
    _state.azimuth -= dx_deg;
    _state.elevation += dy_deg;
    _state.elevation = glm_clamp(_state.elevation, CAMERA_ELV_MIN, CAMERA_ELV_MAX);
}

void camera_zoom(float delta)
{
    _state.distance -= delta * SENSITIVITY;
    _state.distance = glm_clamp(_state.distance, CAMERA_DIST_MIN, CAMERA_DIST_MAX);
}

void camera_left(void) { _camera_azimuth(TRANS_DIR_LEFT); }
void camera_right(void) { _camera_azimuth(TRANS_DIR_RIGHT); }
void camera_up(void) { _camera_elevation(TRANS_DIR_UP); }
void camera_down(void) { _camera_elevation(TRANS_DIR_DOWN); }

cardinal_e camera_cardinal(void)
{
    float azimuth = _state.azimuth;
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
static void _camera_azimuth(transition_dir_e dir)
{
    if (_state.transition.valid) {
        return;
    }

    bool move_left = dir == TRANS_DIR_LEFT;

    float degrees = _state.azimuth;
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

    _state.transition = (transition_t) {
        .direction = dir,
        .start_degrees = _state.azimuth,
        .end_degrees = end_degrees,
        .total_frames = CAMERA_TRANS_FRAMES,
        .current_frame = 0.0f,
        .valid = true,
    };
}

static void _camera_elevation(transition_dir_e dir)
{
    if (_state.transition.valid) {
        return;
    }

    if (dir == TRANS_DIR_DOWN) {
        if (_state.elevation == CAMERA_ELV_LOW) {
            return;
        }

        _state.transition = (transition_t) {
            .direction = dir,
            .start_degrees = _state.elevation,
            .end_degrees = CAMERA_ELV_LOW,
            .total_frames = 30.0f,
            .current_frame = 0.0f,
            .valid = true,
        };
    } else {
        if (_state.elevation == CAMERA_ELV_HIGH) {
            return;
        }

        _state.transition = (transition_t) {
            .direction = dir,
            .start_degrees = _state.elevation,
            .end_degrees = CAMERA_ELV_HIGH,
            .total_frames = 30.0f,
            .current_frame = 0.0f,
            .valid = true,
        };
    }
}

static void _camera_process_transitions(void)
{
    if (!_state.transition.valid) {
        return;
    }

    float t = _state.transition.current_frame / _state.transition.total_frames;
    float new_pos = glm_lerp(_state.transition.start_degrees, _state.transition.end_degrees, t);

    if (_state.transition.direction == TRANS_DIR_LEFT || _state.transition.direction == TRANS_DIR_RIGHT) {
        _state.azimuth = new_pos;
    } else if (_state.transition.direction == TRANS_DIR_UP || _state.transition.direction == TRANS_DIR_DOWN) {
        _state.elevation = new_pos;
    }

    if (_state.transition.current_frame >= _state.transition.total_frames) {
        if (_state.transition.direction == TRANS_DIR_LEFT || _state.transition.direction == TRANS_DIR_RIGHT) {
            _state.azimuth = _state.transition.end_degrees;
        } else if (_state.transition.direction == TRANS_DIR_UP || _state.transition.direction == TRANS_DIR_DOWN) {
            _state.elevation = _state.transition.end_degrees;
        }
        _state.transition.valid = false;
    }

    _state.transition.current_frame++;
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
