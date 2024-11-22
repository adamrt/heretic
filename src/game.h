#ifndef GAME_H_
#define GAME_H_

#include "sokol_app.h"

#include "camera.h"
#include "fft.h"

void game_init(void);
void game_update(void);
void game_input(const sapp_event* event);
void game_load_scenario(int num);
void game_load_map(int num);
void game_load_map_state(int num, map_state_t state);
void game_shutdown(void);

typedef enum mode_e {
    MODE_SCENARIO,
    MODE_MAP,
} mode_e;

typedef struct {
    FILE* bin;
    bool bin_loaded;

    mode_e mode;
    scene_t scene;
    camera_t cam;

    struct {
        bool mouse_left;
        bool mouse_right;
    } input;

    struct {
        scenario_t* scenarios;
        event_t* events;
    } fft;
} game_t;

extern game_t g;

#endif // GAME_H_
