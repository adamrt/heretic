#include "vm.h"
#include "opcode.h"
#include "transition.h"
#include "vm_func.h"

// Virtual machine state
static struct {
    opcode_fn_t handlers[OPCODE_ID_MAX];
    const event_t* current_event;
    size_t current_instruction;
    bool is_executing;

    waittype_e waiting[100];
    int waiting_count;
} _state;

// Forward declarations
static void vm_unwait(waittype_e);

// Initialize the virtual machine
void vm_init(void) {
    vm_reset();

    // Setup the opcode handlers
    _state.handlers[OPCODE_ID_DISPLAYMESSAGE] = fn_display_message;
    _state.handlers[OPCODE_ID_CAMERA] = fn_camera;
    _state.handlers[OPCODE_ID_WAITFORINSTRUCTION] = fn_wait_for_instruction;
    _state.handlers[OPCODE_ID_WARPUNIT] = fn_warp_unit;
}

// Reset the virtual machine state
void vm_reset(void) {
    _state.current_event = NULL;
    _state.current_instruction = 0;
    _state.is_executing = false;
}

void vm_execute_event(const event_t* event) {
    vm_reset();

    _state.current_event = event;
    _state.is_executing = true;
}

void vm_update(void) {
    if (!_state.is_executing) {
        return;
    }

    // Check if we've reached the end of the event
    if (_state.current_instruction >= _state.current_event->instruction_count) {
        vm_reset();
        return;
    }

    // Check if we are waiting on any active transition.
    // These types are different than opcode_ids (waittype_e).
    for (int i = 0; i < _state.waiting_count; i++) {
        waittype_e waittype = _state.waiting[i];
        if (transition_has_active(waittype)) {
            // We are waiting on a transition, do not execute the next instr.
            return;
        } else {
            // If we are not waiting, we can unwait this type.
            vm_unwait(waittype);
        }
    }

    const instruction_t* instr = &_state.current_event->instructions[_state.current_instruction++];
    const opcode_fn_t fn = _state.handlers[instr->opcode];
    if (fn != NULL) {
        fn(instr);
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

static void vm_unwait(waittype_e type) {
    for (int i = 0; i < _state.waiting_count; i++) {
        if (_state.waiting[i] == type) {
            _state.waiting[i] = _state.waiting[--_state.waiting_count];
        }
    }
}

const char* waittype_str(waittype_e waittype) {
    switch (waittype) {
    case WAITTYPE_DIALOG:
        return "Dialog";
    case WAITTYPE_CAMERA:
        return "Camera";
    case WAITTYPE_MAPDARKNESS:
        return "Map Darkness";
    case WAITTYPE_MAPLIGHT:
        return "Map Light";
    case WAITTYPE_BLOCKEND:
        return "Block End";
    case WAITTYPE_UNITANIM:
        return "Unit Animation";
    case WAITTYPE_COLORSCREEN:
        return "Color Screen";
    case WAITTYPE_LOADEVTCHR:
        return "Load Event Character";
    case WAITTYPE_DARKSCREEN:
        return "Dark Screen";
    case WAITTYPE_DISPLAYCONDITIONS:
        return "Display Conditions";
    case WAITTYPE_SHOWGRAPHIC:
        return "Show Graphic";
    case WAITTYPE_EFFECT:
        return "Effect";
    case WAITTYPE_INFLICTSTATUS:
        return "Inflict Status";
    default:
        return "Unknown Wait Type";
    }
}
