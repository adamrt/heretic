#include "cglm/struct/affine.h"

#include "transform.h"

mat4s transform_to_matrix(transform3d_t transform) {
    mat4s model_matrix = glms_mat4_identity();
    model_matrix = glms_translate(model_matrix, transform.translation);
    model_matrix = glms_rotate_x(model_matrix, transform.rotation.x);
    model_matrix = glms_rotate_y(model_matrix, transform.rotation.y);
    model_matrix = glms_rotate_z(model_matrix, transform.rotation.z);
    model_matrix = glms_scale(model_matrix, transform.scale);
    return model_matrix;
}

mat4s transform_to_matrix_around_center(transform3d_t transform, vec3s center) {
    mat4s model = glms_mat4_identity();

    // Step 1: move model so center is at origin
    model = glms_translate(model, glms_vec3_negate(center));

    // Step 2: apply rotation and scale
    model = glms_rotate_x(model, transform.rotation.x);
    model = glms_rotate_y(model, transform.rotation.y);
    model = glms_rotate_z(model, transform.rotation.z);
    model = glms_scale(model, transform.scale);

    // Step 3: move it back
    model = glms_translate(model, glms_vec3_add(center, transform.translation));

    return model;
}
