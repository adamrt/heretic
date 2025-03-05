
#include "model.h"

void model_destroy(model_t model) {
    sg_destroy_buffer(model.vbuf);
    sg_destroy_buffer(model.ibuf);

    texture_destroy(model.texture);
    texture_destroy(model.palette);
}
