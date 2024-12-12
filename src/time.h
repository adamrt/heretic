#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

#include "defines.h"

typedef struct {
    f32 fps;
    u32 frame_count;
    u64 last_time;
} time_t;

void time_init(void);
void time_update(void);
f32 time_get_fps(void);

#endif // TIME_H_
