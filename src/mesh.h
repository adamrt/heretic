#pragma once

#include "cglm/types-struct.h"

#include "defines.h"
#include "image.h"
#include "lighting.h"
#include "span.h"
#include "terrain.h"

constexpr int MESH_MAX_TEX_TRIS = 512;
constexpr int MESH_MAX_TEX_QUADS = 768;
constexpr int MESH_MAX_UNTEX_TRIS = 64;
constexpr int MESH_MAX_UNTEX_QUADS = 256;
constexpr int MESH_MAX_VERTICES = 7620;

typedef struct {
    vec3s position;
    vec3s normal;
    vec2s uv;
    f32 palette_index;
    f32 is_textured;
} vertex_t;

typedef struct {
    vertex_t vertices[MESH_MAX_VERTICES];
    int count;
} vertices_t;

typedef struct {
    vertex_t vertices[3];
    u8 terrain_x;
    u8 terrain_z;
    u8 elevation;
} triangle_t;

typedef struct {
    vertex_t vertices[4];
    u8 terrain_x;
    u8 terrain_z;
    u8 elevation;
} quad_t;

typedef struct {
    triangle_t tex_tris[MESH_MAX_TEX_TRIS];
    int tex_tri_count;

    quad_t tex_quads[MESH_MAX_TEX_QUADS];
    int tex_quad_count;

    triangle_t untex_tris[MESH_MAX_UNTEX_TRIS];
    int untex_tri_count;

    quad_t untex_quads[MESH_MAX_UNTEX_QUADS];
    int untex_quad_count;

    int vertex_count;

    bool valid;
} geometry_t;

typedef struct {
    map_state_t map_state;

    geometry_t geometry;
    image_t palette;
    lighting_t lighting;
    terrain_t terrain;

    bool valid;
} mesh_t;

vec3s read_position(span_t*);
mesh_t read_mesh(span_t*);
void merge_meshes(mesh_t*, const mesh_t*);
vec3s vertices_centered(const vertices_t*);
vertices_t geometry_to_vertices(const geometry_t*);
