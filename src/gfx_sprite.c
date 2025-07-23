// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include <string.h>

#include "cglm/struct/affine-pre.h"
#include "cglm/struct/affine.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/mat4.h"

#include "sokol_gfx.h"

#include "shader.glsl.h"

#include "camera.h"
#include "defines.h"
#include "filesystem.h"
#include "gfx.h"
#include "gfx_sprite.h"
#include "image.h"
#include "memory.h"
#include "mesh.h"
#include "span.h"
#include "texture.h"
#include "util.h"

static struct {
    // There is some wasted space because space on the palette_idx and cache
    // because only certain files have sprites, but this is easier to manage and
    // reason about. they are only u8 and i32 respectively.

    // current_palette_idx is the current palette index for each file.
    u8 current_palette_idx[F_FILE_COUNT];

    // cache the gpu texture for each file, they will be requested per frame.
    texture_t cache[F_FILE_COUNT];

    sprite_t sprites[100];

    sg_pipeline pipeline_3d;
    sg_pipeline pipeline_2d;
    sg_bindings bindings;
} _state;

// Getters
sprite_t* gfx_sprite_get_internals(void) { return _state.sprites; }

void gfx_sprite_init(void) {
    // Initialize the palette index to -1 to make them currently invalid
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        _state.current_palette_idx[i] = UINT8_MAX;
    }

    _state.bindings = (sg_bindings) {
        .vertex_buffers[0] = gfx_get_quad_vbuf(),
        .samplers[SMP_u_sampler] = gfx_get_sampler(),
    };

    _state.pipeline_3d = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_sprite_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_sprite_a_uv].offset = offsetof(vertex_t, uv),
                [ATTR_sprite_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
            },
        },
        .shader = sg_make_shader(sprite_shader_desc(sg_query_backend())),
        .face_winding = gfx_get_face_winding(),
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_GREATER,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .op_rgb = SG_BLENDOP_ADD,
        },
        .label = "sprite-pipeline-3d",
    });

    // The only difference is the depth comparisons. These 2d sprites should
    // always be on top.
    _state.pipeline_2d = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_sprite_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_sprite_a_uv].offset = offsetof(vertex_t, uv),
                [ATTR_sprite_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
            },
        },
        .shader = sg_make_shader(sprite_shader_desc(sg_query_backend())),
        .face_winding = gfx_get_face_winding(),
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            // Disable write and compare so thebg doesn't affect the depth buffer.
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_ALWAYS,
            .write_enabled = false,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .op_rgb = SG_BLENDOP_ADD,
        },
        .label = "sprite-pipeline",
    });
}

void gfx_sprite_shutdown(void) {
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        if (texture_valid(_state.cache[i])) {
            texture_destroy(_state.cache[i]);
        }
    }
}

void gfx_sprite_reset(void) {
    for (usize i = 0; i < 100; i++) {
        _state.sprites[i] = (sprite_t) {0};
    }
}

sprite_t gfx_sprite_create(sprite_type_e type, texture_t texture, vec2s min, vec2s size, transform_t transform) {
    vec2s uv_min, uv_max;

    if (type == SPRITE_2D) {
        // 2D sprites use flipped Y coordinates
        uv_min = (vec2s) {
            .x = min.x / texture.width,
            .y = (min.y + size.y) / texture.height,
        };
        uv_max = (vec2s) {
            .x = (min.x + size.x) / texture.width,
            .y = (min.y) / texture.height,
        };
    } else {
        // 3D sprites use normal Y coordinates
        uv_min = (vec2s) {
            .x = min.x / texture.width,
            .y = min.y / texture.height,
        };
        uv_max = (vec2s) {
            .x = (min.x + size.x) / texture.width,
            .y = (min.y + size.y) / texture.height,
        };
    }

    sprite_t sprite = {
        .type = type,
        .texture = texture,
        .uv_min = uv_min,
        .uv_max = uv_max,
        .transform = transform,
    };
    return sprite;
}

void _sprite2d_render(const sprite_t* sprite) {
    mat4s ortho_proj = glms_ortho(0.0f, GFX_RENDER_WIDTH, 0.0f, GFX_RENDER_HEIGHT, -1.0f, 1.0f);

    // Model matrix for screen position
    mat4s model_mat = glms_mat4_identity();
    model_mat = glms_translate(model_mat, (vec3s) { { sprite->transform.translation.x, sprite->transform.translation.y, 0.0f } });
    model_mat = glms_scale(model_mat, sprite->transform.scale);

    // Set up uniforms
    vs_sprite_params_t vs_params = {
        .u_proj = ortho_proj,
        .u_view = glms_mat4_identity(), // No view transformation for UI
        .u_model = model_mat,
        .u_uv_min = sprite->uv_min,
        .u_uv_max = sprite->uv_max,
    };

    // Bindings for this sprite
    sg_bindings bindings = _state.bindings;
    bindings.images[IMG_u_texture] = sprite->texture.gpu_image;

    // Render the sprite
    sg_apply_pipeline(_state.pipeline_2d);
    sg_apply_bindings(&bindings);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 6, 1);
}

void _sprite3d_render(const sprite_t* sprite) {
    // Get the sprite's model matrix (translation and scale)
    mat4s model_mat = transform_to_matrix(sprite->transform);

    // Retrieve the camera's view matrix
    mat4s view_mat = camera_get_view();

    // Extract and transpose the rotational part of the view matrix
    mat4s rotation_only = glms_mat4_identity();
    rotation_only.col[0] = (vec4s) { { view_mat.col[0].x, view_mat.col[0].y, view_mat.col[0].z, 0.0f } };
    rotation_only.col[1] = (vec4s) { { view_mat.col[1].x, view_mat.col[1].y, view_mat.col[1].z, 0.0f } };
    rotation_only.col[2] = (vec4s) { { view_mat.col[2].x, view_mat.col[2].y, view_mat.col[2].z, 0.0f } };

    mat4s inv_view_rot = glms_mat4_transpose(rotation_only);

    // Build the billboard matrix: rotation + translation + scale
    mat4s billboard_mat = inv_view_rot;

    // Apply scale
    vec3s scale = sprite->transform.scale;
    billboard_mat.col[0] = glms_vec4_scale(billboard_mat.col[0], scale.x);
    billboard_mat.col[1] = glms_vec4_scale(billboard_mat.col[1], scale.y);
    billboard_mat.col[2] = glms_vec4_scale(billboard_mat.col[2], scale.z);

    // Preserve translation from the model matrix
    billboard_mat.col[3] = model_mat.col[3];

    vs_sprite_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = view_mat,
        .u_model = billboard_mat,
        .u_uv_min = sprite->uv_min,
        .u_uv_max = sprite->uv_max,
    };

    ASSERT(texture_valid(sprite->texture), "Invalid sprite texture");

    _state.bindings.images[IMG_u_texture] = sprite->texture.gpu_image;

    sg_apply_pipeline(_state.pipeline_3d);
    sg_apply_bindings(&_state.bindings);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_draw(0, 6, 1);
}

texture_t sprite_get_paletted_texture(file_entry_e entry, int palette_idx) {
    if (_state.current_palette_idx[entry] == palette_idx) {
        return _state.cache[entry];
    } else {
        _state.current_palette_idx[entry] = palette_idx;
    }

    if (texture_valid(_state.cache[entry])) {
        texture_destroy(_state.cache[entry]);
    }

    span_t span = filesystem_read_file(entry);
    image_desc_t desc = image_get_desc(entry);
    image_t image = image_read_using_palette(&span, desc, palette_idx);

    texture_t texture = texture_create(image);
    _state.cache[entry] = texture;

    image_destroy(image);

    return _state.cache[entry];
}

void gfx_sprite_render(void) {
    for (int i = 0; i < 100; i++) {
        if (texture_valid(_state.sprites[i].texture)) {
            if (_state.sprites[i].type == SPRITE_2D) {
                _sprite2d_render(&_state.sprites[i]);
            } else if (_state.sprites[i].type == SPRITE_3D) {
                _sprite3d_render(&_state.sprites[i]);
            }
        }
    }
}
