#include "sokol_gfx.h"

#include "cglm/struct.h"
#include "shader.glsl.h"

#include "camera.h"
#include "fft.h"
#include "game.h"
#include "gfx.h"
#include "gui.h"

#if defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#endif

game_t g = {
    .mode = MODE_SCENARIO,
    .scene = {
        .center_model = true,
        .current_scenario = 52,
    },
};

// Forward declarations
void data_init(void);
static void state_update(void);
static void scenario_prev(void);
static void scenario_next(void);
static void map_load(int num, map_state_t);
static void map_unload(void);

void game_init(void)
{
    gfx_init();
    camera_init();
    gui_init();

#if !defined(__EMSCRIPTEN__)
    data_init();
#endif
}

// data_init is called during game_init() for native builds, and after file
// upload on a wasm build.
void data_init(void)
{
    g.bin = fopen("../fft.bin", "rb");
    assert(g.bin != NULL);
    g.bin_loaded = true;

    bin_load_global_data();
    game_load_scenario(g.scene.current_scenario);
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
            scenario_next();
            break;
        case SAPP_KEYCODE_J:
            scenario_prev();
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

    state_update();
    camera_update();
    gfx_update();
}

void game_load_scenario(int num)
{
    scenario_t scenario = g.fft.scenarios[num];
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };
    map_load(scenario.map_id, scenario_state);
}

void game_load_map(int num)
{
    map_load(num, default_map_state);
}

void game_load_map_state(int num, map_state_t map_state)
{
    map_load(num, map_state);
}

void game_shutdown(void)
{
    fclose(g.bin);
    map_unload();

    bin_free_global_data();

    gui_shutdown();
    gfx_shutdown();
}

static void state_update(void)
{
    if (g.scene.center_model) {
        g.scene.model.transform.translation = g.scene.map->centered_translation;
    } else {
        g.scene.model.transform.translation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    }
}

static void scenario_next(void)
{
    if (g.mode == MODE_SCENARIO) {
        g.scene.current_scenario++;
        game_load_scenario(g.scene.current_scenario);
    } else if (g.mode == MODE_MAP) {
        g.scene.current_map++;
        while (!map_list[g.scene.current_map].valid) {
            g.scene.current_map++;
            if (g.scene.current_map > 125) {
                g.scene.current_map = 0;
            }
        }
        game_load_map(g.scene.current_map);
    }
}

static void scenario_prev(void)
{
    if (g.mode == MODE_SCENARIO) {
        g.scene.current_scenario--;
        game_load_scenario(g.scene.current_scenario);
    } else if (g.mode == MODE_MAP) {
        g.scene.current_map--;
        while (!map_list[g.scene.current_map].valid) {
            g.scene.current_map--;
            if (g.scene.current_map < 0) {
                g.scene.current_map = 125;
            }
        }
        game_load_map(g.scene.current_map);
    }
}

static void map_load(int num, map_state_t map_state)
{
    map_unload();

    map_t* map = read_map(num, map_state);

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(map->vertices),
        .label = "mesh-vertices",
    });

    sg_image texture = sg_make_image(&(sg_image_desc) {
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(map->texture.data),
    });

    sg_image palette = sg_make_image(&(sg_image_desc) {
        .width = PALETTE_WIDTH,
        .height = PALETTE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(map->mesh.palette.data),
    });

    model_t model = {
        .transform.scale = { { 1.0f, 1.0f, 1.0f } },
        .texture = texture,
        .palette = palette,
        .vbuffer = vbuf,
        .bindings.vertex_buffers[0] = vbuf,
        .bindings.fs = {
            .images[SLOT_u_texture] = texture,
            .images[SLOT_u_palette] = palette,
            .samplers[SLOT_u_sampler] = gfx.sampler,
        },
    };

    g.scene.map = map;
    g.scene.model = model;

    g.scene.current_map = num;
}

static void map_unload(void)
{
    if (g.scene.map != NULL) {

        if (g.scene.map->map_data != NULL) {
            free(g.scene.map->map_data);
        }

        sg_destroy_image(g.scene.model.texture);
        sg_destroy_image(g.scene.model.palette);
        sg_destroy_buffer(g.scene.model.vbuffer);
        free(g.scene.map);
    }
}
