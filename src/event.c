#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "event.h"
#include "font.h"
#include "instruction.h"
#include "memory.h"
#include "span.h"

static void read_messages(span_t* span, char* event_text, int* out_len);

event_t read_event(span_t* span) {
    event_t event = { 0 };

    u32 text_offset = span_read_u32(span);
    bool valid = text_offset != 0xF2F2F2F2;
    if (!valid) {
        return event;
    }

    // usize text_size = EVENT_SIZE - text_offset;
    usize code_size = text_offset - 4;

    event.valid = valid;
    memcpy(event.data, span->data, EVENT_SIZE);

    span_t msgspan = { .data = event.data, .offset = 0 };
    event.messages = memory_allocate(EVENT_MESSAGES_LEN);
    read_messages(&msgspan, event.messages, &event.messages_len);

    span_t instspan = { .data = event.data + 4, .offset = 0 };
    event.instructions = memory_allocate(INSTRUCTION_MAX * sizeof(instruction_t));
    event.instruction_count = read_instructions(&instspan, code_size, event.instructions);

    return event;
}

static void read_messages(span_t* span, char* event_text, int* out_len) {
    usize event_text_len = 0;

    usize text_offset = span_read_u32(span);
    usize text_size = 8192 - text_offset;

    usize i = 0;
    while (i < text_size) {
        u8 byte = span_readat_u8(span, text_offset + i);

        // These are special characters. We need to handle them differently.
        // https://ffhacktics.com/wiki/Text_Format#Special_Characters
        switch (byte) {
        case 0xFE:
            // This is the message delimiter.
            event_text[event_text_len++] = 0xFE;
            i++;
            break;
        case 0xE0: {
            // Character name stored somewhere else. Hard coding for now.
            const char* name = "Ramza";
            usize len = strlen(name);
            memcpy(&event_text[event_text_len], name, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xE2: {
            i++;
            if (i >= text_size)
                break;
            u8 delay = span_readat_u8(span, text_offset + i);
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Delay: %d}", (int)delay);
            memcpy(&event_text[event_text_len], buffer, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xE3: {
            i++;
            if (i >= text_size)
                break;
            u8 color = span_readat_u8(span, text_offset + i);
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Color: %d}", (int)color);
            memcpy(&event_text[event_text_len], buffer, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3: {
            // This is a jump to another point in the text section.
            // The next 2 bytes are the jump location and how many bytes to read.
            // https://gomtuu.org/fft/trans/compression/
            i++;
            if (i + 1 >= text_size)
                break;
            u8 second_byte = span_readat_u8(span, text_offset + i);
            u8 third_byte = span_readat_u8(span, text_offset + i + 1);
            (void)second_byte; // Unused
            (void)third_byte;  // Unused
            const char* text_jump = "{TextJump}";
            usize len = strlen(text_jump);
            memcpy(&event_text[event_text_len], text_jump, len);
            event_text_len += len;
            i += 2;
            break;
        }
        case 0xF8:
            event_text[event_text_len++] = '\n';
            i++;
            break;
        case 0xFA:
            // This one is not in the list but it is very common between words.
            // It works well as a space though.
            event_text[event_text_len++] = ' ';
            i++;
            break;
        case 0xFF: {
            const char* close_str = "{Close}";
            usize len = strlen(close_str);
            memcpy(&event_text[event_text_len], close_str, len);
            event_text_len += len;
            i++;
            break;
        }
        default: {
            if (byte > 0xCF) {
                /* Two-byte character */
                if (i + 1 >= text_size) {
                    /* Can't read second byte */
                    i++;
                    break;
                }
                u8 second_byte = span_readat_u8(span, text_offset + i + 1);
                u16 combined = (second_byte | (byte << 8));
                const char* font_str = font_get_char(combined);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&event_text[event_text_len], font_str, len);
                    event_text_len += len;
                    i += 2;
                } else {
                    /* Unknown character */
                    char buffer[64];
                    int len = snprintf(buffer, sizeof(buffer), "{Unknown: 0x%X & 0x%X}", byte, second_byte);
                    memcpy(&event_text[event_text_len], buffer, len);
                    event_text_len += len;
                    i++;
                }
            } else {
                /* Single-byte character */
                const char* font_str = font_get_char(byte);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&event_text[event_text_len], font_str, len);
                    event_text_len += len;
                    i++;
                } else {
                    /* Unknown character */
                    event_text[event_text_len++] = '?';
                    i++;
                }
            }
            break;
        }
        }
    }

    event_text[event_text_len] = '\0';
    *out_len = event_text_len;
}
