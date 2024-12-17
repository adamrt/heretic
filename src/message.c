#include <string.h>

#include "font.h"
#include "message.h"
#include "span.h"

usize read_messages(span_t* span, char* out_text) {
    usize length = 0;

    while (span->offset < span->size) {
        u8 byte = span_read_u8(span);

        // These are special characters. We need to handle them differently.
        // https://ffhacktics.com/wiki/Text_Format#Special_Characters
        switch (byte) {
        case 0xFE:
            // This is the message delimiter.
            out_text[length++] = 0xFE;
            break;
        case 0xE0: {
            // Character name stored somewhere else. Hard coding for now.
            const char* name = "Ramza";
            usize len = strlen(name);
            memcpy(&out_text[length], name, len);
            length += len;
            break;
        }
        case 0xE2: {
            u8 delay = span_read_u8(span);
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Delay: %d}", (int)delay);
            memcpy(&out_text[length], buffer, len);
            length += len;
            break;
        }
        case 0xE3: {
            u8 color = span_read_u8(span);
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Color: %d}", (int)color);
            memcpy(&out_text[length], buffer, len);
            length += len;
            break;
        }
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3: {
            // This is a jump to another point in the text section.
            // The next 2 bytes are the jump location and how many bytes to read.
            // https://gomtuu.org/fft/trans/compression/
            u8 second_byte = span_read_u8(span);
            u8 third_byte = span_read_u8(span);
            (void)second_byte; // Unused
            (void)third_byte;  // Unused
            const char* text_jump = "{TextJump}";
            usize len = strlen(text_jump);
            memcpy(&out_text[length], text_jump, len);
            length += len;
            break;
        }
        case 0xF8: {
            // New line/Line break
            const char* close_str = "{LB}";
            usize len = strlen(close_str);
            memcpy(&out_text[length], close_str, len);
            length += len;
            break;
        }
        case 0xFA:
            // This one is not in the list but it is very common between words.
            // It works well as a space though.
            out_text[length++] = ' ';
            break;
        case 0xFF: {
            const char* close_str = "{Close}";
            usize len = strlen(close_str);
            memcpy(&out_text[length], close_str, len);
            length += len;
            break;
        }
        default: {
            if (byte > 0xCF) {
                /* Two-byte character */
                u8 second_byte = span_read_u8(span);
                u16 combined = (second_byte | (byte << 8));
                const char* font_str = font_get_char(combined);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&out_text[length], font_str, len);
                    length += len;
                } else {
                    /* Unknown character */
                    char buffer[64];
                    int len = snprintf(buffer, sizeof(buffer), "{Unknown: 0x%X & 0x%X}", byte, second_byte);
                    memcpy(&out_text[length], buffer, len);
                    length += len;
                }
            } else {
                /* Single-byte character */
                const char* font_str = font_get_char(byte);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&out_text[length], font_str, len);
                    length += len;
                } else {
                    /* Unknown character */
                    out_text[length++] = '^';
                }
            }
            break;
        }
        }
    }

    out_text[length] = '\0';
    return length;
}
