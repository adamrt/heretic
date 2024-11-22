#ifndef GAME_H_
#define GAME_H_

#include "sokol_app.h"
#include "sokol_time.h"

#include "camera.h"
#include "fft.h"

void game_init(void);
void game_update(void);
void game_input(const sapp_event* event);
void game_shutdown(void);

void game_map_load(int num, map_state_t state);
void game_scenario_load(int num);

typedef enum mode_e {
    MODE_MAP,
    MODE_SCENARIO,
} mode_e;

typedef struct {
    FILE* bin;
    bool bin_loaded;

    mode_e mode;
    scene_t scene;
    camera_t cam;

    struct {
        scenario_t* scenarios;
        event_t* events;
    } fft;

    struct {
        float fps;
        int frame_count;
        uint64_t last_time;
    } time;
} game_t;

extern game_t g;

#endif // GAME_H_
