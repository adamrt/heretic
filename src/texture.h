#pragma once

#include "image.h"
#include "sokol_gfx.h"

// texture_t represents a GPU image/texture.
typedef struct {
    sg_image gpu_image;
    int width;
    int height;
} texture_t;

texture_t texture_create(image_t);
void texture_destroy(texture_t);
bool texture_valid(texture_t);
u64 texture_imgui_id(texture_t); // Maybe move to gui.h/c
