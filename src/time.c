#include "time.h"

#include "sokol_time.h"

static time_t state;

void time_init(void)
{
    stm_setup();

    state.frame_count = 0;
    state.fps = 0.0f;
    state.last_time = stm_now();
}

void time_update(void)
{
    state.frame_count++;
    uint64_t current_time = stm_now();
    uint64_t elapsed_ticks = stm_diff(current_time, state.last_time);
    double elapsed_seconds = stm_sec(elapsed_ticks);

    if (elapsed_seconds >= 1.0) {
        state.fps = state.frame_count / (float)elapsed_seconds;
        state.frame_count = 0;
        state.last_time = current_time;
    }
}

float time_get_fps(void)
{
    return state.fps;
}
