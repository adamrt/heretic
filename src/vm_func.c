#include "vm_func.h"

#include "camera.h"
#include "dialog.h"
#include "gfx.h"
#include "gfx_sprite.h"
#include "message.h"
#include "parse.h"
#include "scene.h"
#include "transition.h"
#include "vm.h"

void fn_display_message(const instruction_t* instr) {
    scene_t* scene = scene_get_internals();

    (void)instr->params[0].value.u8; // unused, typically 0x10
    dialog_t dialog = parse_dialog(instr->params[1].value.u8);
    u16 message_id = instr->params[2].value.u16;
    u8 unit_id = instr->params[3].value.u8;
    (void)instr->params[4].value.u8; // unused, always 0x00
    u8 portrait_row = instr->params[5].value.u8;
    i16 x = instr->params[6].value.i16;
    i16 y = instr->params[7].value.i16;
    i16 arrow_pos = instr->params[8].value.i16;
    dialog_opening_t opening = parse_dialog_opening(instr->params[9].value.u8);

    char text[512];
    message_by_index(scene->event.messages, message_id, text);

    /* printf("-- Messaage: %d %d @ %dx%d --\n", message_id, unit_id, x, y); */
    /* printf("Dialog: type: %d, arrow: %d, alignment: %d\n", dialog.type, dialog.arrow, dialog.alignment); */
    /* printf("Opening: speed: %d, remboucing: %d, darken: %d, arrow: %d\n", opening.speed, opening.remove_bouncing, opening.darken, opening.toggle_arrow_right); */
    /* printf("Text: %s\n", text); */

    int speed = 10;
    if (opening.speed == DIALOG_SPEED_PLUS_50) {
        speed = 15;
    } else if (opening.speed == DIALOG_SPEED_MINUS_50) {
        speed = 5;
    }

    x = GFX_RENDER_WIDTH / 2 + x;
    y = GFX_RENDER_HEIGHT / 2 + y;

    texture_t texture = sprite_get_paletted_texture(F_EVENT__FRAME_BIN, 0);
    sprite2d_t* sprite = &scene->sprite2ds[0];
    *sprite = gfx_sprite2d_create(texture, (vec2s) { { 0.0f, 0.0f } }, (vec2s) { { 32, 32 } }, x, y, 20.0f);

    transition_add(instr->opcode, &sprite->transform.scale, 20.0f, 80.0f, speed);
    transition_add(instr->opcode, &sprite->transform.scale, 20.0f, 40.0f, speed);

    sprite2d_t* portrait = &scene->sprite2ds[1];
    texture_t portrait_texture = sprite_get_evtface_bin_texture(portrait_row + 1, 0);
    *portrait = gfx_sprite2d_create(portrait_texture, (vec2s) { { unit_id * 32.0f, 0.0f } }, (vec2s) { { 32, 48 } }, x, y, 20.0f);

    (void)dialog;
    (void)arrow_pos;
    (void)opening;
}

void fn_camera(const instruction_t* instr) {
    camera_t* cam = camera_get_internals();
    scene_t* scene = scene_get_internals();

    f32 x = parse_coord(COORD_X, instr->params[0].value.i16);
    f32 y = parse_coord(COORD_Y, instr->params[1].value.i16);
    f32 z = parse_coord(COORD_Z, instr->params[2].value.i16);
    f32 pitch_rad = parse_rad(instr->params[3].value.i16);
    f32 maprot_rad = parse_rad(instr->params[4].value.i16);
    f32 yaw_rad = parse_rad(instr->params[5].value.i16);
    f32 zoom = parse_zoom(instr->params[6].value.i16);
    f32 duration = (f32)instr->params[7].value.i16;

    transition_add(instr->opcode, &cam->position.x, cam->position.x, x, duration);
    transition_add(instr->opcode, &cam->position.y, cam->position.y, y, duration);
    transition_add(instr->opcode, &cam->position.z, cam->position.z, z, duration);
    transition_add(instr->opcode, &cam->yaw_rad, cam->yaw_rad, yaw_rad, duration);
    transition_add(instr->opcode, &cam->pitch_rad, cam->pitch_rad, pitch_rad, duration);
    transition_add(instr->opcode, &cam->zoom, cam->zoom, zoom, duration);
    transition_add(instr->opcode, &scene->model.transform.rotation.y, scene->model.transform.rotation.y, maprot_rad, duration);
}

void fn_wait_for_instruction(const instruction_t* instr) {
    waittype_e waittype = instr->params[0].value.u8;
    vm_wait(waittype);
}

void fn_warp_unit(const instruction_t* instr) {
    u8 unit_id = instr->params[0].value.u8;
    u8 unused = instr->params[1].value.u8; // always 0x00
    u8 tile_x = instr->params[2].value.u8;
    u8 tile_y = instr->params[3].value.u8;
    u8 elevation = instr->params[4].value.u8; // 0x00 lower, 0x01 upper
    u8 facing = instr->params[5].value.u8;    // 0x00 south, 0x01 west, 0x02 north, 0x03 east

    // FIXME: unit_id is probably not what we want to use here, but we aren't working
    // with units yet so ignore for now.
    if (unit_id >= 100) {
        printf("Invalid unit id %d\n", unit_id);
        return;
    }

    texture_t texture = sprite_get_paletted_texture(F_EVENT__UNIT_BIN, 0);
    sprite3d_t* sprite = &scene_get_internals()->sprite3ds[unit_id];
    transform3d_t transform = {
        .translation = { { tile_x * 24.0f, elevation * 5.0f, tile_y * 24.0f } },
        .rotation = { { 0.0f, facing * 90.0f, 0.0f } },
        .scale = { { 15.0f, 15.0f, 15.0f } }
    };
    *sprite = gfx_sprite3d_create(texture, (vec2s) { { unit_id * 32.0f, 0.0f } }, (vec2s) { { 32.0f, 40.0f } }, transform);

    (void)unused;
}
