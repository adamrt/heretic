#include <string.h>

#include "defines.h"
#include "span.h"
#include "util.h"

#define SPAN_MAX_BYTES (131072)

void span_read_bytes(span_t* f, usize size, u8* out_bytes) {
    ASSERT(size <= SPAN_MAX_BYTES, "Too many bytes requested.");
    memcpy(out_bytes, &f->data[f->offset], size);
    f->offset += size;
    return;
}

// This returns a f32, but stored as a fixed-point number.
// 1 bit   - sign bit
// 3 bits  - whole
// 12 bits - fraction
f32 span_read_f16(span_t* span) {
    f32 value = span_read_i16(span);
    return value / 4096.0f;
}

void span_print(const span_t* span) {
    printf("Span: \n");
    for (usize i = 0; i < span->size; i++) {
        printf("%02X ", span->data[i]);
        if ((i + 1) % 32 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

// FN_SPAN_READ is a macro that generates a read function for a specific type. It
// reads the value, returns it and increments the offset.
#define FN_SPAN_READ(type)                                                        \
    type span_read_##type(span_t* span) {                                         \
        ASSERT(span->offset + sizeof(type) <= span->size, "Out of bounds read."); \
        type value;                                                               \
        memcpy(&value, &span->data[span->offset], sizeof(type));                  \
        span->offset += sizeof(type);                                             \
        return value;                                                             \
    }

FN_SPAN_READ(u8)
FN_SPAN_READ(u16)
FN_SPAN_READ(u32)
FN_SPAN_READ(i8)
FN_SPAN_READ(i16)
FN_SPAN_READ(i32)

// FN_SPAN_READAT is very similar to FN_SPAN_READ, but accepts an offset to
// start the read at.
#define FN_SPAN_READAT(type)                                                \
    type span_readat_##type(span_t* span, usize offset) {                   \
        ASSERT(offset + sizeof(type) <= span->size, "Out of bounds read."); \
        type value;                                                         \
        memcpy(&value, &span->data[offset], sizeof(type));                  \
        span->offset = offset + sizeof(type);                               \
        return value;                                                       \
    }

FN_SPAN_READAT(u8)
FN_SPAN_READAT(u16)
FN_SPAN_READAT(u32)
FN_SPAN_READAT(i8)
FN_SPAN_READAT(i16)
FN_SPAN_READAT(i32)
