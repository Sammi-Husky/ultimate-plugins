#ifndef RAYGUN_PRINTER_H
#define RAYGUN_PRINTER_H

#include <switch_min.h>
#include <ctype.h>
#include <stdarg.h>

#include "framework/useful/useful.h"
#include "framework/acmd_wrapper.h"

#define RAYGUN_LENGTH 8
#define RAYGUN_HEIGHT 6
#define RAYGUN_HORIZ_OFFSET 2

/*
    segment data list : {Z, Y, X, ZRot, Size}
    segment labels :
         _
        |_| from top to top left, clockwise: a->f + g mid +  \|/ from top mid to top left, clockwise: h->m + --two half g's: n, o
        |_|                                                  /|\
*/

const float segment_dict[15][5] = {
        {0, RAYGUN_HEIGHT*2,   0,                    0, 0.25}, // a
        {0, RAYGUN_HEIGHT,     RAYGUN_LENGTH,       90, 0.25}, // b
        {0, 0,                 RAYGUN_LENGTH,       90, 0.25}, // c
        {0, 0,                 0,                    0, 0.25}, // d
        {0, 0,                 0,                   90, 0.25}, // e
        {0, RAYGUN_HEIGHT,     0,                   90, 0.25}, // f
        {0, RAYGUN_HEIGHT,     0,                    0, 0.25}, // g mid
        {0, RAYGUN_HEIGHT,     RAYGUN_LENGTH/2,     90, 0.25}, // h
        {0, RAYGUN_HEIGHT,     RAYGUN_LENGTH/2,     52, 0.2},  // i
        {0, RAYGUN_HEIGHT,     RAYGUN_LENGTH/2,    -52, 0.2},  // j
        {0, 0,                 RAYGUN_LENGTH/2,     90, 0.25}, // k
        {0, RAYGUN_HEIGHT/2,   RAYGUN_LENGTH*3/16,  52, 0.2},  // l
        {0, RAYGUN_HEIGHT*3/2, RAYGUN_LENGTH*3/16, -52, 0.2},  // m
        {0, RAYGUN_HEIGHT,     0,                    0, 0.15}, // n
        {0, RAYGUN_HEIGHT,     RAYGUN_LENGTH/2,      0, 0.15}, // o
    };

/* 
    Segments making up each character, each index corresponding to:
    'A' through 'Z', '0' through '9', ' ', '-', '+', '#' (where '#' is all segments)
*/
const char* alphabet[] = {   
    "abcefg",
    "adefijn",
    "adef",
    "eflm",
    "adefn",
    "aefn",
    "acdefo",
    "bcefg",
    "adhk",
    "bcd",
    "efnij",
    "def",
    "bcefim",
    "bcefjm",
    "abcdef",
    "abefg",
    "abcdefj",
    "aefijn",
    "acdfg",
    "ahk",
    "bcdef",
    "efil",
    "bcefjl",
    "ijlm",
    "ikm",
    "adil",
    "abcdef",
    "ef",
    "abdeg",
    "abcdg",
    "bcfg",
    "acdfg",
    "acdefg",
    "abc",
    "abcdefg",
    "abcdfg",
    "",
    "g",
    "ghk",
    "abcdefhijklmno",
};

// Each index is a segment's corresponding flipped segment, for when facing left
const char segment_rev[15] = {
    'a',
    'f',
    'e',
    'd',
    'c',
    'b',
    'g',
    'h',
    'm',
    'l',
    'k',
    'j',
    'i',
    'o',
    'n',
};

void show_segment(u64 battle_object_module_accessor, float z, float y, float x, float zrot, float size) {
    Hash40 raygunShot = { .hash = 0x11e470b07fLL };
    Hash40 top = { .hash = 0x031ed91fcaLL };

    Vector3f pos = { .x = x, .y = y, .z = z };
    Vector3f rot = { .x = 0, .y = 90, .z = zrot };
    Vector3f random = { .x = 0, .y = 0, .z = 0 };

    app::lua_bind::EffectModule::req_on_joint(battle_object_module_accessor, raygunShot.hash, top.hash, 
        &pos, &rot, size, 
        &random, &random, 
        0, 0, 0, 0);
}

int alphabet_index(char to_print) {
    if (to_print >= 'A' && to_print <= 'Z')
        return to_print - 'A';
    else if (to_print >= '0' && to_print <= '9')
        return to_print - '0' + 'Z' - 'A' + 1;
    else if (to_print == ' ')
        return 36;
    else if (to_print == '-')
        return 37;
    else if (to_print == '+')
        return 38;
    else if (to_print == '#')
        return 39;
    else
        return -1;
}

void print_char( u64 module_accessor, char to_print, int line_num, float horiz_offset, float facing_left) {
    int alph_index = alphabet_index(to_print);
    if (alph_index < 0 || alph_index >= 40)
        return;
    const char* segment_str = alphabet[alph_index];
    int num_segments = strlen(segment_str);

    float lineOffset = 40 - (line_num * 16);

    for (int i = 0; i < num_segments; i++) {
        const float* segment;
        int index = segment_str[i] - 'a';

        if (facing_left == -1) {
            index = segment_rev[index] - 'a';
		}
        segment = segment_dict[index];

        float z = segment[0];
        float y = segment[1] + lineOffset;
        float x = segment[2] + horiz_offset;
        float zrot = segment[3];

        if (facing_left == -1)
            zrot *= -1;

        float size = segment[4];
        show_segment(module_accessor, z, y, x, zrot, size);
    }
}

void print_string( u64 module_accessor, const char* format, ...) {

    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    // Delete any previous strings
    Hash40 raygunShot = { .hash = 0x11e470b07fLL };
    app::lua_bind::EffectModule::kill_kind(module_accessor, raygunShot.hash, 0, 1);

    int line_num = 0;
    float horiz_offset = 0;
    int char_num = 0;

    float facing_left = app::lua_bind::PostureModule::lr(module_accessor);

    if (strlen(buffer) <= 8 && strchr(buffer, '\n') == NULL) {
        line_num = 1;
	}
    horiz_offset = 0;
    char_num = 0;
    for (size_t i = 0; i < strlen(buffer); i++) {
        char curr_char = buffer[i];
        if (curr_char == '\n') {
            horiz_offset = 0;
            char_num = 0;
            line_num++;
            continue;
        }

        print_char(module_accessor, toupper(curr_char), line_num, horiz_offset, facing_left);

        char_num++;
        // short characters
        if (curr_char == 'D' || curr_char == '1') {
            horiz_offset += facing_left * (RAYGUN_LENGTH/2 + 3);
        } else {
            horiz_offset += facing_left * (RAYGUN_LENGTH + 3);
		}

        if (char_num > 8) {
            horiz_offset = 0;
            char_num = 0;
            line_num++;
        }
    }
}

#endif // RAYGUN_PRINTER_H
