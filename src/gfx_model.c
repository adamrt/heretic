#include "cglm/struct/vec4.h"

#include "camera.h"
#include "gfx.h"
#include "gfx_model.h"
#include "lighting.h"

#include "shader.glsl.h"
#include "util.h"

static struct {
    sg_pipeline pipeline;
    model_t model;
} _state;

void gfx_model_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_standard_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_standard_a_palette_index].format = SG_VERTEXFORMAT_FLOAT,
                [ATTR_standard_a_is_textured].format = SG_VERTEXFORMAT_FLOAT,
            },
        },
        .shader = sg_make_shader(standard_shader_desc(sg_query_backend())),
        .face_winding = gfx_get_face_winding(),
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "standard-pipeline",
    });
}

void gfx_model_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
}

model_t gfx_model_create(const map_t* map, map_state_t map_state) {
    mesh_t final_mesh = {};
    image_t final_texture = {};
    if (map->primary_mesh.valid) {
        final_mesh = map->primary_mesh;
    } else {
        final_mesh = map->override_mesh;
    }

    for (int i = 0; i < map->alt_mesh_count; i++) {
        mesh_t alt_mesh = map->alt_meshes[i];
        if (alt_mesh.valid && map_state_eq(alt_mesh.map_state, map_state)) {
            merge_meshes(&final_mesh, &alt_mesh);
            break;
        }
    }

    for (int i = 0; i < map->texture_count; i++) {
        image_t texture = map->textures[i];

        if (texture.valid && map_state_eq(texture.map_state, map_state)) {
            final_texture = texture;
            break;
        }
        if (texture.valid && map_state_default(texture.map_state)) {
            if (!final_texture.valid) {
                final_texture = texture;
            }
        }
    }

    ASSERT(final_mesh.valid, "Map mesh is invalid");
    ASSERT(final_texture.valid, "Map texture is invalid");

    vertices_t vertices = geometry_to_vertices(&final_mesh.geometry);

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(vertices),
        .label = "mesh-vertices",
    });

    texture_t texture = texture_create(final_texture);
    texture_t palette = texture_create(final_mesh.palette);

    vec3s centered_translation = vertices_centered(&vertices);

    model_t model = {
        .vertex_count = final_mesh.geometry.vertex_count,
        .lighting = final_mesh.lighting,
        .center = centered_translation,
        .transform.scale = { { 1.0f, 1.0f, 1.0f } },
        .vbuf = vbuf,
        .texture = texture,
        .palette = palette,
    };
    return model;
}

void gfx_model_destroy(void) {
    sg_destroy_buffer(_state.model.vbuf);
    sg_destroy_buffer(_state.model.ibuf);

    texture_destroy(_state.model.texture);
    texture_destroy(_state.model.palette);
}

void gfx_model_render(void) {
    mat4s model_mat = transform_to_matrix(_state.model.transform);

    vs_standard_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = camera_get_view(),
        .u_model = model_mat,
    };

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = _state.model.lighting.ambient_color;
    fs_params.u_ambient_strength = _state.model.lighting.ambient_strength;

    int light_count = 0;
    for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {

        light_t light = _state.model.lighting.lights[i];
        if (!light.valid) {
            continue;
        }

        fs_params.u_light_colors[light_count] = light.color;
        fs_params.u_light_directions[light_count] = glms_vec4(light.direction, 1.0f);
        light_count++;
    }
    fs_params.u_light_count = light_count;

    sg_bindings bindings = {
        .vertex_buffers[0] = _state.model.vbuf,
        .index_buffer = _state.model.ibuf,
        .samplers[SMP_u_sampler] = gfx_get_sampler(),
        .images = {
            [IMG_u_texture] = _state.model.texture.gpu_image,
            [IMG_u_palette] = _state.model.palette.gpu_image,
        },
    };

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&bindings);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_apply_uniforms(1, &SG_RANGE(fs_params));
    sg_draw(0, _state.model.vertex_count, 1);
}

// Getters
void gfx_model_set(model_t model) { _state.model = model; }
void gfx_model_set_y_rotation(f32 maprot) { _state.model.transform.rotation.y = maprot; }
transform3d_t* gfx_model_get_transform(void) { return &_state.model.transform; }
lighting_t* gfx_model_get_lighting(void) { return &_state.model.lighting; }
