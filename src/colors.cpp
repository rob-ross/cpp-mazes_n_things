//  colors.cpp
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

#include <ncurses.h>

#include "colors.h"
#include "ncutils.h"



namespace mazes_n_things {

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




} // namespace mazes_n_things


