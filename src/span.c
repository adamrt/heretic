#include <string.h>

#include "defines.h"
#include "span.h"
#include "util.h"

#define FILE_SIZE_MAX (131072)

// FN_SPAN_READ is a macro that generates a read function for a specific type. It
// reads the value, returns it and increments the offset.
#define FN_SPAN_READ(type)                                       \
    type span_read_##type(span_t* span) {                        \
        type value;                                              \
        memcpy(&value, &span->data[span->offset], sizeof(type)); \
        span->offset += sizeof(type);                            \
        return value;                                            \
    }

FN_SPAN_READ(u8)
FN_SPAN_READ(u16)
FN_SPAN_READ(u32)
FN_SPAN_READ(i8)
FN_SPAN_READ(i16)
FN_SPAN_READ(i32)

// FN_SPAN_READAT is a macro that generates a read function for a specific type at a
// specified offset. It reads the value, returns it and sets the offset to the
// givin offset plus the size of the type.
#define FN_SPAN_READAT(type)                               \
    type span_readat_##type(span_t* span, usize offset) {  \
        type value;                                        \
        memcpy(&value, &span->data[offset], sizeof(type)); \
        span->offset = offset + sizeof(type);              \
        return value;                                      \
    }

FN_SPAN_READAT(u8)
FN_SPAN_READAT(u16)
FN_SPAN_READAT(u32)
FN_SPAN_READAT(i8)
FN_SPAN_READAT(i16)
FN_SPAN_READAT(i32)

void span_read_bytes(span_t* f, usize size, u8* out_bytes) {
    ASSERT(size < FILE_SIZE_MAX, "File size too large");
    memcpy(out_bytes, &f->data[f->offset], size);
    f->offset += size;
    return;
}

f32 span_read_f1x3x12(span_t* span) {
    f32 value = span_read_i16(span);
    return value / 4096.0f;
}
