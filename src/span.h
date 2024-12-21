#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "defines.h"

// Span/view struct to simplify reading of binary data.
typedef struct {
    const u8* data;
    usize size;
    usize offset;
} span_t;

void span_read_bytes(span_t*, usize, u8*);

u8 span_read_u8(span_t*);
u16 span_read_u16(span_t*);
u32 span_read_u32(span_t*);
i8 span_read_i8(span_t*);
i16 span_read_i16(span_t*);
i32 span_read_i32(span_t*);
f32 span_read_f16(span_t*);

u8 span_readat_u8(span_t*, usize);
u16 span_readat_u16(span_t*, usize);
u32 span_readat_u32(span_t*, usize);
i8 span_readat_i8(span_t*, usize);
i16 span_readat_i16(span_t*, usize);
i32 span_readat_i32(span_t*, usize);
f32 span_readat_f1x3x12(span_t*, usize);

void span_print(const span_t*);
