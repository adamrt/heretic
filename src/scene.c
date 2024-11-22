#include "sokol_gfx.h"

#include "cglm/struct.h"
#include "shader.glsl.h"

#include "scene.h"

#include "game.h"

void game_map_load(int num, map_state_t map_state)
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

void game_scenario_load(int num)
{
    scenario_t scenario = g.fft.scenarios[num];
    map_state_t scenario_state = {
        .time = scenario.time,
        .weather = scenario.weather,
        .layout = 0,
    };
    game_map_load(scenario.map_id, scenario_state);
}

void map_unload(void)
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

void map_next(void)
{
    if (g.mode == MODE_SCENARIO) {
        g.scene.current_scenario++;
        game_scenario_load(g.scene.current_scenario);
    } else if (g.mode == MODE_MAP) {
        g.scene.current_map++;
        while (!map_list[g.scene.current_map].valid) {
            g.scene.current_map++;
            if (g.scene.current_map > 125) {
                g.scene.current_map = 0;
            }
        }
        game_map_load(g.scene.current_map, default_map_state);
    }
}

void map_prev(void)
{
    if (g.mode == MODE_SCENARIO) {
        g.scene.current_scenario--;
        game_scenario_load(g.scene.current_scenario);
    } else if (g.mode == MODE_MAP) {
        g.scene.current_map--;
        while (!map_list[g.scene.current_map].valid) {
            g.scene.current_map--;
            if (g.scene.current_map < 0) {
                g.scene.current_map = 125;
            }
        }
        game_map_load(g.scene.current_map, default_map_state);
    }
}
