#include "game.h"
#include "background.h"
#include "camera.h"
#include "filesystem.h"
#include "font.h"
#include "gfx.h"
#include "gui.h"
#include "memory.h"
#include "scene.h"
#include "sprite.h"
#include "time.h"
#include "transition.h"
#include "vm.h"

#if defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#endif

// data_init opens the FFT bin file and loads the default scene. It is called
// during game_init() for native builds, and after file upload on a wasm build.
void data_init(void) {
    filesystem_init();
    font_init();
    sprite_init();
    scene_init();
}

void game_init(void) {
    memory_init();
    time_init();
    camera_init();
    vm_init();
    gfx_init();
    background_init();
    gui_init();

#if !defined(__EMSCRIPTEN__)
    data_init();
#endif
}

void game_shutdown(void) {
    scene_shutdown();
    filesystem_shutdown();
    font_shutdown();
    sprite_shutdown();
    gui_shutdown();
    background_shutdown();
    gfx_shutdown();
    memory_shutdown();
}

void game_update(void) {
    time_update();
    vm_update();
    transition_update();
    scene_update();
    scene_render();
}

void game_input(const sapp_event* event) {
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

        case SAPP_KEYCODE_W:
            camera_freefly_motion((freefly_motion_t) { .forward = 1.0f });
            break;
        case SAPP_KEYCODE_A:
            camera_freefly_motion((freefly_motion_t) { .right = -1.0f });
            break;
        case SAPP_KEYCODE_S:
            camera_freefly_motion((freefly_motion_t) { .forward = -1.0f });
            break;
        case SAPP_KEYCODE_D:
            camera_freefly_motion((freefly_motion_t) { .right = 1.0f });
            break;
        case SAPP_KEYCODE_R:
            camera_freefly_motion((freefly_motion_t) { .up = 1.0f });
            break;
        case SAPP_KEYCODE_F:
            camera_freefly_motion((freefly_motion_t) { .up = -1.0f });
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
            orbit_motion_t motion = {
                .theta_deg = event->mouse_dx,
                .phi_deg = event->mouse_dy,
            };
            camera_orbit_motion(motion);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL: {
        orbit_motion_t motion = { .dolly = event->scroll_y };
        camera_orbit_motion(motion);
        break;
    }

    default:
        break;
    }
}
