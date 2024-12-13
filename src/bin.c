#include <string.h>

#include "bin.h"
#include "defines.h"
#include "util.h"

#define FILE_SIZE_MAX (131072)

void read_bytes(buffer_t* f, usize size, u8* out_bytes)
{
    ASSERT(size < FILE_SIZE_MAX, "File size too large");
    for (usize i = 0; i < size; i++) {
        out_bytes[i] = read_u8(f);
    }
    return;
}

u8 read_u8(buffer_t* f)
{
    u8 value;
    memcpy(&value, &f->data[f->offset], sizeof(u8));
    f->offset += sizeof(u8);
    return value;
}

u16 read_u16(buffer_t* f)
{
    u16 value;
    memcpy(&value, &f->data[f->offset], sizeof(u16));
    f->offset += sizeof(u16);
    return value;
}

u32 read_u32(buffer_t* f)
{
    u32 value;
    memcpy(&value, &f->data[f->offset], sizeof(u32));
    f->offset += sizeof(u32);
    return value;
}

i8 read_i8(buffer_t* f)
{
    i8 value;
    memcpy(&value, &f->data[f->offset], sizeof(i8));
    f->offset += sizeof(i8);
    return value;
}

i16 read_i16(buffer_t* f)
{
    i16 value;
    memcpy(&value, &f->data[f->offset], sizeof(i16));
    f->offset += sizeof(i16);
    return value;
}

i32 read_i32(buffer_t* f)
{
    i32 value;
    memcpy(&value, &f->data[f->offset], sizeof(i32));
    f->offset += sizeof(i32);
    return value;
}

f32 read_f1x3x12(buffer_t* f)
{
    f32 value = read_i16(f);
    return value / 4096.0f;
}
