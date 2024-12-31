#include "cglm/struct/affine.h"

#include "model.h"

mat4s model_matrix(transform_t transform) {
    mat4s model_matrix = glms_mat4_identity();
    model_matrix = glms_translate(model_matrix, transform.translation);
    model_matrix = glms_rotate_x(model_matrix, transform.rotation.x);
    model_matrix = glms_rotate_y(model_matrix, transform.rotation.y);
    model_matrix = glms_rotate_z(model_matrix, transform.rotation.z);
    model_matrix = glms_scale(model_matrix, transform.scale);
    return model_matrix;
}

void model_destroy(model_t model) {
    sg_destroy_buffer(model.vbuf);
    sg_destroy_buffer(model.ibuf);

    texture_destroy(model.texture);
    texture_destroy(model.palette);
}
