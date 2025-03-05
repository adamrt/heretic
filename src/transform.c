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
