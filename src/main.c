#include "sokol_app.h"
#include "sokol_log.h"

#include "game.h"
#include "gfx.h"

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc) {
        .window_title = "Heretic",
        .width = GFX_WINDOW_WIDTH,
        .height = GFX_WINDOW_HEIGHT,
        .init_cb = game_init,
        .event_cb = game_input,
        .frame_cb = game_update,
        .cleanup_cb = game_shutdown,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
