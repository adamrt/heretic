#pragma once

#include "mesh.h"

// Fullscreen quad vertices (CCW winding)
//
// clang-format off
vertex_t shape_quad_vertices[] = {
    // Triangle 1 (bottom-left → bottom-right → top-left)
    { .position = {{ -1.0f, -1.0f, 0.0f }}, .uv = {{ 0.0f, 1.0f }} }, // bottom-left
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left

    // Triangle 2 (top-left → bottom-right → top-right)
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{  1.0f,  1.0f, 0.0f }}, .uv = {{ 1.0f, 0.0f }} }, // top-right
};
// clang-format on
