#include "game.h"
#include "bin.h"
#include "camera.h"
#include "gfx.h"
#include "gui.h"
#include "scene.h"
#include "time.h"

#if defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#endif

// data_init opens the FFT bin file and loads the default scene. It is called
// during game_init() for native builds, and after file upload on a wasm build.
void data_init(void)
{
    bin_init();
    gui_cache();
    scene_init();
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
    bin_shutdown();
    gui_shutdown();
    gfx_shutdown();
}

void game_update(void)
{
    if (!bin_is_loaded()) {
        return;
    }

    time_update();
    scene_update();
    camera_update();
    scene_render();
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
            camera_key_left();
            break;
        case SAPP_KEYCODE_RIGHT:
            camera_key_right();
            break;
        case SAPP_KEYCODE_UP:
            camera_key_up();
            break;
        case SAPP_KEYCODE_DOWN:
            camera_key_down();
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
            camera_mouse_movement(event->mouse_dx, event->mouse_dy);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        camera_mouse_wheel(event->scroll_y);
        break;

    default:
        break;
    }
}
