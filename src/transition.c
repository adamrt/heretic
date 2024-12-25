#include <stdbool.h>

#include "cglm/util.h"

#include "transition.h"

static transition_manager_t _state;

void transition_update(void) {
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

void transition_add(opcode_id_t opcode_id, void* target, f32 start, f32 end, f32 duration) {
    ASSERT(_state.transaction_count <= TRANSITION_MAX, "Too many transitions");

    transition_t* t = &_state.transitions[_state.transaction_count++];
    t->opcode_id = opcode_id;
    t->start = start;
    t->end = end;
    t->frame_total = duration;
    t->frame_current = 0.0f;
    t->target = target;
}

bool transition_active(void) {
    return _state.transaction_count > 0;
}
