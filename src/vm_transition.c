#include <stdbool.h>

#include "cglm/util.h"

#include "util.h"
#include "vm.h"
#include "vm_opcode.h"
#include "vm_transition.h"

static transition_manager_t _state;

void vm_transition_reset(void) {
    _state.transaction_count = 0;
}

void vm_transition_update(void) {
    for (usize i = 0; i < _state.transaction_count;) {
        transition_t* t = &_state.transitions[i];
        t->frame_current++;

        f32 progress = glm_clamp(t->frame_current / t->frame_total, 0.0f, 1.0f);
        f32 value = t->start + (t->end - t->start) * progress;

        *(f32*)(t->target) = value;

        if (progress >= 1.0f) {
            _state.transitions[i] = _state.transitions[_state.transaction_count - 1];
            _state.transaction_count--;
        } else {
            i++;
        }
    }
}

void vm_transition_add(opcode_e opcode, void* target, f32 start, f32 end, f32 duration) {
    ASSERT(_state.transaction_count <= VM_TRANSITION_MAX, "Too many transitions");

    transition_t* t = &_state.transitions[_state.transaction_count++];
    t->opcode = opcode;
    t->start = start;
    t->end = end;
    t->frame_total = duration;
    t->frame_current = 0.0f;
    t->target = target;
}

bool vm_transition_has_active(waittype_e type) {
    opcode_e opcodes[4];
    int count = 0;

    switch (type) {
    case WAITTYPE_DIALOG:
        opcodes[0] = OPCODE_DISPLAYMESSAGE;
        opcodes[1] = OPCODE_CHANGEDIALOG;
        count = 2;
        break;
    case WAITTYPE_CAMERA:
        opcodes[0] = OPCODE_CAMERA;
        opcodes[1] = OPCODE_CAMERAFUSIONSTART;
        opcodes[2] = OPCODE_CAMERAFUSIONEND;
        opcodes[3] = OPCODE_FOCUS;
        count = 4;
        break;
    case WAITTYPE_MAPDARKNESS:
        opcodes[0] = OPCODE_MAPDARKNESS;
        count = 1;
        break;
    case WAITTYPE_MAPLIGHT:
        opcodes[0] = OPCODE_MAPLIGHT;
        count = 1;
        break;
    case WAITTYPE_BLOCKEND:
        opcodes[0] = OPCODE_BLOCKSTART;
        opcodes[1] = OPCODE_BLOCKEND;
        count = 2;
        break;
    case WAITTYPE_UNITANIM:
        opcodes[0] = OPCODE_UNITANIM;
        count = 1;
        break;
    case WAITTYPE_COLORSCREEN:
        opcodes[0] = OPCODE_COLORSCREEN;
        count = 1;
        break;
    case WAITTYPE_LOADEVTCHR:
        opcodes[0] = OPCODE_LOADEVTCHR;
        count = 1;
        break;
    case 0x36:
        opcodes[0] = OPCODE_DARKSCREEN;
        opcodes[1] = OPCODE_REMOVEDARKSCREEN;
        count = 2;
        break;
    case WAITTYPE_DISPLAYCONDITIONS:
        opcodes[0] = OPCODE_DISPLAYCONDITIONS;
        count = 1;
        break;
    case WAITTYPE_SHOWGRAPHIC:
        opcodes[0] = OPCODE_SHOWGRAPHIC;
        count = 1;
        break;
    case WAITTYPE_EFFECT:
        opcodes[0] = OPCODE_EFFECT;
        opcodes[1] = OPCODE_EFFECTSTART;
        opcodes[2] = OPCODE_EFFECTEND;
        count = 3;
        break;
    case WAITTYPE_INFLICTSTATUS:
        opcodes[0] = OPCODE_INFLICTSTATUS;
        count = 1;
        break;
    }

    for (usize i = 0; i < _state.transaction_count; i++) {
        for (int j = 0; j < count; j++) {
            if (_state.transitions[i].opcode == opcodes[j]) {
                return true;
            }
        }
    }

    return false;
}
