#include "cglm/struct/cam.h"
#include "cglm/struct/vec3.h"
#include "cglm/types-struct.h"

#include "sokol_app.h"

#include "camera.h"

#define MAX_PHI     (glm_rad(89.9f))
#define SENSITIVITY (2.0f)

static camera_t _state;

static vec3s _to_cartesian(spherical_t);
static spherical_t _to_spherical(vec3s);

void camera_init(void) {
    _state.target = glms_vec3_zero();
    _state.position = _to_cartesian((spherical_t) {
        .radius = 256.0f,
        .theta = glm_rad(135.0f), // SW
        .phi = glm_rad(30.0f),
    });
}

void camera_update_transform(motion_t m) {
    spherical_t sph = _to_spherical(_state.position);

    sph.theta += glm_rad(m.oribit.x);
    sph.phi += glm_rad(m.oribit.y);
    sph.phi = glm_clamp(sph.phi, -MAX_PHI, MAX_PHI);

    sph.radius -= m.dolly;
    sph.radius = glm_clamp(sph.radius, CAMERA_DIST_MIN, CAMERA_DIST_MAX);

    _state.position = _to_cartesian((spherical_t) {
        .radius = sph.radius,
        .phi = sph.phi,
        .theta = sph.theta,
    });
}

spherical_t camera_get_spherical(void) {
    return _to_spherical(_state.position);
}

mat4s camera_get_view(void) {
    return glms_lookat(_state.position, _state.target, GLMS_YUP);
}

mat4s camera_get_proj(void) {
    f32 aspect = sapp_widthf() / sapp_heightf();

    if (_state.use_perspective) {
        return glms_perspective(glm_rad(60.0f), aspect, 1.0f, 2000.0f);
    } else {
        spherical_t s = _to_spherical(_state.position);
        f32 w = s.radius;
        f32 h = w / aspect;
        return glms_ortho(-w, w, -h, h, 0.01f, 2000.0f);
    }
}

static vec3s _to_cartesian(spherical_t s) {
    vec3s pos = { 0 };
    pos.x = s.radius * cosf(s.phi) * sinf(s.theta);
    pos.y = s.radius * sinf(s.phi);
    pos.z = s.radius * -cosf(s.phi) * cosf(s.theta);
    return pos;
}

static spherical_t _to_spherical(vec3s pos) {
    spherical_t s = { 0 };
    s.radius = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    s.theta = atan2f(pos.x, -pos.z);
    s.phi = asinf(pos.y / s.radius);
    return s;
}

camera_t* camera_get_internals(void) {
    return &_state;
}
