#include "sokol_gfx.h"

#include "cglm/struct.h"
#include "shader.glsl.h"

#include "scene.h"

#include "game.h"

typedef enum {
    SWITCH_PREV,
    SWITCH_NEXT,
} switch_e;

static void scene_switch(switch_e dir);
static void scene_map_unload(void);

void scene_init(void)
{
    g.scene.center_model = true;
    g.scene.current_scenario = 52;
    scene_load_scenario(g.scene.current_scenario);
}

void scene_shutdown(void)
{
    scene_map_unload();
}

void scene_load_map(int num, map_state_t map_state)
{
    scene_map_unload();

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
        .bindings.samplers[SMP_u_sampler] = gfx.sampler,
        .bindings.images = {
            [IMG_u_texture] = texture,
            [IMG_u_palette] = palette,
        },
    };

    g.scene.map = map;
    g.scene.model = model;
    g.scene.current_map = num;
}

void scene_load_scenario(int num)
{
    scenario_t scenario = g.fft.scenarios[num];
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };
    scene_load_map(scenario.map_id, scenario_state);
}

void scene_prev(void)
{
    scene_switch(SWITCH_PREV);
}

void scene_next(void)
{
    scene_switch(SWITCH_NEXT);
}

static void scene_switch(switch_e dir)
{
    bool is_prev = dir == SWITCH_PREV;

    switch (g.mode) {
    case MODE_SCENARIO:
        g.scene.current_scenario = is_prev ? g.scene.current_scenario - 1 : g.scene.current_scenario + 1;
        scene_load_scenario(g.scene.current_scenario);
        break;

    case MODE_MAP:
        g.scene.current_map = is_prev ? g.scene.current_map - 1 : g.scene.current_map + 1;
        while (!map_list[g.scene.current_map].valid) {
            g.scene.current_map = is_prev ? g.scene.current_map - 1 : g.scene.current_map + 1;
            if (g.scene.current_map < 0) {
                g.scene.current_map = 125;
            }
            if (g.scene.current_map > 125) {
                g.scene.current_map = 0;
            }
        }
        scene_load_map(g.scene.current_map, default_map_state);
        break;

    default:
        assert(false);
    }
}

static void scene_map_unload(void)
{
    if (g.scene.map != NULL) {

        if (g.scene.map->map_data != NULL) {
            free(g.scene.map->map_data);
        }

        sg_destroy_image(g.scene.model.bindings.images[IMG_u_texture]);
        sg_destroy_image(g.scene.model.bindings.images[IMG_u_palette]);
        sg_destroy_buffer(g.scene.model.bindings.vertex_buffers[0]);

        free(g.scene.map);
    }
}
