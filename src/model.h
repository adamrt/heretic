#ifndef MODEL_H_
#define MODEL_H_

#include "cglm/struct.h"
#include "sokol_gfx.h"

// GLOBAL_SCALE is used to scale the vertex data. This might be used later to
// scale camera movement and other positional related instruction data.
//
// We could use the original i16 data and pass those to OpenGL and let it
// normalize, but OpenGL will do it based on the full range of i16, not the max
// value of our vertex data. So we just pic a number here that is reasonable and
// use it everywhere.
#define GLOBAL_SCALE (256.0f)

#define GEOMETRY_MAX_VERTICES (5120)

#define TEXTURE_WIDTH (256)
#define TEXTURE_HEIGHT (1024)
#define TEXTURE_SIZE (TEXTURE_WIDTH * TEXTURE_HEIGHT) // 262144
#define TEXTURE_BYTE_SIZE (TEXTURE_SIZE * 4)

#define PALETTE_WIDTH (256)
#define PALETTE_HEIGHT (1)
#define PALETTE_SIZE (PALETTE_WIDTH * PALETTE_HEIGHT)
#define PALETTE_BYTE_SIZE (PALETTE_SIZE * 4)

typedef struct {
    vec3s position;
    vec3s normal;
    vec2s uv;
    float palette_index;
} vertex_t;

typedef struct {
    vertex_t vertices[GEOMETRY_MAX_VERTICES];
    int count;
} geometry_t;

typedef struct {
    uint8_t data[TEXTURE_BYTE_SIZE];
} texture_t;

typedef struct {
    uint8_t data[PALETTE_BYTE_SIZE];
} palette_t;

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

typedef struct {
    geometry_t geometry;
    palette_t palette;
} mesh_t;

typedef struct {
    mesh_t mesh;
    texture_t texture;

    transform_t transform;
    mat4s model_matrix;         // computed from transform
    vec3s centered_translation; // computed from geometry

    sg_bindings bindings;
} model_t;

vec3s geometry_centered_translation(geometry_t* geometry);
vec3s geometry_normalized_scale(geometry_t* geometry);

#endif // MODEL_H_
