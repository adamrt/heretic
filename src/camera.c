#include "cglm/struct/cam.h"
#include "cglm/struct/vec3.h"
#include "cglm/types-struct.h"

#include "gfx.h"
#include "sokol_app.h"

#include "camera.h"

constexpr f32 MAX_PHI = 89.9f * GLM_PIf / 180.0f; // glm_rad isn't constexpr, so we do it ourselves
constexpr f32 DEFAULT_DISTANCE = 256.0f * 2.0f;
constexpr f32 ZOOM_SENSITIVITY = 0.002f;

static camera_t _state;

static vec3s _to_cartesian(spherical_t);
static spherical_t _to_spherical(vec3s);

void camera_init(void) {
    camera_reset();
}

void camera_reset(void) {
    _state.zoom = 1.0f;
    camera_set_orbit(glms_vec3_zero(), glm_rad(135.0f), glm_rad(30.0f), DEFAULT_DISTANCE);
}

void camera_freefly_motion(freefly_motion_t m) {
    f32 yaw_rad = _state.yaw_rad + glm_rad(m.yaw_deg);
    f32 pitch_rad = glm_clamp((_state.pitch_rad + glm_rad(m.pitch_deg)), -MAX_PHI, MAX_PHI);

    vec3s forward = {};
    forward.x = cosf(pitch_rad) * sinf(yaw_rad);
    forward.y = sinf(pitch_rad);
    forward.z = -cosf(pitch_rad) * cosf(yaw_rad);

    vec3s right = glms_normalize(glms_vec3_cross(forward, GLMS_YUP));
    vec3s up = glms_vec3_cross(forward, right);

    vec3s velocity = glms_vec3_scale(forward, m.forward);
    velocity = glms_vec3_add(velocity, glms_vec3_scale(right, m.right));
    velocity = glms_vec3_add(velocity, glms_vec3_scale(up, m.up));

    vec3s position = glms_vec3_add(_state.position, velocity);

    camera_set_freefly(position, yaw_rad, pitch_rad, _state.zoom);
}

void camera_orbit_motion(orbit_motion_t m) {
    spherical_t sph = _to_spherical(_state.position);

    f32 theta_rad = sph.theta_rad + glm_rad(m.theta_deg);
    f32 phi_rad = glm_clamp((sph.phi_rad + glm_rad(m.phi_deg)), -MAX_PHI, MAX_PHI);

    // Dolly / Zoom
    if (_state.use_perspective) {
        // Physically move camera
        sph.radius = glm_clamp(sph.radius - m.dolly, CAMERA_DIST_MIN, CAMERA_DIST_MAX);
    } else {
        // Change ortho frustum
        m.dolly *= ZOOM_SENSITIVITY;
        _state.zoom = glm_clamp(_state.zoom + m.dolly, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);
    }

    camera_set_orbit(glms_vec3_zero(), theta_rad, phi_rad, sph.radius);
}

void camera_set_freefly(vec3s position, f32 yaw, f32 pitch, f32 zoom) {
    _state.position = position;
    _state.yaw_rad = yaw;
    _state.pitch_rad = pitch;
    _state.zoom = zoom;
}

void camera_set_orbit(vec3s center, f32 theta, f32 phi, f32 distance) {
    _state.position.x = center.x + distance * cosf(phi) * sinf(theta);
    _state.position.y = center.y + distance * sinf(phi);
    _state.position.z = center.z + distance * -cosf(phi) * cosf(theta);

    vec3s forward = glms_vec3_sub(center, _state.position);
    forward = glms_vec3_normalize(forward);

    _state.yaw_rad = atan2f(forward.x, -forward.z);
    _state.pitch_rad = asinf(forward.y);
}

spherical_t camera_get_spherical(void) {
    return _to_spherical(_state.position);
}

mat4s camera_get_view(void) {
    vec3s forward = { {
        cosf(_state.pitch_rad) * sinf(_state.yaw_rad),
        sinf(_state.pitch_rad),
        -cosf(_state.pitch_rad) * cosf(_state.yaw_rad),
    } };
    vec3s center = glms_vec3_add(_state.position, forward);
    return glms_lookat(_state.position, center, GLMS_YUP);
}

mat4s camera_get_proj(void) {
    f32 aspect = GFX_RENDER_WIDTH / (f32)GFX_RENDER_HEIGHT;

    if (_state.use_perspective) {
        return glms_perspective(glm_rad(60.0f), aspect, 1.0f, 2000.0f);
    } else {
        // TODO: Why are we dividing by 2? it should not be
        f32 w = GFX_RENDER_WIDTH * _state.zoom / 2.0f;
        f32 h = GFX_RENDER_HEIGHT * _state.zoom / 2.0f;
        return glms_ortho(0, w, h, 0, -2048.0f, 2048.0f);
    }
}

static vec3s _to_cartesian(spherical_t s) {
    vec3s pos = {};
    pos.x = s.radius * cosf(s.phi_rad) * sinf(s.theta_rad);
    pos.y = s.radius * sinf(s.phi_rad);
    pos.z = s.radius * -cosf(s.phi_rad) * cosf(s.theta_rad);
    return pos;
}

static spherical_t _to_spherical(vec3s pos) {
    spherical_t s = {};
    s.radius = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    s.theta_rad = atan2f(pos.x, -pos.z);
    s.phi_rad = asinf(pos.y / s.radius);
    return s;
}

camera_t* camera_get_internals(void) {
    return &_state;
}
