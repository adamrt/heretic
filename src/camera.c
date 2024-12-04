#include "cglm/struct/cam.h"
#include "cglm/struct/vec3.h"
#include "cglm/types-struct.h"

#include "sokol_app.h"

#include "camera.h"
#include "util.h"

#define SENSITIVITY (2.0f)

#define DIR_N   (0.0f)
#define DIR_NNE (22.5f)
#define DIR_NE  (45.0f)
#define DIR_ENE (67.5f)
#define DIR_E   (90.0f)
#define DIR_ESE (112.5f)
#define DIR_SE  (135.0f)
#define DIR_SSE (157.5f)
#define DIR_S   (180.0f)
#define DIR_SSW (202.5f)
#define DIR_SW  (225.0f)
#define DIR_WSW (247.5f)
#define DIR_W   (270.0f)
#define DIR_WNW (292.5f)
#define DIR_NW  (315.0f)
#define DIR_NNW (337.5f)

#define CAMERA_ELV_MIN (-89.1)
#define CAMERA_ELV_MAX (89.1)

#define CAMERA_ELV_LOW  (30.0f)
#define CAMERA_ELV_HIGH (45.0f)

#define CAMERA_TRANS_FRAMES (45.0f)

// FIXME: Add a type two switch between cameras.
//
// Rename camera_t to game_camera_t along with the functions and then add public
// functions (camera_init(), camera_left(), etc) to switch between the two that
// feeds input to both internally.
static struct {
    camera_t cam;
    orbit_camera_t orb;
} _state;

// Forward declarations
static void _orbit_camera_azimuth(transition_dir_e);
static void _orbit_camera_elevation(transition_dir_e);
static void _camera_process_transitions(void);

void camera_init(void)
{
    _state.cam.position = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    _state.cam.yaw = 0.0f;
    _state.cam.pitch = -30.0f;
    _state.cam.znear = 0.01f;
    _state.cam.zfar = 2000.0f;
    _state.cam.frustum_scale = 128.0f;
}

void camera_update(void)
{
    float yaw_rad = glm_rad(_state.cam.yaw);
    float pitch_rad = glm_rad(_state.cam.pitch);

    // pitch_rad = glm_clamp(pitch_rad, -89.1f, 89.1f);

    vec3s forward = glms_vec3_normalize((vec3s) { {
        cosf(pitch_rad) * sinf(yaw_rad),
        sinf(pitch_rad),
        -cosf(pitch_rad) * cosf(yaw_rad),
    } });

    vec3s target = glms_vec3_add(_state.cam.position, forward);
    _state.cam.view_mat = glms_lookat(_state.cam.position, target, GLMS_YUP);

    float aspect = sapp_widthf() / sapp_heightf();
    float w = _state.cam.frustum_scale;
    float h = w / aspect;

    if (_state.cam.use_perspective) {
        _state.cam.proj_mat = glms_perspective(glm_rad(60.0f), aspect, _state.cam.znear, _state.cam.zfar);
    } else {
        _state.cam.proj_mat = glms_ortho(-w, w, -h, h, _state.cam.znear, _state.cam.zfar);
    }
}

camera_t* camera_get_internals(void) { return &_state.cam; }
mat4s camera_get_view(void) { return _state.cam.view_mat; }
mat4s camera_get_proj(void) { return _state.cam.proj_mat; }

void orbit_camera_init(void)
{
    _state.orb.target = glms_vec3_zero();

    _state.orb.azimuth = DIR_SE;
    _state.orb.elevation = CAMERA_ELV_LOW;

    _state.orb.distance = 256.0f;
    _state.orb.znear = 0.01f;
    _state.orb.zfar = 1000.0f;
}

void orbit_camera_update(void)
{
    float aspect = sapp_widthf() / sapp_heightf();
    float w = _state.orb.distance;
    float h = w / aspect;

    _camera_process_transitions();

    // Transitions work with negative degrees sometimes so don't keep camera
    // position degrees positive mid-transition.
    if (!_state.orb.transition.valid) {
        if (_state.orb.azimuth >= 360.0f) {
            _state.orb.azimuth -= 360.0f;
        }
        if (_state.orb.azimuth < 0.0f) {
            _state.orb.azimuth += 360.0f;
        }
    }

    float azimuth_rad = glm_rad(_state.orb.azimuth);
    float elevation_rad = glm_rad(_state.orb.elevation);

    vec3s position = { {
        cosf(elevation_rad) * sinf(azimuth_rad),
        sinf(elevation_rad),
        -cosf(elevation_rad) * cosf(azimuth_rad),
    } };

    vec3s scaled_position = glms_vec3_scale(position, _state.orb.distance);

    _state.orb.eye = glms_vec3_add(_state.orb.target, scaled_position);
    _state.orb.view_mat = glms_lookat(_state.orb.eye, _state.orb.target, GLMS_YUP);

    if (_state.orb.use_perspective) {
        _state.orb.proj_mat = glms_perspective(glm_rad(60.0f), aspect, _state.orb.znear, _state.orb.zfar);
    } else {
        _state.orb.proj_mat = glms_ortho(-w, w, -h, h, _state.orb.znear, _state.orb.zfar);
    }
}

orbit_camera_t* orbit_camera_get_internals(void) { return &_state.orb; }
mat4s orbit_camera_get_view(void) { return _state.orb.view_mat; }
mat4s orbit_camera_get_proj(void) { return _state.orb.proj_mat; }

void orbit_camera_orbit(float dx_deg, float dy_deg)
{
    _state.orb.azimuth += dx_deg;
    _state.orb.elevation += dy_deg;
    _state.orb.elevation = glm_clamp(_state.orb.elevation, CAMERA_ELV_MIN, CAMERA_ELV_MAX);
}

void orbit_camera_zoom(float delta)
{
    _state.orb.distance -= delta * SENSITIVITY;
    _state.orb.distance = glm_clamp(_state.orb.distance, CAMERA_DIST_MIN, CAMERA_DIST_MAX);
}

void orbit_camera_left(void) { _orbit_camera_azimuth(TRANS_DIR_LEFT); }
void orbit_camera_right(void) { _orbit_camera_azimuth(TRANS_DIR_RIGHT); }
void orbit_camera_up(void) { _orbit_camera_elevation(TRANS_DIR_UP); }
void orbit_camera_down(void) { _orbit_camera_elevation(TRANS_DIR_DOWN); }

cardinal_e orbit_camera_cardinal(void)
{
    float azimuth = _state.orb.azimuth;
    int corner_width = 15;

    if (azimuth > 345.0f || azimuth < 15.0f) {
        return CARDINAL_N;
    }
    if (fabs(azimuth - DIR_W) < corner_width) {
        return CARDINAL_W;
    }
    if (fabs(azimuth - DIR_S) < corner_width) {
        return CARDINAL_S;
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
static void _orbit_camera_azimuth(transition_dir_e dir)
{
    if (_state.orb.transition.valid) {
        return;
    }

    bool move_left = dir == TRANS_DIR_LEFT;

    float degrees = _state.orb.azimuth;
    float end_degrees = 0.0f;

    if (degrees == DIR_NE) {
        end_degrees = move_left ? DIR_SE : DIR_NW - 360.0f;
    } else if (degrees == DIR_SE) {
        end_degrees = move_left ? DIR_SW : DIR_NE;
    } else if (degrees == DIR_SW) {
        end_degrees = move_left ? DIR_NW : DIR_SE;
    } else if (degrees == DIR_NW) {
        end_degrees = move_left ? DIR_NE + 360.0f : DIR_SW;
    } else if (degrees > DIR_NE && degrees < DIR_SE) {
        end_degrees = move_left ? DIR_SE : DIR_NE;
    } else if (degrees > DIR_SE && degrees < DIR_SW) {
        end_degrees = move_left ? DIR_SW : DIR_SE;
    } else if (degrees > DIR_SW && degrees < DIR_NW) {
        end_degrees = move_left ? DIR_NW : DIR_SW;
    } else if (degrees > DIR_NW && degrees <= 360.0f) {
        end_degrees = move_left ? DIR_NE + 360.0f : DIR_NW;
    } else if (degrees >= 0.0f && degrees < DIR_NE) {
        end_degrees = move_left ? DIR_NE : DIR_NW - 360.0f;
    } else {
        ASSERT(false, "Invalid azimuth %f", degrees);
    }

    _state.orb.transition = (transition_t) {
        .start_azimuth = _state.orb.azimuth,
        .end_azimuth = end_degrees,
        .total_frames = CAMERA_TRANS_FRAMES,
        .valid = true,
    };
}

static void _orbit_camera_elevation(transition_dir_e dir)
{
    if (_state.orb.transition.valid) {
        return;
    }

    if ((dir == TRANS_DIR_DOWN && _state.orb.elevation == CAMERA_ELV_LOW)
        || (dir == TRANS_DIR_UP && _state.orb.elevation == CAMERA_ELV_HIGH)) {
        return;
    }

    bool move_up = dir == TRANS_DIR_UP;

    _state.orb.transition = (transition_t) {
        .start_elevation = _state.orb.elevation,
        .end_elevation = move_up ? CAMERA_ELV_HIGH : CAMERA_ELV_LOW,
        .total_frames = CAMERA_TRANS_FRAMES,
        .valid = true,
    };
}

static void _camera_process_transitions(void)
{
    transition_t* trans = &_state.orb.transition;
    if (!trans->valid) {
        return;
    }

    float t = trans->current_frame / trans->total_frames;

    if (trans->start_azimuth != trans->end_azimuth) {
        _state.orb.azimuth = glm_lerp(trans->start_azimuth, trans->end_azimuth, t);
    }

    if (trans->start_elevation != trans->end_elevation) {
        _state.orb.elevation = glm_lerp(trans->start_elevation, trans->end_elevation, t);
    }

    if (trans->current_frame >= trans->total_frames) {
        if (trans->start_azimuth != trans->end_azimuth) {
            _state.orb.azimuth = trans->end_azimuth;
        }
        if (trans->start_elevation != trans->end_elevation) {
            _state.orb.elevation = trans->end_elevation;
        }
        trans->valid = false;
    }

    trans->current_frame++;
}

const char* orbit_camera_cardinal_str(void)
{
    cardinal_e dir = orbit_camera_cardinal();
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
