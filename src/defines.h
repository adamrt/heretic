#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t usize;

// Maximum number of bytes that can be stored in a bytes_t.
// This is used for resources. Textures are the largest.
enum {
    BYTES_MAX = 131072,
};

typedef struct {
    u8 data[BYTES_MAX];
    usize size;
} bytes_t;
