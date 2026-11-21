//  colors.h
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.


#ifndef MAZES_N_THINGS_COLORS_H
#define MAZES_N_THINGS_COLORS_H

#include <ncurses.h>

#include <format>
#include <string>

#include "ncutils.h"

namespace mazes_n_things {

// todo experiment with the standard colors.
// 1. can we re-define the 8 standards? We're not supposed to, but it would be interesting to check
//      ANWER: yes, we are able to redefine the first 8 colors. Not sure if we're supposed to or not.
// 2. what's the first color index we can re-define?
//      Answer : 0, apparently! The last color id is COLORS - 1, = 255 on Terminal and Ghosttty
// 3. investigate the default colors, use_default_colors(), assume_default_colors(), negative indexes
//
// ncurses uses "color-value" to mean color id, 0 to COLORS - 1.
// curses assumes that COLOR_BLACK is the default background color for all terminals. That can be changed with
// assume_default_colors.
// start_color  initializes the special color pair 0 to the default foreground and background colors.  No other color
// pairs are initialized.
// a special color id -1 is used in certain extended functions to denote the default color (see use_default_colors.)
// color pair 0 is special; it denotes “no color”.
// Color pair 0 is assumed to be white on black, but is actually whatever the terminal implements before color
// is initialized.  It cannot be modified by the application.
//
// init_pair() : If default colors are used (see use_default_colors(3X)) the upper limit is adjusted to allow
// for extra pairs which use a default color in foreground and/or background.
// As an extension, ncurses allows you to set color pair 0 via the assume_default_colors(3X) routine, or to
// specify the use of default colors (color number -1) if you first invoke the use_default_colors routine.
// In ncurses, init_pair accepts negative foreground and background color arguments to support its
// use_default_colors extension, but only after the latter function has been called.

enum  ColorIndex : short {
    COLOR_FIRST = 10, // sentinel index, not used
    COLOR_GREY,
    COLOR_DARK_GREY,
    COLOR_LIGHT_GREY,
    COLOR_LIGHTER_GREY,
    COLOR_DARKER_GREY,
    COLOR_LAST,
};

enum ColorPairIndex : short {
    CPI_FIRST = 10, // sentinel index, not used
    CPI_MENU_BAR             ,
    CPI_MENU_BAR_HIGHLIGHT   ,
    CPI_MENU_BODY            ,
    CPI_MENU_BODY_HIGHLIGHT  ,
    CPI_MENU_BODY_DISABLED   ,
    CPI_MAIN_WIN             ,
    CPI_LAST,
};


// index constants for color methods. These don't define colors, they just reserve indexes.
// colors get defined in init_app_colors()

constexpr unsigned CP_MENU_BAR = COLOR_PAIR(CPI_MENU_BAR);
constexpr unsigned CP_MENU_BAR_HIGHLIGHT = COLOR_PAIR(CPI_MENU_BAR_HIGHLIGHT);

constexpr unsigned CP_MENU_BODY = COLOR_PAIR(CPI_MENU_BODY);
constexpr unsigned CP_MENU_BODY_HIGHLIGHT = COLOR_PAIR(CPI_MENU_BAR_HIGHLIGHT);
constexpr unsigned CP_MENU_BODY_DISABLED = COLOR_PAIR(CPI_MENU_BODY_DISABLED);
constexpr unsigned CP_MAIN_WIN = COLOR_PAIR(CPI_MAIN_WIN);

struct ColorContent {
    int  color_id{};
    int  r{};
    int  g{};
    int  b{};
    bool is_extended{};

    /**
     * Return the short r,g,b color components for the given color_id.
     * Calls color_content()
     * @param color_id must be < 256
     * @return ColorContent by value, with the components set from color_content()
     */
    static ColorContent content( short color_id) {
        short r_, g_, b_;
        color_content(color_id, &r_, &g_, &b_);
        return { .color_id = color_id, .r = r_, .g = g_, .b = b_, .is_extended = false };
    }

    /**
     * Return the int r,g,b extended color components for the given ext_color_id.
     * Calls extended_color_content()
     * @param ext_color_id must be > 255
     * @return ColorContent by value, with the components set from extended_color_content()
     */
    static ColorContent extendedContent( int ext_color_id) {
        int r_, g_, b_;
        extended_color_content(ext_color_id, &r_, &g_, &b_);
        return { .color_id = ext_color_id, .r = r_, .g = g_, .b = b_, .is_extended = true };
    }

    [[nodiscard]] std::string toString() const {
        const char * type = is_extended ? "ext_content" : "content";
        return std::format("{}({}): {{ r:{}, g:{}, b:{} }}", type, color_id, r, g, b);
    }

    // this was a temp experiment. It's not a good solution. The returned C str lives
    // beyond this method invocation, but the next time this method is called, that previous
    // char* is invalidated. Just use toString().c_str().
    // [[nodiscard]] const char * c_str() const {
    //     thread_local std::string value;
    //     value = toString();
    //     return value.c_str();
    // }
};


// color testing
void log_default_colors(short start_inclusive, short end_inclusive);
void log_extended_colors();
void test_init_colors(ClosedRange range);
void test_init_extended_colors(ClosedRange range);
void log_color_pairs(ClosedRange range);
} // namespace mazes_n_things

#endif //MAZES_N_THINGS_COLORS_H
