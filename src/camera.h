#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "defines.h"

#define CAMERA_DIST_MIN  (0.01f)
#define CAMERA_DIST_MAX  (1000.0f)
#define CAMERA_ZNEAR_MIN (0.01f)
#define CAMERA_ZFAR_MAX  (2000.0f)

typedef enum {
    TRANS_DIR_LEFT,
    TRANS_DIR_RIGHT,
    TRANS_DIR_UP,
    TRANS_DIR_DOWN,
} transition_dir_e;

typedef struct {
    vec3s start_position;
    vec3s end_position;

    f32 start_azimuth;
    f32 end_azimuth;

    f32 start_elevation;
    f32 end_elevation;

    f32 current_frame;
    f32 total_frames;

    bool valid;
} transition_t;

typedef enum {
    CAMTYPE_GAME = 0,
    CAMTYPE_ORBIT = 1,
} camtype_e;

typedef struct {
    mat4s proj_mat, view_mat;
    vec3s position, target;
    f32 azimuth, elevation; // In degrees
    f32 znear, zfar;
    f32 frustum_scale;
    bool use_perspective;
    transition_t transition;
} orbit_camera_t;

typedef struct {
    mat4s proj_mat, view_mat;
    vec3s position;
    f32 yaw, pitch;
    f32 znear, zfar;
    f32 frustum_scale;
    bool use_perspective;
} game_camera_t;

void camera_init(void);
void camera_update(void);
mat4s camera_get_view(void);
mat4s camera_get_proj(void);
camtype_e camera_get_type(void);
void camera_set_type(camtype_e);

void camera_mouse_movement(f32, f32);
void camera_mouse_wheel(f32);
void camera_key_left(void);
void camera_key_right(void);
void camera_key_up(void);
void camera_key_down(void);

game_camera_t* game_camera_get_internals(void);
orbit_camera_t* orbit_camera_get_internals(void);

// These are listed clockwise to match the FFT data storage. We might change
// later since we use counter-clockwise for the camera since we are RHS. These
// values match the polygon visibility data layout (unless its backwards).
typedef enum {
    CARDINAL_SW = 0x2,
    CARDINAL_NW = 0x3,
    CARDINAL_NE = 0x4,
    CARDINAL_SE = 0x5,
    CARDINAL_SSW = 0x6,
    CARDINAL_WSW = 0x7,
    CARDINAL_WNW = 0x8,
    CARDINAL_NNW = 0x9,
    CARDINAL_NNE = 0xA,
    CARDINAL_ENE = 0xB,
    CARDINAL_ESE = 0xC,
    CARDINAL_SSE = 0xD,

    // These are our own values just to represent all directions.
    CARDINAL_S = 0xF0,
    CARDINAL_W = 0xF1,
    CARDINAL_N = 0xF2,
    CARDINAL_E = 0xF3,
    CARDINAL_UNKNOWN = 0x0,
} cardinal_e;

cardinal_e orbit_camera_cardinal(void);
const char* orbit_camera_cardinal_str(void);
