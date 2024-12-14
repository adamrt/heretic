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

static void _camera_azimuth(transition_dir_e);
static void _camera_elevation(transition_dir_e);
static void _camera_process_transitions(void);
static camera_t _state;

void camera_init(void) {
    _state.target = glms_vec3_zero();
    _state.distance = 256.0f;
    _state.azimuth = DIR_SE;
    _state.elevation = CAMERA_ELV_LOW;
}

void camera_update(void) {
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

    f32 azimuth_rad = glm_rad(_state.azimuth);
    f32 elevation_rad = glm_rad(_state.elevation);

    _state.position.x = _state.distance * cosf(elevation_rad) * sinf(azimuth_rad);
    _state.position.y = _state.distance * sinf(elevation_rad);
    _state.position.z = _state.distance * -cosf(elevation_rad) * cosf(azimuth_rad);

    _state.position = glms_vec3_add(_state.target, _state.position);
}

mat4s camera_get_view(void) {
    return glms_lookat(_state.position, _state.target, GLMS_YUP);
}

mat4s camera_get_proj(void) {
    f32 aspect = sapp_widthf() / sapp_heightf();
    f32 w = _state.distance;
    f32 h = w / aspect;

    if (_state.use_perspective) {
        return glms_perspective(glm_rad(60.0f), aspect, 0.01f, 2000.0f);
    } else {
        return glms_ortho(-w, w, -h, h, 0.01f, 2000.0f);
    }
}

void camera_mouse_movement(f32 dx_deg, f32 dy_deg) {
    _state.azimuth += dx_deg;
    _state.elevation += dy_deg;
    _state.elevation = glm_clamp(_state.elevation, CAMERA_ELV_MIN, CAMERA_ELV_MAX);
}

void camera_mouse_wheel(f32 delta) {
    _state.distance -= delta * SENSITIVITY;
    _state.distance = glm_clamp(_state.distance, CAMERA_DIST_MIN, CAMERA_DIST_MAX);
}

void camera_key_left(void) { _camera_azimuth(TRANS_DIR_LEFT); }
void camera_key_right(void) { _camera_azimuth(TRANS_DIR_RIGHT); }
void camera_key_up(void) { _camera_elevation(TRANS_DIR_UP); }
void camera_key_down(void) { _camera_elevation(TRANS_DIR_DOWN); }

camera_t* camera_get_internals(void) { return &_state; }

// Move camera to the next intercardinal direction to the left.
//
// Compensate for the orbital camera free movement by moving the camera to the
// nearest intercardinal direction.
//
// Camera transitions deal with values < 0.0f and > 360.0f to simplify
// calculations. When a transition finishes, the camera_update() will bring the
// values back within 0.0f-360.0f range.
static void _camera_azimuth(transition_dir_e dir) {
    if (_state.transition.valid) {
        return;
    }

    bool move_left = dir == TRANS_DIR_LEFT;

    f32 degrees = _state.azimuth;
    f32 end_degrees = 0.0f;

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

    _state.transition = (transition_t) {
        .start_azimuth = _state.azimuth,
        .end_azimuth = end_degrees,
        .total_frames = CAMERA_TRANS_FRAMES,
        .valid = true,
    };
}

static void _camera_elevation(transition_dir_e dir) {
    if (_state.transition.valid) {
        return;
    }

    if ((dir == TRANS_DIR_DOWN && _state.elevation == CAMERA_ELV_LOW)
        || (dir == TRANS_DIR_UP && _state.elevation == CAMERA_ELV_HIGH)) {
        return;
    }

    bool move_up = dir == TRANS_DIR_UP;

    _state.transition = (transition_t) {
        .start_elevation = _state.elevation,
        .end_elevation = move_up ? CAMERA_ELV_HIGH : CAMERA_ELV_LOW,
        .total_frames = CAMERA_TRANS_FRAMES,
        .valid = true,
    };
}

static void _camera_process_transitions(void) {
    transition_t* trans = &_state.transition;
    if (!trans->valid) {
        return;
    }

    f32 t = trans->current_frame / trans->total_frames;

    if (trans->start_azimuth != trans->end_azimuth) {
        _state.azimuth = glm_lerp(trans->start_azimuth, trans->end_azimuth, t);
    }

    if (trans->start_elevation != trans->end_elevation) {
        _state.elevation = glm_lerp(trans->start_elevation, trans->end_elevation, t);
    }

    if (trans->current_frame >= trans->total_frames) {
        if (trans->start_azimuth != trans->end_azimuth) {
            _state.azimuth = trans->end_azimuth;
        }
        if (trans->start_elevation != trans->end_elevation) {
            _state.elevation = trans->end_elevation;
        }
        trans->valid = false;
    }

    trans->current_frame++;
}

cardinal_e camera_cardinal(void) {
    f32 azimuth = _state.azimuth;
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

const char* camera_cardinal_str(void) {
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
