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

    // These are the same as above but they are for sprites that have multiple
    // rows, each with thier own palette. We must track the palette index for
    // each row and cache the texture for each row. Currently only used for
    // EVTFACE.BIN, but most likely will be expanded for other files when we
    // find them.
    u8 current_palette_idx_evtface[8][8];
    texture_t cache_evtface[8];

    sprite3d_t sprite3ds[100];
    sprite2d_t sprite2ds[100];

    sg_pipeline pipeline_3d;
    sg_pipeline pipeline_2d;
    sg_bindings bindings;
} _state;

typedef struct {
    int tex_width;
    int tex_height;
    int tex_offset;

    int pal_count;
    int pal_offset;
    int pal_default; // Not used yet
} paletted_image_4bpp_desc_t;

const paletted_image_4bpp_desc_t paletted_image_desc_list[] = {
    [F_EVENT__FRAME_BIN] = { 256, 288, 0, 22, 36864, 5 },
    [F_EVENT__ITEM_BIN] = { 256, 256, 0, 16, 32768, 0 },
    [F_EVENT__UNIT_BIN] = { 256, 480, 0, 128, 61440, 0 },
};

static image_t _read_paletted_sprite(span_t*, int, int, image_t, usize);
static image_t _read_paletted_image_4bpp(span_t*, paletted_image_4bpp_desc_t, int);

// Getters
sprite3d_t* gfx_sprite3d_get_internals(void) { return _state.sprite3ds; }
sprite2d_t* gfx_sprite2d_get_internals(void) { return _state.sprite2ds; }

void gfx_sprite_init(void) {
    // Initialize the palette index to -1 to make them currently invalid
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        _state.current_palette_idx[i] = UINT8_MAX;
    }

    for (usize row = 0; row < 8; row++) {
        for (usize col = 0; col < 8; col++) {
            _state.current_palette_idx_evtface[row][col] = UINT8_MAX;
        }
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
    for (usize row = 0; row < 8; row++) {
        if (texture_valid(_state.cache_evtface[row])) {
            texture_destroy(_state.cache_evtface[row]);
        }
    }
}

void gfx_sprite_reset(void) {
    for (usize i = 0; i < 100; i++) {
        _state.sprite2ds[i] = (sprite2d_t) {};
        _state.sprite3ds[i] = (sprite3d_t) {};
    }
}

sprite2d_t gfx_sprite2d_create(texture_t texture, vec2s min, vec2s size, f32 x, f32 y, f32 scale) {
    vec2s uv_min = (vec2s) {
        .x = min.x / texture.width,
        .y = (min.y + size.y) / texture.height,
    };
    vec2s uv_max = (vec2s) {
        .x = (min.x + size.x) / texture.width,
        .y = (min.y) / texture.height,
    };

    sprite2d_t sprite = {
        .texture = texture,
        .uv_min = uv_min,
        .uv_max = uv_max,
        .transform = {
            .scale = { { scale, scale, scale } },
            .screen_pos = { { x, y } },
        },
    };
    return sprite;
}

sprite3d_t gfx_sprite3d_create(texture_t texture, vec2s min, vec2s size, transform3d_t transform) {
    vec2s uv_min = (vec2s) { { min.x / texture.width, min.y / texture.height } };
    vec2s uv_max = (vec2s) { { (min.x + size.x) / texture.width, (min.y + size.y) / texture.height } };

    sprite3d_t sprite = {
        .texture = texture,
        .uv_min = uv_min,
        .uv_max = uv_max,
        .transform = transform,
    };
    return sprite;
}

void _sprite2d_render(const sprite2d_t* sprite) {
    mat4s ortho_proj = glms_ortho(0.0f, GFX_RENDER_WIDTH, 0.0f, GFX_RENDER_HEIGHT, -1.0f, 1.0f);

    // Model matrix for screen position
    mat4s model_mat = glms_mat4_identity();
    model_mat = glms_translate(model_mat, (vec3s) { { sprite->transform.screen_pos.x, sprite->transform.screen_pos.y, 0.0f } });
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

void _sprite3d_render(const sprite3d_t* sprite) {
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
    image_t image = _read_paletted_image_4bpp(&span, paletted_image_desc_list[entry], palette_idx);
    _state.cache[entry] = texture_create(image);
    image_destroy(image);

    return _state.cache[entry];
}

void gfx_sprite_render(void) {
    for (int i = 0; i < 100; i++) {
        if (texture_valid(_state.sprite3ds[i].texture)) {
            _sprite3d_render(&_state.sprite3ds[i]);
        }
        if (texture_valid(_state.sprite2ds[i].texture)) {
            _sprite2d_render(&_state.sprite2ds[i]);
        }
    }
}

//
// EVTFACE.BIN
//

// This reads a single row of portraits. Each row has it's own palette at the end.
// We read them as individual rows so we can apply the palette per row.
static image_t _read_image_row_evtface_bin(span_t* span, int row, int palette_idx) {
    constexpr int width = 256;
    constexpr int height = 48;
    constexpr int dims = width * height;
    constexpr int size = dims * 4;

    // Basic dimensions
    constexpr int cols = 8;
    constexpr int portrait_width = 32;
    constexpr int portrait_height = 48;
    constexpr int bytes_per_row = 8192;
    constexpr int bytes_per_portrait = 768;
    constexpr int palette_offset = 6144; // per row

    u8* data = memory_allocate(size);

    u32 pal_offset = row * bytes_per_row + palette_offset;
    span->offset = pal_offset;
    image_t palette = image_read_palette(span, 16);

    for (int col = 0; col < cols; col++) {
        int tex_offset = row * bytes_per_row + col * bytes_per_portrait;
        span->offset = tex_offset;
        image_t portrait_image = _read_paletted_sprite(span, portrait_width, portrait_height, palette, palette_idx);
        int dest_x = col * portrait_width;

        for (int y = 0; y < portrait_height; y++) {
            int src_index = y * portrait_width * 4;
            int dest_index = y * width * 4 + dest_x * 4;
            memcpy(&data[dest_index], &portrait_image.data[src_index], portrait_width * 4);
        }
        image_destroy(portrait_image);
    }

    image_destroy(palette);

    image_t row_image = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
    return row_image;
}

texture_t sprite_get_evtface_bin_texture(int row_idx, int palette_idx) {
    file_entry_e entry = F_EVENT__EVTFACE_BIN;
    if (_state.current_palette_idx_evtface[row_idx][palette_idx] == palette_idx) {
        return _state.cache_evtface[row_idx];
    } else {
        _state.current_palette_idx_evtface[row_idx][palette_idx] = palette_idx;
    }

    if (texture_valid(_state.cache_evtface[row_idx])) {
        texture_destroy(_state.cache_evtface[row_idx]);
    }

    span_t span = filesystem_read_file(entry);
    image_t image = _read_image_row_evtface_bin(&span, row_idx, palette_idx);
    _state.cache_evtface[row_idx] = texture_create(image);
    image_destroy(image);

    return _state.cache_evtface[row_idx];
}

//
// Shared functions
//

static image_t _read_paletted_sprite(span_t* span, int width, int height, image_t palette, usize palette_idx) {
    const int dims = width * height;

    image_t image = image_read_4bpp(span, width, height);
    usize palette_offset = (16 * 4 * palette_idx); // 16 colors * 4 bytes per color * item_index

    // image is an RGBA image with 4 bytes per pixel. Each color's RGBA are all
    // the same value (the palette index). So we loop over each 4th pixel and index
    // the palette to get the RGBA values, and copy the palettes RGBA values to the
    // original index image.
    for (int i = 0; i < dims * 4; i = i + 4) {
        u8 pixel = image.data[i];
        memcpy(&image.data[i], &palette.data[pixel * 4 + palette_offset], 4);
    }

    return image;
}

static image_t _read_paletted_image_4bpp(span_t* span, paletted_image_4bpp_desc_t desc, int pindex) {
    span->offset = desc.pal_offset;
    image_t palette = image_read_palette(span, desc.pal_count);

    span->offset = desc.tex_offset;
    image_t image = _read_paletted_sprite(span, desc.tex_width, desc.tex_height, palette, pindex);

    image_destroy(palette);

    return image;
}
