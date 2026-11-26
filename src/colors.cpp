//  colors.cpp
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

#include <ncurses.h>

#include "colors.h"
#include "ncutils.h"
#include <stdint.h>
#include <stdlib.h>


namespace mazes_n_things {

// returns true if the env reports that it supports truecolor
bool has_truecolor() {
    const char *ct = getenv("COLORTERM");
    return ct && (strcmp(ct, "truecolor") == 0 || strcmp(ct, "24bit") == 0) && COLORS >= 16777216;
}

// -----------------------------------------------------------------
//   Color Tests
// -----------------------------------------------------------------

// write the first 8 color id components to the logfile.
void log_default_colors(short start_inclusive, short end_inclusive) {
    const auto to_print = static_cast<short>(end_inclusive - start_inclusive + 1);
    log_printlnf("");
    log_printf("log_default_colors: first %hd default colors are:\n", to_print);
    for (short i{start_inclusive};  i <= end_inclusive; i++ ) {
        ColorContent c = ColorContent::content(i);
        log_printlnf("%s", c.toString().c_str());
    }

}

// write the first 257 color ids via extended_color_component()
// we'll see if they are the same for the first 256 color ids.
void log_extended_colors() {
    const int to_print = 16;
    log_printlnf("");
    log_printf("log_extended_colors: first %d default extended colors are:\n", to_print);
    for (int i{};  i < to_print; i++ ) {
        ColorContent c = ColorContent::extendedContent(i);
        log_printlnf("%s", c.toString().c_str());
    }
}

void log_color_pairs(ClosedRange range) {
    log_printlnf("");
    log_printf("log_color_pairs: first %d color pairs are:\n", range.length() );
    for (int i{range.start};  i < range.length(); i++ ) {
        int f, b;
        int result = extended_pair_content( i, &f, &b);
        ColorContent fore = ColorContent::extendedContent(f);
        ColorContent back = ColorContent::extendedContent(b);
        const char * msg = result == OK ? "OK" : "ERR";

        log_printlnf("pair id:%d :  %s: fore:%s, back:%s",
            i, msg, fore.toString().c_str(), back.toString().c_str());
    }
}


constexpr int COLOR_COMPONENT_MIN = 0;
constexpr int COLOR_COMPONENT_MAX = 1000;

// helper to simplify casting int to short
static short random_color_component() {
    return static_cast<short>(
        random_int(COLOR_COMPONENT_MIN, COLOR_COMPONENT_MAX)
    );
}

// We'll try to assign 256 colors via init_colors() and log the results
void test_init_colors(ClosedRange range) {
    const auto to_assign = static_cast<short>(range.end - range.start + 1);
    log_printlnf();
    log_printf("test_assign_colors: assigning %hd colors:\n", to_assign);
    const auto start = static_cast<short>(range.start);
    const auto end = static_cast<short>(range.end);
    for (short i{ start };  i <= end; i++ ) {
        short r{random_color_component()};
        short g{random_color_component()};
        short b{random_color_component()};
        int result = init_color(i, r, g, b);
        const char * msg = result == OK ? "OK" : "ERR";
        ColorContent c = ColorContent::content(i);
        log_printlnf("init_color(id:%hd, r:%hd, g:%hd, b:%hd): ret=%s -  %s",
            i, r, g, b, msg, c.toString().c_str());
    }
}

void test_init_extended_colors(ClosedRange range) {
    const auto to_assign = range.end - range.start + 1;
    log_printlnf();
    log_printf("test_assign_colors: assigning %d colors:\n", to_assign);

    for (int i{ range.start };  i <= range.end; i++ ) {
        int r{ random_int(COLOR_COMPONENT_MIN, COLOR_COMPONENT_MAX) };
        int g{ random_int(COLOR_COMPONENT_MIN, COLOR_COMPONENT_MAX) };
        int b{ random_int(COLOR_COMPONENT_MIN, COLOR_COMPONENT_MAX) };
        int result = init_extended_color(i, r, g, b);
        const char * msg = result == OK ? "OK" : "ERR";
        ColorContent c = ColorContent::extendedContent(i);
        log_printlnf("init_extended_color(id:%d, r:%d, g:%d, b:%d): ret=%s -  %s",
            i, r, g, b, msg, c.toString().c_str());
    }
}

// color_content() just reports ncurses' internal color cache. It doesn't qery terminfo
// to see what colors it has set. xterm-256 has all 256 color indices populated already, but
// color_content doesn't return these - only the first 8 standard colors.
// the following methods allow you to determine what the colors at each index actually are
// at program start.

/*

    Notice how rgb_to_xterm256() compares Euclidean RGB distance against both the 6×6×6 cube and
    the 24-step grayscale ramp. Because the 6×6×6 cube only has 6 gray points (0, 95, 135, 175, 215, 255),
    checking the grayscale ramp prevents neutral grays (like #262626 or #4e4e4e) from getting tinted or
    overly quantized on 256-color terminals like Terminal.app.

    Next steps:
    • How to wrap this into a unified color pair allocator that uses init_extended_pair in Ghostty
    and automatically falls back to rgb_to_xterm256 + init_pair in Terminal.app?
    • How ncurses handles use_default_colors() (color -1) if you want transparent terminal backgrounds
    alongside 24-bit or 256 colors?

*/
typedef struct {
    uint8_t r, g, b;
} RGB;

// Convert any xterm-256 index (0-255) to its standard 8-bit RGB values
RGB xterm256_to_rgb(uint8_t index) {
    // 0-15: Standard xterm defaults (note: terminals may theme these)
    static const RGB ansi16[16] = {
        {0, 0, 0},       {128, 0, 0},     {0, 128, 0},     {128, 128, 0},
        {0, 0, 128},     {128, 0, 128},   {0, 128, 128},   {192, 192, 192},
        {128, 128, 128}, {255, 0, 0},     {0, 255, 0},     {255, 255, 0},
        {0, 0, 255},     {255, 0, 255},   {0, 255, 255},   {255, 255, 255}
    };
    if (index < 16) {
        return ansi16[index];
    }

    // 16-231: 6x6x6 RGB color cube
    if (index < 232) {
        static const uint8_t levels[6] = {0, 95, 135, 175, 215, 255};
        uint8_t n = index - 16;
        uint8_t r = n / 36;
        uint8_t g = (n % 36) / 6;
        uint8_t b = n % 6;
        return {levels[r], levels[g], levels[b]};
    }

    // 232-255: 24-step grayscale ramp (8, 18, 28, ..., 238)
    uint8_t gray = 8 + (index - 232) * 10;
    return {gray, gray, gray};
}

// Snap a single 8-bit channel (0-255) to the nearest 6x6x6 cube index (0-5)
static uint8_t nearest_cube_idx(uint8_t v) {
    if (v < 48)  return 0; // closer to 0 than 95
    if (v < 115) return 1; // closer to 95 than 135
    return (v - 35) / 40;  // 135, 175, 215, 255 are evenly spaced by 40
}

// Convert any 24-bit RGB color to the closest fixed xterm-256 index (16-255)
uint8_t rgb_to_xterm256(uint8_t r, uint8_t g, uint8_t b) {
    static const uint8_t levels[6] = {0, 95, 135, 175, 215, 255};

    // Candidate 1: Nearest point in the 6x6x6 color cube (16-231)
    uint8_t ri = nearest_cube_idx(r);
    uint8_t gi = nearest_cube_idx(g);
    uint8_t bi = nearest_cube_idx(b);
    uint8_t cube_idx = 16 + (36 * ri) + (6 * gi) + bi;

    int dr = r - levels[ri];
    int dg = g - levels[gi];
    int db = b - levels[bi];
    int cube_dist = dr * dr + dg * dg + db * db;

    // Candidate 2: Nearest step in the grayscale ramp (232-255)
    int avg = (r + g + b) / 3;
    int gray_step = (avg - 8 + 5) / 10;
    if (gray_step < 0)  gray_step = 0;
    if (gray_step > 23) gray_step = 23;
    uint8_t gray_idx = 232 + gray_step;

    int gv = 8 + gray_step * 10;
    int gray_dist = (r - gv) * (r - gv) + (g - gv) * (g - gv) + (b - gv) * (b - gv);

    return (gray_dist < cube_dist) ? gray_idx : cube_idx;
}


} // namespace mazes_n_things


