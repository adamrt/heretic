#include "map.h"
#include "sokol_gfx.h"

#include "cglm/types-struct.h"
#include "shader.glsl.h"

#include "background.h"
#include "event.h"
#include "gfx.h"
#include "scenario.h"
#include "scene.h"
#include "sprite.h"
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
    _state.mode = MODE_EVENT;
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
        background_render(_state.model.lighting.bg_top, _state.model.lighting.bg_bottom);
        gfx_render_model(&_state.model);
        for (int i = 0; i < 5; i++) {
            if (texture_valid(_state.sprites[i].texture)) {
                sprite_render(&_state.sprites[i]);
            }
        }
    }
    gfx_render_end();
}

void scene_load_map(int num, map_state_t map_state) {
    _scene_map_unload();

    map_t* map = read_map(num);
    model_t model = map_make_model(map, map_state);

    _state.map = map;
    _state.model = model;
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

    texture_t texture = sprite_get_paletted_texture(F_EVENT__UNIT_BIN, 0);

    f32 z = -30.0f;
    for (int i = 0; i < 5; i++) {
        transform_t tranform = {
            .translation = { { -23.0f, 23.0f, z } },
            .rotation = { { 0.0f, 0.0f, 0.0f } },
            .scale = { { 15.0f, 15.0f, 15.0f } },
        };
        sprite_t sprite = sprite_create(texture, (vec2s) { { 24.0f * i, 0.0f } }, (vec2s) { { 24.0f, 40.0f } }, tranform);
        _state.sprites[i] = sprite;
        z += 30.0f;
    }
}

void scene_set_map_rotation(f32 maprot) {
    _state.model.transform.rotation.y = maprot;
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
    map_destroy(_state.map);
    model_destroy(_state.model);
}
