#pragma once

#include "event.h"
#include "opcode.h"
#include "vm_func.h"

typedef enum {
    WAITTYPE_DIALOG = 0x01,
    WAITTYPE_CAMERA = 0x04,
    WAITTYPE_MAPDARKNESS = 0x06,
    WAITTYPE_MAPLIGHT = 0x07,
    WAITTYPE_BLOCKEND = 0x08,
    WAITTYPE_UNITANIM = 0x0B,
    WAITTYPE_COLORSCREEN = 0x0C,
    WAITTYPE_LOADEVTCHR = 0x34,
    WAITTYPE_DARKSCREEN = 0x36,
    WAITTYPE_DISPLAYCONDITIONS = 0x38,
    WAITTYPE_SHOWGRAPHIC = 0x3D,
    WAITTYPE_EFFECT = 0x41,
    WAITTYPE_INFLICTSTATUS = 0x43,
} waittype_e;

// Virtual machine state
typedef struct {
    opcode_fn_t handlers[OPCODE_ID_MAX];
    const event_t* current_event;
    size_t current_instruction;
    bool is_executing;
} vm_t;

void vm_init(void);
void vm_execute_event(const event_t* event);
void vm_update(void);
void vm_reset(void);
int vm_get_current_instruction(void);
