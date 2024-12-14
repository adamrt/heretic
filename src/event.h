#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bin.h"
#include "defines.h"

#define EVENT_SIZE            (8192)
#define EVENT_COUNT           (500)
#define EVENT_TEXT_SIZE_MAX   (8141)
#define EVENT_CODE_SIZE_MAX   (3647)
#define EVENT_MESSAGE_MAX     (1024)
#define EVENT_INSTRUCTION_MAX (768)
#define EVENT_PARAMETER_MAX   (14)

// There are 126 opcodes in the game. The highest opcode id is 242.
#define OPCODE_COUNT  (126)
#define OPCODE_ID_MAX (243)

typedef struct {
    char* cstr;
    usize len;
} message_t;

// An event is a list of text and instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
//     These are battle setup events and other non map events.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
typedef struct {
    u8 code[EVENT_CODE_SIZE_MAX];
    usize code_size;

    message_t* messages;
    int message_count;

    u8 data[EVENT_SIZE];
    bool valid;
} event_t;

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

event_t read_event(buffer_t*);
void read_messages(buffer_t*, message_t*, int*);
instruction_t* event_get_instructions(event_t, int*);

extern const opcode_t opcode_list[OPCODE_ID_MAX];
