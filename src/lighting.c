#include "cglm/cglm.h"

#include "lighting.h"
#include "mesh.h"

static float read_light_color(file_t*);
static vec4s read_rgb8(file_t*);

// read_light_color clamps the value between 0.0 and 1.0. These unclamped values
// are used to affect the lighting model but it isn't understood yet.
// https://ffhacktics.com/wiki/Maps/Mesh#Light_colors_and_positions.2C_background_gradient_colors
lighting_t read_lighting(file_t* f)
{
    lighting_t lighting = { 0 };

    f->offset = 0x64;
    uint32_t intra_file_ptr = read_u32(f);
    if (intra_file_ptr == 0) {
        return lighting;
    }

    f->offset = intra_file_ptr;

    vec4s a_color = { .w = 1.0f };
    vec4s b_color = { .w = 1.0f };
    vec4s c_color = { .w = 1.0f };

    a_color.x = read_light_color(f);
    b_color.x = read_light_color(f);
    c_color.x = read_light_color(f);
    a_color.y = read_light_color(f);
    b_color.y = read_light_color(f);
    c_color.y = read_light_color(f);
    a_color.z = read_light_color(f);
    b_color.z = read_light_color(f);
    c_color.z = read_light_color(f);

    bool a_valid = a_color.r + a_color.g + a_color.b > 0.0f;
    bool b_valid = b_color.r + b_color.g + b_color.b > 0.0f;
    bool c_valid = c_color.r + c_color.g + c_color.b > 0.0f;

    vec3s a_pos = read_position(f);
    vec3s b_pos = read_position(f);
    vec3s c_pos = read_position(f);

    lighting.lights[0] = (light_t) { .color = a_color, .direction = a_pos, .valid = a_valid };
    lighting.lights[1] = (light_t) { .color = b_color, .direction = b_pos, .valid = b_valid };
    lighting.lights[2] = (light_t) { .color = c_color, .direction = c_pos, .valid = c_valid };

    lighting.ambient_color = read_rgb8(f);
    lighting.ambient_strength = 2.0f;

    lighting.bg_top = read_rgb8(f);
    lighting.bg_bottom = read_rgb8(f);

    lighting.valid = true;
    return lighting;
}

static vec4s read_rgb8(file_t* f)
{
    vec4s color = { 0 };
    color.r = read_u8(f) / 255.0f;
    color.g = read_u8(f) / 255.0f;
    color.b = read_u8(f) / 255.0f;
    color.a = 1.0f;
    return color;
}

static float read_light_color(file_t* f)
{
    float val = read_f1x3x12(f);
    return glm_min(glm_max(0.0f, val), 1.0f);
}
