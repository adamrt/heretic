#include <assert.h>
#include <stdio.h>

#include "cglm/types-struct.h"
#include "sokol_gfx.h"

#include "shader.glsl.h"

#include "event.h"
#include "scenario_record.h"
#include "scene.h"

static scene_t _state;

// Temporary
scene_t* scene_get_internals(void) { return &_state; }

typedef enum {
    SWITCH_PREV,
    SWITCH_NEXT,
} switch_e;

static void _scene_switch(switch_e dir);
static void _scene_map_unload(void);
static void _scene_scenario_unload(void);

void scene_init(void)
{
    _state.center_model = true;
    _state.current_scenario = 10;
    scene_load_scenario(_state.current_scenario);
}

void scene_shutdown(void)
{
    _scene_map_unload();
    _scene_scenario_unload();
}

void scene_update(void)
{
    if (_state.center_model) {
        _state.model.transform.translation = _state.map->centered_translation;
    } else {
        _state.model.transform.translation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
    }
}

void scene_load_map(int num, map_state_t map_state)
{
    _scene_map_unload();

    map_t* map = calloc(1, sizeof(map_t));
    read_map(num, map_state, map);

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
        .bindings.vertex_buffers[0] = vbuf,
        .bindings.samplers[SMP_u_sampler] = gfx_get_sampler(),
        .bindings.images = {
            [IMG_u_texture] = texture,
            [IMG_u_palette] = palette,
        },
    };

    _state.map = map;
    _state.model = model;
    _state.current_map = num;
}

void scene_load_scenario(int num)
{
    scenario_record_t scenario = scenario_get_record(num);
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };
    event_t event = event_get_event(num);
    _state.messages = event_get_messages(&event);
    scene_load_map(scenario.map_id, scenario_state);
}

char** scene_get_messages(void)
{
    return _state.messages;
}

void scene_prev(void)
{
    _scene_switch(SWITCH_PREV);
}

void scene_next(void)
{
    _scene_switch(SWITCH_NEXT);
}

static void _scene_switch(switch_e dir)
{
    bool is_prev = dir == SWITCH_PREV;

    switch (_state.mode) {
    case MODE_SCENARIO:
        _state.current_scenario = is_prev ? _state.current_scenario - 1 : _state.current_scenario + 1;
        while (true) {
            if (_state.current_scenario < 0) {
                _state.current_scenario = SCENARIO_COUNT - 1;
            }
            if (_state.current_scenario > SCENARIO_COUNT - 1) {
                _state.current_scenario = 0;
            }
            scenario_record_t scenario = scenario_get_record(_state.current_scenario);
            event_t event = event_get_event(scenario.event_id);
            if (!event.valid) {
                _state.current_scenario = is_prev ? _state.current_scenario - 1 : _state.current_scenario + 1;
                continue;
            }
            scene_load_scenario(_state.current_scenario);
            break;
        }
        break;

    case MODE_MAP:
        _state.current_map = is_prev ? _state.current_map - 1 : _state.current_map + 1;
        while (!map_list[_state.current_map].valid) {
            _state.current_map = is_prev ? _state.current_map - 1 : _state.current_map + 1;
            if (_state.current_map < 0) {
                _state.current_map = MAP_COUNT - 1;
            }
            if (_state.current_map > MAP_COUNT - 1) {
                _state.current_map = 0;
            }
        }
        scene_load_map(_state.current_map, default_map_state);
        break;

    default:
        assert(false);
    }
}

static void _scene_map_unload(void)
{
    if (_state.map != NULL) {

        if (_state.map->map_data != NULL) {
            free(_state.map->map_data);
        }

        sg_destroy_image(_state.model.bindings.images[IMG_u_texture]);
        sg_destroy_image(_state.model.bindings.images[IMG_u_palette]);
        sg_destroy_buffer(_state.model.bindings.vertex_buffers[0]);

        free(_state.map);
    }
}
static void _scene_scenario_unload(void)
{
    if (_state.messages != NULL) {
        for (int i = 0; i < EVENT_MESSAGE_MAX; i++) {
            if (_state.messages[i] != NULL) {
                free((void*)_state.messages[i]);
            }
        }
    }
}
