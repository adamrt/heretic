#include "vm.h"
#include "camera.h"
#include "opcode.h"
#include "transition.h"
#include "vm_func.h"

static vm_t _state;

void vm_init(void) {
    vm_reset();

    _state.handlers[OPCODE_ID_DISPLAYMESSAGE] = fn_10_displaymessage;
    _state.handlers[OPCODE_ID_CAMERA] = fn_19_camera;
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
    if (_state.is_executing && !transition_active()) {
        _state.current_instruction++;

        // Check if we've reached the end of the event
        if (_state.current_instruction >= _state.current_event->instruction_count) {
            _state.is_executing = false;
            _state.current_event = NULL;
            return;
        }

        const instruction_t* instr = &_state.current_event->instructions[_state.current_instruction];
        const opcode_fn_t fn = _state.handlers[instr->opcode];
        if (fn != NULL) {
            fn(instr);
        }
    }
}
