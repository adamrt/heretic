#ifndef EVENT_H_
#define EVENT_H_

#include <stdbool.h>
#include <stdint.h>

#define EVENT_TEXT_SIZE_MAX (8141)
#define EVENT_CODE_SIZE_MAX (3647)
#define EVENT_MESSAGE_MAX   (1024)

// There are 126 opcodes in the game. The highest opcode id is 242.
#define OPCODE_COUNT  (126)
#define OPCODE_ID_MAX (243)

// An event is a list of text and instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
//     These are battle setup events and other non map events.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
typedef struct {
    uint8_t text[EVENT_TEXT_SIZE_MAX];
    uint8_t code[EVENT_CODE_SIZE_MAX];
    int text_size;
    int code_size;
    bool valid;
} event_t;

typedef struct {
    int id;
    const char* name;
    int arg_types[10];
    int arg_count;
} opcode_t;

event_t event_get(int);
char** event_get_messages(event_t*);

extern const opcode_t opcode_list[OPCODE_ID_MAX];

#endif // EVENT_H_
