#pragma once

#include "defines.h"
#include "span.h"
#include "vm_opcode.h"

enum {
    VM_INSTRUCTION_MAX = 768,
};

typedef enum {
    VM_PARAM_TYPE_NONE = 0,
    VM_PARAM_TYPE_U8 = 1,
    VM_PARAM_TYPE_U16 = 2,
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
