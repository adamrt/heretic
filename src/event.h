#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "span.h"

#define EVENT_SIZE            (8192)
#define EVENT_COUNT           (500)
#define EVENT_MESSAGES_LEN    (16384)
#define EVENT_INSTRUCTION_MAX (768)
#define EVENT_PARAMETER_MAX   (14)

// There are 126 opcodes in the game. The highest opcode id is 242.
#define OPCODE_COUNT  (126)
#define OPCODE_ID_MAX (243)

typedef struct {
    int id;
    const char* name;
    int param_sizes[EVENT_PARAMETER_MAX];
    int param_count;
} opcode_t;

typedef enum {
    PARAMETER_TYPE_NONE = 0,
    PARAMETER_TYPE_U8 = 1,
    PARAMETER_TYPE_U16 = 2,
} parameter_type_e;

typedef struct {
    parameter_type_e type;
    union {
        u8 u8;
        u16 u16;
    } value;
} parameter_t;

typedef struct {
    int code;
    parameter_t parameters[EVENT_PARAMETER_MAX];
} instruction_t;

// An event is a list of text and instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
//     These are battle setup events and other non map events.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
typedef struct {
    char* messages;
    int messages_len;

    instruction_t* instructions;
    int instruction_count;

    u8 data[EVENT_SIZE];
    bool valid;
} event_t;

event_t read_event(span_t*);
void read_messages(span_t*, char*, int*);
void read_instructions(span_t*, instruction_t*, int*);

extern const opcode_t opcode_list[OPCODE_ID_MAX];
