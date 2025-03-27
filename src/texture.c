#include "texture.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_imgui.h"

texture_t texture_create(image_t image) {
    sg_image_desc desc = {};
    desc.width = image.width;
    desc.height = image.height;
    desc.data.subimage[0][0].size = image.size;
    desc.data.subimage[0][0].ptr = image.data;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    sg_image gpu_image = sg_make_image(&desc);

    texture_t texture = {};
    texture.width = image.width;
    texture.height = image.height;
    texture.gpu_image = gpu_image;
    return texture;
}

void texture_destroy(texture_t texture) {
    sg_destroy_image(texture.gpu_image);
}

bool texture_valid(texture_t texture) {
    return texture.gpu_image.id != SG_INVALID_ID;
}

u64 texture_imgui_id(texture_t texture) {
    return simgui_imtextureid(texture.gpu_image);
}
