#include <string.h>

#include "bin.h"
#include "defines.h"
#include "util.h"

#define FILE_SIZE_MAX (131072)

// FN_READ is a macro that generates a read function for a specific type. It
// reads the value, returns it and increments the offset.
#define FN_READ(type)                                      \
    type read_##type(buffer_t* f)                          \
    {                                                      \
        type value;                                        \
        memcpy(&value, &f->data[f->offset], sizeof(type)); \
        f->offset += sizeof(type);                         \
        return value;                                      \
    }

FN_READ(u8)
FN_READ(u16)
FN_READ(u32)
FN_READ(i8)
FN_READ(i16)
FN_READ(i32)

// FN_READ_AT is a macro that generates a read function for a specific type at a
// specified offset. It reads the value, returns it and sets the offset to the
// givin offset plus the size of the type.
#define FN_READ_AT(type)                                \
    type read_##type##_at(buffer_t* f, usize offset)    \
    {                                                   \
        type value;                                     \
        memcpy(&value, &f->data[offset], sizeof(type)); \
        f->offset = offset + sizeof(type);              \
        return value;                                   \
    }

FN_READ_AT(u8)
FN_READ_AT(u16)
FN_READ_AT(u32)
FN_READ_AT(i8)
FN_READ_AT(i16)
FN_READ_AT(i32)

void read_bytes(buffer_t* f, usize size, u8* out_bytes)
{
    ASSERT(size < FILE_SIZE_MAX, "File size too large");
    memcpy(out_bytes, &f->data[f->offset], size);
    f->offset += size;
    return;
}

f32 read_f1x3x12(buffer_t* f)
{
    f32 value = read_i16(f);
    return value / 4096.0f;
}
