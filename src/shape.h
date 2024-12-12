#ifndef SHAPE_H_
#define SHAPE_H_

#include <stdint.h>

#include "defines.h"
#include "mesh.h"

// Fullscreen quad vertices.
//
// These are are used for the background gradiant quad and the display quad.
// The background grandiant shader applies Z=1.0f to all vertices to push the quad
// behind the scene.

// clang-format off
vertex_t shape_quad_vertices[] = {
    { .position = {{ -1.0f, -1.0f, 0.0f }}, .uv = {{ 0.0f, 1.0f }} }, // bottom-left
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left
    { .position = {{  1.0f,  1.0f, 0.0f }}, .uv = {{ 1.0f, 0.0f }} }  // top-right
};

u16 shape_quad_indices[] = { 0, 1, 2, 1, 3, 2 };
// clang-format on

#endif // SHAPE_H_
