#include "game.h"
#include "camera.h"
#include "event.h"
#include "gfx.h"
#include "gui.h"
#include "scenario.h"

#if defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#endif

game_t g = {
    .mode = MODE_SCENARIO,
};

// Forward declarations
static void state_update(void);
static void time_init(void);
static void time_update(void);

// data_init is called during game_init() for native builds, and after file
// upload on a wasm build.
void data_init(void)
{
    g.bin = fopen("../fft.bin", "rb");
    assert(g.bin != NULL);
    g.bin_loaded = true;

    load_events();
    load_scenarios();

    scene_init();
}

static void data_shutdown(void)
{
    free(g.fft.scenarios);
    free(g.fft.events);
    fclose(g.bin);
}

void game_init(void)
{
    time_init();
    camera_init();
    gfx_init();
    gui_init();

#if !defined(__EMSCRIPTEN__)
    data_init();
#endif
}

void game_shutdown(void)
{
    scene_shutdown();
    data_shutdown();
    gui_shutdown();
    gfx_shutdown();
}

void game_input(const sapp_event* event)
{
    bool handled_by_ui = gui_input(event);
    bool is_mouse_event = event->type == SAPP_EVENTTYPE_MOUSE_MOVE
        || event->type == SAPP_EVENTTYPE_MOUSE_SCROLL
        || event->type == SAPP_EVENTTYPE_MOUSE_DOWN
        || event->type == SAPP_EVENTTYPE_MOUSE_UP;

    if (handled_by_ui && is_mouse_event) {
        return;
    }

    switch (event->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        switch (event->key_code) {
        case SAPP_KEYCODE_ESCAPE:
            sapp_request_quit();
            break;
        case SAPP_KEYCODE_K:
            scene_next();
            break;
        case SAPP_KEYCODE_J:
            scene_prev();
            break;
        case SAPP_KEYCODE_LEFT:
            camera_left();
            break;
        case SAPP_KEYCODE_RIGHT:
            camera_right();
            break;
        case SAPP_KEYCODE_UP:
            camera_up();
            break;
        case SAPP_KEYCODE_DOWN:
            camera_down();
            break;
        default:
            break;
        }

        __attribute__((fallthrough));
    case SAPP_EVENTTYPE_MOUSE_DOWN:
    case SAPP_EVENTTYPE_MOUSE_UP: {
        bool is_down = (event->type == SAPP_EVENTTYPE_MOUSE_DOWN);
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(is_down);
        }
        break;
    }

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (sapp_mouse_locked()) {
            camera_orbit(event->mouse_dx, event->mouse_dy);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        camera_zoom(event->scroll_y);
        break;

    default:
        break;
    }
}

void game_update(void)
{
    if (!g.bin_loaded) {
        return;
    }

    time_update();
    state_update();
    camera_update();
    gfx_update();
}

static void state_update(void)
{
    if (g.scene.center_model) {
        g.scene.model.transform.translation = g.scene.map->centered_translation;
    } else {
        g.scene.model.transform.translation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    }
}

static void time_init(void)
{
    stm_setup();
    g.time.last_time = stm_now();
}

static void time_update(void)
{
    g.time.frame_count++;
    uint64_t current_time = stm_now();
    uint64_t elapsed_ticks = stm_diff(current_time, g.time.last_time);
    double elapsed_seconds = stm_sec(elapsed_ticks);

    if (elapsed_seconds >= 1.0) {
        g.time.fps = g.time.frame_count / (float)elapsed_seconds;
        g.time.frame_count = 0;
        g.time.last_time = current_time;
    }
}
