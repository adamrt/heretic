#pragma once

#include <stdint.h>

#include "mesh.h"

// Fullscreen quad vertices.
//
// These are CW since thats what the PS1 meshes use.
//
// clang-format off
vertex_t shape_quad_vertices[] = {
    // Triangle 1
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{ -1.0f, -1.0f, 0.0f }}, .uv = {{ 0.0f, 1.0f }} }, // bottom-left
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left

    // Triangle 2
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left
    { .position = {{  1.0f,  1.0f, 0.0f }}, .uv = {{ 1.0f, 0.0f }} }, // top-right
};
// clang-format on
