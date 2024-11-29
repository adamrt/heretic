#include <float.h>

#include "cglm/types-struct.h"
#include "cglm/util.h"

#include "lighting.h"
#include "mesh.h"
#include "texture.h"
#include "util.h"

static vec3s read_normal(file_t*);
static geometry_t read_geometry(file_t*);
static vec2s process_tex_coords(float u, float v, uint8_t page);

mesh_t read_mesh(file_t* f)
{
    mesh_t mesh = { 0 };

    mesh.geometry = read_geometry(f);
    mesh.palette = read_palette(f);
    mesh.lighting = read_lighting(f);

    bool is_valid = mesh.geometry.valid || mesh.palette.valid || mesh.lighting.valid;
    mesh.valid = is_valid;

    return mesh;
}

static geometry_t read_geometry(file_t* f)
{
    geometry_t geometry = { 0 };

    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    f->offset = 0x40;
    f->offset = read_u32(f);
    if (f->offset == 0) {
        return geometry;
    }

    // The number of each type of polygon.
    int N = read_u16(f); // Textured triangles
    int P = read_u16(f); // Textured quads
    int Q = read_u16(f); // Untextured triangles
    int R = read_u16(f); // Untextured quads

    // Validate maximum values
    ASSERT(N < MESH_MAX_TEX_TRIS && P < MESH_MAX_TEX_QUADS && Q < MESH_MAX_UNTEX_TRIS && R < MESH_MAX_TEX_QUADS, "Mesh polygon count exceeded");

    geometry.tex_tri_count = N;
    geometry.tex_quad_count = P;
    geometry.untex_tri_count = Q;
    geometry.untex_quad_count = R;
    geometry.vertex_count = N * 3 + (P * 6) + Q * 3 + (R * 6);

    // Textured triangle
    for (int i = 0; i < N; i++) {
        geometry.tex_tris[i].a.position = read_position(f);
        geometry.tex_tris[i].b.position = read_position(f);
        geometry.tex_tris[i].c.position = read_position(f);
    }

    // Textured quads
    for (int i = 0; i < P; i++) {
        geometry.tex_quads[i].a.position = read_position(f);
        geometry.tex_quads[i].b.position = read_position(f);
        geometry.tex_quads[i].c.position = read_position(f);
        geometry.tex_quads[i].d.position = read_position(f);
    }

    // Untextured triangle
    for (int i = 0; i < Q; i++) {
        geometry.untex_tris[i].a.position = read_position(f);
        geometry.untex_tris[i].b.position = read_position(f);
        geometry.untex_tris[i].c.position = read_position(f);
    }

    // Untextured quads
    for (int i = 0; i < R; i++) {
        geometry.untex_quads[i].a.position = read_position(f);
        geometry.untex_quads[i].b.position = read_position(f);
        geometry.untex_quads[i].c.position = read_position(f);
        geometry.untex_quads[i].d.position = read_position(f);
    }

    // Triangle normals
    for (int i = 0; i < N; i++) {
        geometry.tex_tris[i].a.normal = read_normal(f);
        geometry.tex_tris[i].b.normal = read_normal(f);
        geometry.tex_tris[i].c.normal = read_normal(f);
    };

    // Quad normals
    for (int i = 0; i < P; i++) {
        geometry.tex_quads[i].a.normal = read_normal(f);
        geometry.tex_quads[i].b.normal = read_normal(f);
        geometry.tex_quads[i].c.normal = read_normal(f);
        geometry.tex_quads[i].d.normal = read_normal(f);
    };

    // Triangle UV
    for (int i = 0; i < N; i++) {
        float au = read_u8(f);
        float av = read_u8(f);
        float palette = read_u8(f);
        (void)read_u8(f); // padding
        float bu = read_u8(f);
        float bv = read_u8(f);
        float page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f);                 // padding
        float cu = read_u8(f);
        float cv = read_u8(f);

        vec2s a = process_tex_coords(au, av, page);
        vec2s b = process_tex_coords(bu, bv, page);
        vec2s c = process_tex_coords(cu, cv, page);

        geometry.tex_tris[i].a.uv = a;
        geometry.tex_tris[i].a.palette_index = palette;
        geometry.tex_tris[i].a.is_textured = 1.0f;
        geometry.tex_tris[i].b.uv = b;
        geometry.tex_tris[i].b.palette_index = palette;
        geometry.tex_tris[i].b.is_textured = 1.0f;
        geometry.tex_tris[i].c.uv = c;
        geometry.tex_tris[i].c.palette_index = palette;
        geometry.tex_tris[i].c.is_textured = 1.0f;
    }

    // Quad UV
    for (int i = 0; i < P; i++) {
        float au = read_u8(f);
        float av = read_u8(f);
        float palette = read_u8(f);
        (void)read_u8(f); // padding
        float bu = read_u8(f);
        float bv = read_u8(f);
        float page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f);                 // padding
        float cu = read_u8(f);
        float cv = read_u8(f);
        float du = read_u8(f);
        float dv = read_u8(f);

        vec2s a = process_tex_coords(au, av, page);
        vec2s b = process_tex_coords(bu, bv, page);
        vec2s c = process_tex_coords(cu, cv, page);
        vec2s d = process_tex_coords(du, dv, page);

        geometry.tex_quads[i].a.uv = a;
        geometry.tex_quads[i].a.palette_index = palette;
        geometry.tex_quads[i].a.is_textured = 1.0f;
        geometry.tex_quads[i].b.uv = b;
        geometry.tex_quads[i].b.palette_index = palette;
        geometry.tex_quads[i].b.is_textured = 1.0f;
        geometry.tex_quads[i].c.uv = c;
        geometry.tex_quads[i].c.palette_index = palette;
        geometry.tex_quads[i].c.is_textured = 1.0f;
        geometry.tex_quads[i].d.uv = d;
        geometry.tex_quads[i].d.palette_index = palette;
        geometry.tex_quads[i].d.is_textured = 1.0f;
    }

    geometry.valid = true;
    return geometry;
}

vec3s read_position(file_t* f)
{
    float x = read_i16(f);
    float y = read_i16(f);
    float z = read_i16(f);

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

static vec3s read_normal(file_t* f)
{
    float x = read_f1x3x12(f);
    float y = read_f1x3x12(f);
    float z = read_f1x3x12(f);

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

void merge_meshes(mesh_t* dst, mesh_t* src)
{
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
}

vertices_t geometry_to_vertices(geometry_t* geometry)
{
    vertices_t vertices = { 0 };

    // 10 triangles = 30 offset
    int vcount = 0;

    for (int i = 0; i < geometry->tex_tri_count; i++) {
        vertices.vertices[vcount++] = geometry->tex_tris[i].a;
        vertices.vertices[vcount++] = geometry->tex_tris[i].b;
        vertices.vertices[vcount++] = geometry->tex_tris[i].c;
    }

    for (int i = 0; i < geometry->tex_quad_count; i++) {
        vertices.vertices[vcount++] = geometry->tex_quads[i].a;
        vertices.vertices[vcount++] = geometry->tex_quads[i].b;
        vertices.vertices[vcount++] = geometry->tex_quads[i].c;

        vertices.vertices[vcount++] = geometry->tex_quads[i].b;
        vertices.vertices[vcount++] = geometry->tex_quads[i].d;
        vertices.vertices[vcount++] = geometry->tex_quads[i].c;
    }

    for (int i = 0; i < geometry->untex_tri_count; i++) {
        vertices.vertices[vcount++] = geometry->untex_tris[i].a;
        vertices.vertices[vcount++] = geometry->untex_tris[i].b;
        vertices.vertices[vcount++] = geometry->untex_tris[i].c;
    }

    for (int i = 0; i < geometry->untex_quad_count; i++) {
        vertices.vertices[vcount++] = geometry->untex_quads[i].a;
        vertices.vertices[vcount++] = geometry->untex_quads[i].b;
        vertices.vertices[vcount++] = geometry->untex_quads[i].c;

        vertices.vertices[vcount++] = geometry->untex_quads[i].b;
        vertices.vertices[vcount++] = geometry->untex_quads[i].d;
        vertices.vertices[vcount++] = geometry->untex_quads[i].c;
    }

    vertices.count = vcount;

    return vertices;
}

vec3s vertices_centered(vertices_t* vertices)
{
    float min_x = FLT_MAX;
    float max_x = -FLT_MAX;
    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;

    for (int i = 0; i < vertices->count; i++) {
        const vertex_t vertex = vertices->vertices[i];

        min_x = glm_min(vertex.position.x, min_x);
        min_y = glm_min(vertex.position.y, min_y);
        min_z = glm_min(vertex.position.z, min_z);

        max_x = glm_max(vertex.position.x, max_x);
        max_y = glm_max(vertex.position.y, max_y);
        max_z = glm_max(vertex.position.z, max_z);
    }

    float x = -(max_x + min_x) / 2.0f;
    float y = -(max_y + min_y) / 2.0f;
    float z = -(max_z + min_z) / 2.0f;

    return (vec3s) { { x, y, z } };
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
static vec2s process_tex_coords(float u, float v, uint8_t page)
{
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return (vec2s) { { u, v } };
}
