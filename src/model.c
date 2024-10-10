#include <float.h>

#include "model.h"

vec3s geometry_centered_translation(geometry_t* geometry)
{
    float min_x = FLT_MAX;
    float max_x = -FLT_MAX;
    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;

    for (int i = 0; i < geometry->count; i++) {
        const vertex_t vertex = geometry->vertices[i];

        min_x = MIN(vertex.position.x, min_x);
        min_y = MIN(vertex.position.y, min_y);
        min_z = MIN(vertex.position.z, min_z);

        max_x = MAX(vertex.position.x, max_x);
        max_y = MAX(vertex.position.y, max_y);
        max_z = MAX(vertex.position.z, max_z);
    }

    float x = -(max_x + min_x) / 2.0f;
    float y = -(max_y + min_y) / 2.0f;
    float z = -(max_z + min_z) / 2.0f;

    return (vec3s) { { x, y, z } };
}

vec3s geometry_normalized_scale(geometry_t* geometry)
{
    float min_x = FLT_MAX;
    float max_x = -FLT_MAX;
    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;

    // Iterate through all vertices to find min and max for each axis
    for (int i = 0; i < geometry->count; i++) {
        const vertex_t vertex = geometry->vertices[i];

        // Update min values
        min_x = MIN(min_x, vertex.position.x);
        min_y = MIN(min_y, vertex.position.y);
        min_z = MIN(min_z, vertex.position.z);

        // Update max values
        max_x = MAX(max_x, vertex.position.x);
        max_y = MAX(max_y, vertex.position.y);
        max_z = MAX(max_z, vertex.position.z);
    }

    // Compute the size along each axis
    float size_x = max_x - min_x;
    float size_y = max_y - min_y;
    float size_z = max_z - min_z;

    // Find the largest dimension
    float largest_dimension = MAX(MAX(size_x, size_y), size_z);

    // Prevent division by zero
    if (largest_dimension == 0.0f) {
        // Mesh has zero size; return zero scaling
        vec3s zero = { { 0.0f, 0.0f, 0.0f } };
        return zero;
    }

    // Compute the scaling factor
    float scaling_factor = 2.0f / largest_dimension;

    // Create the normalized scaling vector
    vec3s normalized_scale = { { scaling_factor, scaling_factor, scaling_factor } };

    return normalized_scale;
}
