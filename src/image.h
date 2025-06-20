// image_t is a struct that represents an image in memory.
//
// All the image_read_*() functions heap allocate
// and image_destroy() should be called to free them.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "filesystem.h"
#include "map_record.h"
#include "span.h"

typedef struct {
    int width;
    int height;
    usize size;
    u8* data;
    bool valid;
} image_t;

typedef enum {
    IMG_4BPP,
    IMG_4BPP_PAL,
    IMG_16BPP,
} image_type_e;

typedef struct {
    char* name;
    file_entry_e entry;
    image_type_e type;

    int width;
    int height;

    int data_offset;
    int data_length;

    int pal_offset;
    int pal_length;
    int pal_count;
    int pal_default;
} image_desc_t;

extern const image_desc_t image_desc_list[4];
image_desc_t image_get_desc(file_entry_e);

void image_destroy(image_t);

image_t image_read(span_t* span, image_desc_t desc);
image_t image_read_using_palette(span_t* span, image_desc_t desc, int pal_index);

image_t image_read_palette(span_t*, int);
image_t image_read_4bpp(span_t*, int, int);
image_t image_read_4bpp_pal(span_t*, int, int, image_t, usize);
image_t image_read_16bpp(span_t*, int, int);
