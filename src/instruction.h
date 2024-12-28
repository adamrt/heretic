#pragma once

#include "defines.h"
#include "opcode.h"
#include "span.h"

constexpr int INSTRUCTION_MAX = 768;

typedef enum {
    PARAM_TYPE_NONE = 0,
    PARAM_TYPE_U8 = 1,
    PARAM_TYPE_U16 = 2,
} param_type_e;

typedef struct {
    param_type_e type;
    union {
        u8 u8;
        i8 i8;
        u16 u16;
        i16 i16;
    } value;
} param_t;

typedef struct {
    opcode_id_t opcode;
    param_t params[OPCODE_PARAM_MAX];
    u8 param_count;
} instruction_t;

usize read_instructions(span_t*, instruction_t*);
