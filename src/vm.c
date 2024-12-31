#include "vm.h"
#include "camera.h"
#include "opcode.h"
#include "transition.h"
#include "vm_func.h"

static vm_t _state;

void vm_init(void) {
    vm_reset();

    _state.handlers[OPCODE_ID_DISPLAYMESSAGE] = fn_display_message;
    _state.handlers[OPCODE_ID_CAMERA] = fn_camera;
    _state.handlers[OPCODE_ID_WAITFORINSTRUCTION] = fn_wait_for_instruction;
    _state.handlers[OPCODE_ID_WARPUNIT] = fn_warp_unit;
}

void vm_reset(void) {
    _state.current_event = NULL;
    _state.current_instruction = 0;
    _state.is_executing = false;
    camera_reset();
}

void vm_execute_event(const event_t* event) {
    _state.current_event = event;
    _state.current_instruction = 0;
    _state.is_executing = true;
}

void vm_update(void) {
    if (_state.is_executing) {

        // Check if we've reached the end of the event
        if (_state.current_instruction >= _state.current_event->instruction_count) {
            _state.is_executing = false;
            _state.current_event = NULL;
            return;
        }

        // Check if we are waiting on any active transition in our.
        // These types are different than opcode_ids (waittype_e).
        for (int i = 0; i < _state.waiting_count; i++) {
            waittype_e type = _state.waiting[i];
            if (transition_has_active(type)) {
                return;
            } else {
                vm_unwait(type);
            }
        }

        const instruction_t* instr = &_state.current_event->instructions[_state.current_instruction++];
        const opcode_fn_t fn = _state.handlers[instr->opcode];
        if (fn != NULL) {
            fn(instr);
        }
    }
}

int vm_get_current_instruction(void) {
    return _state.current_instruction;
}

void vm_wait(waittype_e type) {
    if (transition_has_active(type)) {
        _state.waiting[_state.waiting_count++] = type;
    }
}

void vm_unwait(waittype_e type) {
    for (int i = 0; i < _state.waiting_count; i++) {
        if (_state.waiting[i] == type) {
            _state.waiting[i] = _state.waiting[--_state.waiting_count];
        }
    }
}
