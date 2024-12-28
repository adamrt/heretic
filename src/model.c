#include "model.h"

void model_destroy(model_t model) {
    sg_destroy_buffer(model.vbuf);
    sg_destroy_buffer(model.ibuf);
    sg_destroy_image(model.texture);
    sg_destroy_image(model.palette);
}
