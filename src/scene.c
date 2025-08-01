#include "camera.h"
#include "sokol_gfx.h"

#include "cglm/types-struct.h"
#include "shader.glsl.h"

#include "gfx.h"
#include "gfx_background.h"
#include "gfx_line.h"
#include "gfx_model.h"
#include "gfx_sprite.h"
#include "map.h"
#include "scenario.h"
#include "scene.h"
#include "unit.h"
#include "util.h"
#include "vm.h"
#include "vm_event.h"
#include "vm_transition.h"

static scene_t _state;

// Temporary
scene_t* scene_get_internals(void) { return &_state; }

typedef enum {
    SWITCH_PREV,
    SWITCH_NEXT,
} switch_e;

static void _scene_switch(switch_e dir);

void scene_init(void) {
    _state.current_scenario_id = 78;
    _state.mode = MODE_EVENT;
    scene_load_scenario(_state.current_scenario_id);
}

void scene_reset(void) {
    _state.current_map = 0;

    vm_reset();
    vm_transition_reset();
    camera_reset();

    map_destroy(_state.map);
    gfx_model_destroy();
    gfx_sprite_reset();
}

void scene_shutdown(void) {
    scene_reset();
}

void scene_render(void) {
    gfx_render_begin();
    {
        gfx_background_render();
        gfx_model_render();
        gfx_line_render_axis();
        gfx_sprite_render();
    }
    gfx_render_end();
}

void scene_load_map(int num, map_state_t map_state) {
    scene_reset();

    map_t* map = read_map(num);
    model_t model = gfx_model_create(map, map_state);
    gfx_model_set(model);
    gfx_background_set(model.lighting.bg_top, model.lighting.bg_bottom);

    _state.map = map;
    _state.map_state = map_state;
    _state.current_map = num;
}

void scene_load_units(int entd_id) {
    units_t units = unit_get_units(entd_id);
    _state.units = units;
}

void scene_load_scenario(int scenario_id) {
    scenario_t scenario = scenario_get_scenario(scenario_id);
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };

    _state.event = vm_event_get_event(scenario.event_id);
    scene_load_map(scenario.map_id, scenario_state);
    scene_load_units(scenario.entd_id);
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
    case MODE_EVENT:
        _state.current_scenario_id = is_prev ? _state.current_scenario_id - 1 : _state.current_scenario_id + 1;
        while (true) {
            if (_state.current_scenario_id < 0) {
                _state.current_scenario_id = SCENARIO_COUNT - 1;
            }
            if (_state.current_scenario_id > SCENARIO_COUNT - 1) {
                _state.current_scenario_id = 0;
            }
            event_desc_t desc = vm_event_get_desc_by_scenario_id(_state.current_scenario_id);
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
