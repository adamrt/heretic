#include "sokol_time.h"

#include "defines.h"
#include "time.h"

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
    u64 current_time = stm_now();
    u64 elapsed_ticks = stm_diff(current_time, state.last_time);
    f64 elapsed_seconds = stm_sec(elapsed_ticks);

    if (elapsed_seconds >= 1.0) {
        state.fps = state.frame_count / (f32)elapsed_seconds;
        state.frame_count = 0;
        state.last_time = current_time;
    }
}

f32 time_get_fps(void)
{
    return state.fps;
}
