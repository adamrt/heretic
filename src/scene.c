#include <stdio.h>

#include "sokol_gfx.h"

#include "cglm/types-struct.h"
#include "shader.glsl.h"

#include "background.h"
#include "event.h"
#include "memory.h"
#include "scenario.h"
#include "scene.h"
#include "util.h"
#include "vm.h"

static scene_t _state;

// Temporary
scene_t* scene_get_internals(void) { return &_state; }

typedef enum {
    SWITCH_PREV,
    SWITCH_NEXT,
} switch_e;

static void _scene_switch(switch_e dir);
static void _scene_map_unload(void);

void scene_init(void) {
    _state.current_scenario_id = 78;
    _state.mode = MODE_SCENARIO;
    scene_load_scenario(_state.current_scenario_id);
}

void scene_shutdown(void) {
    _scene_map_unload();
}

void scene_update(void) {
}

void scene_render(void) {
    gfx_render_begin();
    {
        background_render(_state.map->mesh.lighting.bg_top, _state.map->mesh.lighting.bg_bottom);
        for (int i = 0; i < _state.model_count; i++) {
            gfx_render_model(&_state.models[i], &_state.map->mesh.lighting);
        }
    }
    gfx_render_end();
}

void scene_load_map(int num, map_state_t map_state) {
    _scene_map_unload();

    map_t* map = read_map(num, map_state);
    model_t model = gfx_map_to_model(map);

    _state.map = map;
    _state.models[_state.model_count++] = model;
    _state.current_map = num;
}

void scene_load_scenario(int scenario_id) {
    vm_reset();

    scenario_t scenario = scenario_get_scenario(scenario_id);
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };

    _state.event = event_get_event(scenario.event_id);
    scene_load_map(scenario.map_id, scenario_state);
}

void scene_set_map_rotation(f32 maprot) {
    _state.models[0].transform.rotation.y = maprot;
}

event_t scene_get_event(void) { return _state.event; }

void scene_prev(void) {
    _scene_switch(SWITCH_PREV);
}

void scene_next(void) {
    _scene_switch(SWITCH_NEXT);
}

static void _scene_switch(switch_e dir) {
    bool is_prev = dir == SWITCH_PREV;

    switch (_state.mode) {
    case MODE_SCENARIO:
        _state.current_scenario_id = is_prev ? _state.current_scenario_id - 1 : _state.current_scenario_id + 1;
        while (true) {
            if (_state.current_scenario_id < 0) {
                _state.current_scenario_id = SCENARIO_COUNT - 1;
            }
            if (_state.current_scenario_id > SCENARIO_COUNT - 1) {
                _state.current_scenario_id = 0;
            }
            event_desc_t desc = event_get_desc_by_scenario_id(_state.current_scenario_id);
            if (!desc.usable) {
                _state.current_scenario_id = is_prev ? _state.current_scenario_id - 1 : _state.current_scenario_id + 1;
                continue;
            }
            scene_load_scenario(_state.current_scenario_id);
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
        ASSERT(false, "Invalid mode %d", _state.mode);
    }
}

static void _scene_map_unload(void) {
    if (_state.map == NULL) {
        return;
    }

    memory_free(_state.map->map_data);
    memory_free(_state.map);

    _state.model_count--;
    sg_destroy_image(_state.models[_state.model_count].texture);
    sg_destroy_image(_state.models[_state.model_count].palette);
    sg_destroy_buffer(_state.models[_state.model_count].vbuf);
    sg_destroy_buffer(_state.models[_state.model_count].ibuf);
}
