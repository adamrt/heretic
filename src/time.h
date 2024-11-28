#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

typedef struct {
    float fps;
    int frame_count;
    uint64_t last_time;
} time_t;

void time_init(void);
void time_update(void);
float time_get_fps(void);

#endif // TIME_H_
