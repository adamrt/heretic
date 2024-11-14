#include "gfx.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear.h"
#include "src/nuklear_internal.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"

#include "util/sokol_nuklear.h"

#include "bin.h"
#include "game.h"
#include "gui.h"

static void gui_draw(void);

void gui_init(void)
{
    snk_setup(&(snk_desc_t) {
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });
}

void gui_update(void)
{
    gui_draw();
    snk_render(sapp_width(), sapp_height());
}

bool gui_input(const sapp_event* event)
{
    return snk_handle_event(event);
}

void gui_shutdown(void)
{
    snk_shutdown();
}

void render_dropdown(struct nk_context* ctx)
{
    const int num_items = 500;
    scenario_t scenario = game.fft.scenarios.scenarios[game.scene.current_scenario];

    char selected_buffer[64];
    snprintf(selected_buffer, 64, "%d %s", scenario.id, map_list[scenario.map_id].name);

    // Begin the combo (dropdown) box
    if (nk_combo_begin_label(ctx, selected_buffer, nk_vec2(300, 200))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        // Loop through items and create selectable labels
        for (int i = 0; i < num_items; ++i) {

            scenario_t scenario = game.fft.scenarios.scenarios[i];
            char item_buffer[64];
            snprintf(item_buffer, 64, "%d %s", scenario.id, map_list[scenario.map_id].name);

            if (nk_combo_item_label(ctx, item_buffer, NK_TEXT_LEFT)) {
                game.scene.current_scenario = i;
                game_load_scenario(game.scene.current_scenario);
            }
        }

        // End the combo (dropdown) box
        nk_combo_end(ctx);
    }
}

static void draw_scenarios(struct nk_context* ctx)
{
    if (nk_begin(ctx, "Scenarios", nk_rect(10, GFX_DISPLAY_HEIGHT - 250, 1270, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 15, 2);
        for (int i = 0; i < game.fft.scenarios.count; i++) {
            scenario_t* scenario = &game.fft.scenarios.scenarios[i];

            char buffer_id[64];
            snprintf(buffer_id, 64, "Scenario %d, ID: %d, Map: %s", i, scenario->id, map_list[scenario->map_id].name);

            char weather_name[12];
            weather_str(scenario->weather, weather_name);
            char time_name[8];
            time_str(scenario->time, time_name);
            char buffer_weather[40];
            snprintf(buffer_weather, 40, "Weather: %s, Time: %s, ENTD: %d", weather_name, time_name, scenario->entd_id);

            nk_label(ctx, buffer_id, NK_TEXT_LEFT);
            nk_label(ctx, buffer_weather, NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}

static void gui_draw(void)
{
    struct nk_context* ctx = snk_new_frame();

    draw_scenarios(ctx);

    if (nk_begin(ctx, "Heretic", nk_rect(10, 25, 350, 550), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 25, 1);

        render_dropdown(ctx);

        scenario_t scenario = game.fft.scenarios.scenarios[game.scene.current_scenario];
        map_desc_t map = map_list[scenario.map_id];

        char mapbuffer[64];
        snprintf(mapbuffer, 64, "Map %d: %s", map.id, map.name);
        nk_label(ctx, mapbuffer, NK_TEXT_LEFT);

        char weather_name[12];
        weather_str(scenario.weather, weather_name);
        char time_name[8];
        time_str(scenario.time, time_name);
        char condbuffer[40];
        snprintf(condbuffer, 40, "Weather: %s, Time: %s", weather_name, time_name);
        nk_label(ctx, condbuffer, NK_TEXT_LEFT);

        nk_bool centered = game.scene.center_model;
        nk_checkbox_label(ctx, "Centered", &centered);
        game.scene.center_model = centered;

        for (int i = 0; i < MESH_MAX_LIGHTS; i++) {
            light_t* light = &game.scene.model.mesh.lighting.lights[i];
            if (!light->valid) {
                continue;
            }

            char buffer[64];
            snprintf(buffer, 64, "Light %d", i);
            nk_label(ctx, buffer, NK_TEXT_LEFT);

            struct nk_colorf light_color_nk = { light->color.r, light->color.g, light->color.b, light->color.a };
            if (nk_combo_begin_color(ctx, nk_rgba_f(light->color.r, light->color.g, light->color.b, light->color.a), nk_vec2(200, 400))) {
                nk_layout_row_dynamic(ctx, 120, 1);

                light_color_nk = nk_color_picker(ctx, light_color_nk, NK_RGBA);

                nk_layout_row_dynamic(ctx, 25, 2);
                nk_layout_row_dynamic(ctx, 25, 2);
                nk_option_label(ctx, "RGBA", true);
                nk_layout_row_dynamic(ctx, 25, 1);

                light_color_nk.r = nk_propertyf(ctx, "#R:", 0, light_color_nk.r, 1.0f, 0.01f, 0.005f);
                light_color_nk.g = nk_propertyf(ctx, "#G:", 0, light_color_nk.g, 1.0f, 0.01f, 0.005f);
                light_color_nk.b = nk_propertyf(ctx, "#B:", 0, light_color_nk.b, 1.0f, 0.01f, 0.005f);
                light_color_nk.a = nk_propertyf(ctx, "#A:", 0, light_color_nk.a, 1.0f, 0.01f, 0.005f);

                light->color = (vec4s) { { light_color_nk.r, light_color_nk.g, light_color_nk.b, light_color_nk.a } };
                nk_combo_end(ctx);
            }

            char posbuffer[64];
            snprintf(posbuffer, 64, "%.2f, %.2f, %.2f", light->direction.x, light->direction.y, light->direction.z);
            if (nk_combo_begin_label(ctx, posbuffer, nk_vec2(200, 200))) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_float(ctx, "#X:", -30.0f, &light->direction.x, 30.0f, 1, 0.5f);
                nk_property_float(ctx, "#Y:", -30.0f, &light->direction.y, 30.0f, 1, 0.5f);
                nk_property_float(ctx, "#Z:", -30.0f, &light->direction.z, 30.0f, 1, 0.5f);
                nk_combo_end(ctx);
            }
        }

        nk_label(ctx, "Ambient Color", NK_TEXT_LEFT);
        vec4s* ambient_color = &game.scene.model.mesh.lighting.ambient_color;
        struct nk_colorf ambient_color_nk = { ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a };
        if (nk_combo_begin_color(ctx, nk_rgba_f(ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a), nk_vec2(200, 400))) {
            nk_layout_row_dynamic(ctx, 120, 1);

            ambient_color_nk = nk_color_picker(ctx, ambient_color_nk, NK_RGBA);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_option_label(ctx, "RGBA", true);
            nk_layout_row_dynamic(ctx, 25, 1);

            ambient_color_nk.r = nk_propertyf(ctx, "#R:", 0, ambient_color_nk.r, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.g = nk_propertyf(ctx, "#G:", 0, ambient_color_nk.g, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.b = nk_propertyf(ctx, "#B:", 0, ambient_color_nk.b, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.a = nk_propertyf(ctx, "#A:", 0, ambient_color_nk.a, 1.0f, 0.01f, 0.005f);

            *ambient_color = (vec4s) { { ambient_color_nk.r, ambient_color_nk.g, ambient_color_nk.b, ambient_color_nk.a } };
            nk_combo_end(ctx);
        }

        nk_label(ctx, "Ambient Strength", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0, &game.scene.model.mesh.lighting.ambient_strength, 3.0f, 0.1f);
    }
    nk_end(ctx);

    return;
}
