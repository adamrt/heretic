#include "vm_func.h"

#include "camera.h"
#include "dialog.h"
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

    (void)dialog;
    (void)unit_id;
    (void)portrait_row;
    (void)x;
    (void)y;
    (void)arrow_pos;
    (void)opening;
}

void fn_camera(const instruction_t* instr) {
    camera_t* cam = camera_get_internals();
    scene_t* scene = scene_get_internals();

    f32 x = parse_coord(instr->params[0].value.i16);
    f32 y = -parse_coord(instr->params[1].value.i16);
    f32 z = parse_coord(instr->params[2].value.i16);
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
