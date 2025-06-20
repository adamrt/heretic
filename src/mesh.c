#include <float.h>

#include "cglm/types-struct.h"
#include "cglm/util.h"

#include "defines.h"
#include "image.h"
#include "lighting.h"
#include "mesh.h"
#include "terrain.h"
#include "util.h"

static geometry_t _read_geometry(span_t*);
static image_t _read_palette(span_t*);
static vec3s _read_normal(span_t*);
static vec2s _process_tex_coords(f32 u, f32 v, u8 page);

mesh_t read_mesh(span_t* span) {
    mesh_t mesh = {};

    mesh.geometry = _read_geometry(span);
    mesh.palette = _read_palette(span);
    mesh.lighting = read_lighting(span);
    mesh.terrain = read_terrain(span);

    bool is_valid = mesh.geometry.valid || mesh.palette.valid || mesh.lighting.valid;
    mesh.valid = is_valid;

    return mesh;
}

static geometry_t _read_geometry(span_t* span) {
    geometry_t geometry = {};

    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    span->offset = span_readat_u32(span, 0x40);
    if (span->offset == 0) {
        return geometry;
    }

    // The number of each type of polygon.
    u16 N = span_read_u16(span); // Textured triangles
    u16 P = span_read_u16(span); // Textured quads
    u16 Q = span_read_u16(span); // Untextured triangles
    u16 R = span_read_u16(span); // Untextured quads

    // Validate maximum values
    ASSERT(N < MESH_MAX_TEX_TRIS && P < MESH_MAX_TEX_QUADS && Q < MESH_MAX_UNTEX_TRIS && R < MESH_MAX_TEX_QUADS, "Mesh polygon count exceeded");

    geometry.tex_tri_count = N;
    geometry.tex_quad_count = P;
    geometry.untex_tri_count = Q;
    geometry.untex_quad_count = R;
    geometry.vertex_count = N * 3 + (P * 6) + Q * 3 + (R * 6);

    // Textured triangle
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 3; j++) {
            geometry.tex_tris[i].vertices[j].position = read_position(span);
        }
    }

    // Textured quads
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < 4; j++) {
            geometry.tex_quads[i].vertices[j].position = read_position(span);
        }
    }

    // Untextured triangle
    for (int i = 0; i < Q; i++) {
        for (int j = 0; j < 3; j++) {
            geometry.untex_tris[i].vertices[j].position = read_position(span);
        }
    }

    // Untextured quads
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < 4; j++) {
            geometry.untex_quads[i].vertices[j].position = read_position(span);
        }
    }

    // Triangle normals
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 3; j++) {
            geometry.tex_tris[i].vertices[j].normal = _read_normal(span);
        }
    };

    // Quad normals
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < 4; j++) {
            geometry.tex_quads[i].vertices[j].normal = _read_normal(span);
        }
    };

    // Triangle UV
    for (int i = 0; i < N; i++) {
        f32 au = span_read_u8(span);
        f32 av = span_read_u8(span);
        f32 palette = span_read_u8(span);
        (void)span_read_u8(span); // padding
        f32 bu = span_read_u8(span);
        f32 bv = span_read_u8(span);
        // FIXME: Page's byte has two other important bits for the texture image to use.
        f32 page = (span_read_u8(span) & 0x03); // 0b00000011
        (void)span_read_u8(span);               // padding
        f32 cu = span_read_u8(span);
        f32 cv = span_read_u8(span);

        vec2s a = _process_tex_coords(au, av, page);
        vec2s b = _process_tex_coords(bu, bv, page);
        vec2s c = _process_tex_coords(cu, cv, page);

        geometry.tex_tris[i].vertices[0].uv = a;
        geometry.tex_tris[i].vertices[0].palette_index = palette;
        geometry.tex_tris[i].vertices[0].is_textured = 1.0f;
        geometry.tex_tris[i].vertices[1].uv = b;
        geometry.tex_tris[i].vertices[1].palette_index = palette;
        geometry.tex_tris[i].vertices[1].is_textured = 1.0f;
        geometry.tex_tris[i].vertices[2].uv = c;
        geometry.tex_tris[i].vertices[2].palette_index = palette;
        geometry.tex_tris[i].vertices[2].is_textured = 1.0f;
    }

    // Quad UV
    for (int i = 0; i < P; i++) {
        f32 au = span_read_u8(span);
        f32 av = span_read_u8(span);
        f32 palette = span_read_u8(span);
        (void)span_read_u8(span); // padding
        f32 bu = span_read_u8(span);
        f32 bv = span_read_u8(span);
        // FIXME: Page's byte has two other important bits for the texture image to use.
        f32 page = (span_read_u8(span) & 0x03); // 0b00000011
        (void)span_read_u8(span);               // padding
        f32 cu = span_read_u8(span);
        f32 cv = span_read_u8(span);
        f32 du = span_read_u8(span);
        f32 dv = span_read_u8(span);

        vec2s a = _process_tex_coords(au, av, page);
        vec2s b = _process_tex_coords(bu, bv, page);
        vec2s c = _process_tex_coords(cu, cv, page);
        vec2s d = _process_tex_coords(du, dv, page);

        geometry.tex_quads[i].vertices[0].uv = a;
        geometry.tex_quads[i].vertices[0].palette_index = palette;
        geometry.tex_quads[i].vertices[0].is_textured = 1.0f;
        geometry.tex_quads[i].vertices[1].uv = b;
        geometry.tex_quads[i].vertices[1].palette_index = palette;
        geometry.tex_quads[i].vertices[1].is_textured = 1.0f;
        geometry.tex_quads[i].vertices[2].uv = c;
        geometry.tex_quads[i].vertices[2].palette_index = palette;
        geometry.tex_quads[i].vertices[2].is_textured = 1.0f;
        geometry.tex_quads[i].vertices[3].uv = d;
        geometry.tex_quads[i].vertices[3].palette_index = palette;
        geometry.tex_quads[i].vertices[3].is_textured = 1.0f;
    }

    // Unknown Untextured Polygon Data (skip over it)
    span->offset += 4 * Q + 4 * R;

    // Polygon tile locations (length N * 2 + P * 2)
    for (int i = 0; i < N; i++) {
        u8 zy = span_read_u8(span);
        u8 z = (zy >> 1) & 0b11111110;
        u8 y = (zy >> 0) & 0b00000001;
        u8 x = span_read_u8(span);
        geometry.tex_tris[i].terrain_x = x;
        geometry.tex_tris[i].terrain_z = z;
        geometry.tex_tris[i].elevation = y;
    }

    for (int i = 0; i < P; i++) {
        u8 zy = span_read_u8(span);
        u8 z = (zy >> 1) & 0b11111110;
        u8 y = (zy >> 0) & 0b00000001;
        u8 x = span_read_u8(span);
        geometry.tex_quads[i].terrain_x = x;
        geometry.tex_quads[i].terrain_z = z;
        geometry.tex_quads[i].elevation = y;
    }

    geometry.valid = true;
    return geometry;
}

vec3s read_position(span_t* span) {
    f32 x = span_read_i16(span);
    f32 y = span_read_i16(span);
    f32 z = span_read_i16(span);

    return (vec3s) { { x, y, z } };
}

static vec3s _read_normal(span_t* span) {
    f32 x = span_read_f16(span);
    f32 y = span_read_f16(span);
    f32 z = span_read_f16(span);

    return (vec3s) { { x, y, z } };
}

void merge_meshes(mesh_t* dst, const mesh_t* src) {
    ASSERT(dst != NULL, "Destination mesh is NULL");
    ASSERT(src != NULL, "Source mesh is NULL");

    if (src->geometry.valid) {
        dst->geometry = src->geometry;
    }

    if (src->palette.valid) {
        dst->palette = src->palette;
    }

    if (src->lighting.valid) {
        for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {
            if (src->lighting.lights[i].valid) {
                for (int j = 0; j < LIGHTING_MAX_LIGHTS; j++) {
                    dst->lighting.lights[j] = src->lighting.lights[j];
                }
                break;
            }
        }

        dst->lighting.ambient_strength = src->lighting.ambient_strength;
        dst->lighting.ambient_color = src->lighting.ambient_color;
        dst->lighting.bg_top = src->lighting.bg_top;
        dst->lighting.bg_bottom = src->lighting.bg_bottom;
    }

    if (src->terrain.valid) {
        dst->terrain = src->terrain;
    }
}

vertices_t geometry_to_vertices(const geometry_t* geometry) {
    vertices_t vertices = {};

    int vcount = 0;

    for (int i = 0; i < geometry->tex_tri_count; i++) {
        for (int j = 0; j < 3; j++) {
            vertices.vertices[vcount++] = geometry->tex_tris[i].vertices[j];
        }
    }

    for (int i = 0; i < geometry->tex_quad_count; i++) {
        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[0];
        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[1];
        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[2];

        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[1];
        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[3];
        vertices.vertices[vcount++] = geometry->tex_quads[i].vertices[2];
    }

    for (int i = 0; i < geometry->untex_tri_count; i++) {
        vertices.vertices[vcount++] = geometry->untex_tris[i].vertices[0];
        vertices.vertices[vcount++] = geometry->untex_tris[i].vertices[1];
        vertices.vertices[vcount++] = geometry->untex_tris[i].vertices[2];
    }

    for (int i = 0; i < geometry->untex_quad_count; i++) {
        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[0];
        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[1];
        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[2];

        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[1];
        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[3];
        vertices.vertices[vcount++] = geometry->untex_quads[i].vertices[2];
    }

    vertices.count = vcount;

    return vertices;
}

// Returns the center of the vertices in the mesh.
vec3s vertices_center(const vertices_t* vertices) {
    f32 min_x = FLT_MAX, max_x = -FLT_MAX;
    f32 min_y = FLT_MAX, max_y = -FLT_MAX;
    f32 min_z = FLT_MAX, max_z = -FLT_MAX;

    for (int i = 0; i < vertices->count; i++) {
        const vertex_t v = vertices->vertices[i];
        min_x = glm_min(v.position.x, min_x);
        min_y = glm_min(v.position.y, min_y);
        min_z = glm_min(v.position.z, min_z);
        max_x = glm_max(v.position.x, max_x);
        max_y = glm_max(v.position.y, max_y);
        max_z = glm_max(v.position.z, max_z);
    }

    return (vec3s) { {
        (min_x + max_x) / 2.0f,
        (min_y + max_y) / 2.0f,
        (min_z + max_z) / 2.0f,
    } };
}

// 16 palettes of 16 colors of 4 bytes
// process_tex_coords has two functions:
//
// 1. Update the v coordinate to the specific page of the texture. FFT
//    Textures have 4 pages (256x1024) and the original V specifies
//    the pixel on one of the 4 pages. Multiply the page by the height
//    of a single page (256).
// 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
//    divide them by their max to get a 0.0-1.0 value.
static vec2s _process_tex_coords(f32 u, f32 v, u8 page) {
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return (vec2s) { { u, v } };
}

static image_t _read_palette(span_t* span) {
    u32 intra_file_ptr = span_readat_u32(span, 0x44);
    if (intra_file_ptr == 0) {
        return (image_t) {};
    }
    span->offset = intra_file_ptr;

    const int palette_rows = 16; // All map textures use 16
    return image_read_palette(span, palette_rows);
}
