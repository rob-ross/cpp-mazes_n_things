//  entry_point_01.cpp
//
//  Created by Rob Ross on 11/19/26.
//
//  Copyright (c) 2026.  All rights reserved.

#include <ncurses.h>
#include <panel.h>

#include <iostream>
#include <clocale>

#include "ncutils.h"
#include "colors.h"
#include "widget.h"

namespace mazes_n_things {
struct TextRange;
}

namespace entry_point_1 {
using namespace mazes_n_things;

void init_app_colors() {


    // menu colors r,g,b in range [0,1000]
    // note that the color id cannot be > 255 when calling init_color()
    init_color( COLOR_DARKER_GREY, 150, 150, 150 );
    init_color( COLOR_DARK_GREY,  250, 250, 250 );
    init_color( COLOR_GREY, 500, 500, 500 );
    init_color( COLOR_LIGHT_GREY, 750, 750, 750 );
    init_color( COLOR_LIGHTER_GREY, 900, 900, 900 );


    // main window COLOR_PAIR
    init_pair(CPI_MAIN_WIN, COLOR_BLACK, COLOR_MAGENTA);

    // menu COLOR_PAIRs
    init_pair(CPI_MENU_BAR, COLOR_WHITE, COLOR_DARK_GREY);
    init_pair(CPI_MENU_BAR_HIGHLIGHT, COLOR_LIGHTER_GREY, COLOR_GREY);

    init_pair(CPI_MENU_BODY, COLOR_WHITE, COLOR_DARKER_GREY);
    init_pair(CPI_MENU_BODY_HIGHLIGHT, COLOR_WHITE, COLOR_BLUE);
    init_pair(CPI_MENU_BODY_DISABLED, COLOR_GREY , COLOR_DARKER_GREY);


}

enum class ColorState { NONE, STARTED, CAN_CHANGE };

struct GlobalState {
    ColorState color_state{ColorState::NONE};
};
// attempt to initialize ncursor's color system
bool try_start_color(GlobalState * gstate) {
    if (!has_colors() || start_color() != OK) {
        return false;
    }
    if ( !can_change_color()) {
        gstate->color_state = ColorState::STARTED;
        return true;
    }
    gstate->color_state = ColorState::CAN_CHANGE;
    return true;
}

void test_colors() {
    start_color();
    // captures terminal default colors at startup as "-1". Then assigns these to COLOR_PAIR(0) as fore=-1,back=-1
    // then anywhere you use this color pair, you use those default terminal colors. In Mac Terminal, it supports
    // alpha transparency, so if you have the opacity set below 100% in your terminal settings, when you use these
    // default colors, say as the bgkgrnd in a window, you get transparent windows!
    // Doesn't work in Ghosttty though.
    // assume_default_colors(-1,-1);

    log_printf("COLORS=%d, COLOR_PAIRS=%d, \n", COLORS, COLOR_PAIRS);
    // Terminal (xterm-256): COLORS=256, COLOR_PAIRS=65536
    // Ghosttty (xterm-256): COLORS=256, COLOR_PAIRS=32767
    // Note that calling either init_color or init_extended_color share the color id.
    // They map to the same color index. They just use different precision (short vs int) to
    // define that color id.
    //
    //
    // get colors for COLOR_PAIR(0), and the default foreground/background
    log_printf("%s\n", ColorContent::content(0).toString().c_str());
    short fore, back;
    pair_content(0, &fore, &back);
    log_printf("pair_content(0): fore:%d, back:%d \n", fore, back);

    log_default_colors(0,255);
    // test_init_colors({.start = 255, .end = 300 });
    // interesting: starting at 128, it succeeds until 256 then init_extended_color() fails!
    // hypothesis: if you start initing colors below 256, you can't init above 256.
    // test_init_extended_colors({.start = 128, .end = 385 });
    // if this is the case, the line below will succeed twice (254, 255) then fail from 256-260
    // test_init_extended_colors({.start = 254, .end = 260 });
    // YES they hypothesis holds. This is indeed what happens. What if we only ever use
    // color ids > 255?
    // test_init_extended_colors({.start = 256, .end = 260 });
    // fails at 256
    // CONCLUSION : you can only use color ids from 0 to the value of COLORS - 1, regardless of
    // whether you call init_color() or init_extended_color().

    // now we test the max r,g,b values that can be supplied to this function
    // test_init_extended_colors({.start = 0, .end = 256 });
    // init_extended_colors() still only takes rgb values between 0 and 1000.
    // log_color_pairs( {.start = 0, .end = 3} );
}

constexpr int MENUBAR_HEIGHT = 1; // 1 row tall. width is screen width.
constexpr int STATUS_REGION_HEIGHT = 3; // height is 2 but we need 1 row for border

Layout create_layout() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WindowPtr root_win{newwin(rows, cols, 0, 0)}; // full screen window (same size as terminal window)
    WINDOW* root_window = root_win.get();
    keypad(root_window, true);
    WindowPtr menubar_win{derwin(root_window, 1, cols, 0, 0)};
    WindowPtr status_win{derwin(root_window, STATUS_REGION_HEIGHT, cols,
                               rows - STATUS_REGION_HEIGHT - 1, 0)};

    int mainw_height = rows - MENUBAR_HEIGHT - STATUS_REGION_HEIGHT;
    int mainw_y = MENUBAR_HEIGHT;

    WindowPtr main_win{derwin(root_window, mainw_height, cols, mainw_y, 0)};

    // create MenuBarModel
    std::vector<TextRange> titles{
        {.text = "App",  .range = { .start =  2, .end =  4 } },
        {.text = "File", .range = { .start =  7, .end = 10 } },
        {.text = "Edit", .range = { .start = 13, .end = 16 } },
        {.text = "View", .range = { .start = 19, .end = 22 } },
        {.text = "Help", .range = { .start = 25, .end = 28 } }
    };
    MenuBarModel mbm{ .titles = titles };
    MenuBarView  mbv{ };
    MenuBarController mbc{};

    using MenuBarWidget = Widget<MenuBarModel, MenuBarView, MenuBarController>;
    auto mb_widget = std::make_unique<MenuBarWidget>(mbm, mbv, mbc);

    Layout layout;
    layout.root_window = std::move(root_win);
    Bounds menu_bounds = window_bounds(menubar_win.get());
    layout.regions.emplace_back(
        std::move(menubar_win),
        std::move(mb_widget),
        menu_bounds
    );


    return layout;
}

void main_event_loop(Layout& layout) {
    WINDOW * root_win = layout.root_window.get();
    while ( true ) {
        wint_t ch;
        int ret = wget_wch(root_win, &ch); // blocks
        if (ret != ERR ) {
            log_printlnf("ret = '%o'", ch);
            if (ret == OK && ch == CTRL('q')) {
                break; // quit event loop
            }

            // if the event is a keyboard event, forward it to the currently focused Region.
            // if the Region can't handle it, we try to handle it here. Otherwise, the event is unhandled.
            // if the event is a mouse button event, we see if we can handle it first, or if we need to
            // forward it to a Region, not necessarily the focused region. A mouse click in a non-focused Region
            // may cause that Region to be focused.
            if (ret == OK ) {
                if ( ch == KEY_MOUSE) {
                    MEVENT event;
                    if (getmouse(&event) == OK) {
                        // determine if a mouse click is in a Region, and if so, forward that event
                    }
                } else {

                }
            } else if (ret == KEY_CODE_YES) {

            }
        }

    }
}

int main(int argc, const char * logfile_path) {
    init_utf8_locale();  // ensures utf-8 mode enabled
    setup_terminal_environment();

    // todo temp debug during development so we don't have to manually pass a logfile name all the time
    logfile_path = logfile_path ? logfile_path : "./mazes_1_log.txt";
    // if a log file name was provided, use it to init the logging system
    init_log_file(logfile_path);

    // init ncurses environment for this specific application
    initscr();
    disable_rep();
    // we want to capture each keystroke as it happens instead of waiting for enter/newline.
    // we can use cbreak() (character break) or raw() for this.
    // raw() intercepts every character and passes it directly to us.
    // cbreak() is like raw(), but it lets the terminal capture control keys like ctrl-C, ctrl-Z, ctrl-X
    // since we need to use ctrl-C for copy, ctrl-X for cut, etc. in our app, we use raw() here
    raw();
    // don't print the user's keypress. we want full control of what gets printed
    noecho();
    // convert special keys from an ANSI escape sequence to a single key-code. Simplifies key processing.
    // note that keyok() is more fine-grained. It allows enabling/disabling individual keycodes.
    // keypad( true) enables a particular set of keys, but not all.
    // this has to be called on any window from which you want to get keyboard input. different windows
    // can have different states (true/false) set for each.
    keypad(stdscr, true);
    // key codes are synthetic. A key like `up-arrow` sends an escape sequence of 2 or more characters to
    // ncurses. when keypad(true) is called on a window, those sequences are combined into a single key code.
    // when ncurses sees the first key in the sequence, the escape key, it must wait some period of time to see
    // if other characters are sent as part of a single sequence. Then it can treat that entire sequence as one
    // key code. set_escdelay() sets how long (in milliseconds)  ncurses waits after an escape for more
    // characters to arrive. the default is 1000 (1 second) which means after pressing the esc key, ncurses waits a full
    // second. This makes processing a single esc press very tedious. So we set this to a small value where the wait
    // is imperceptible to a human, but still slow enough for the keyboard to send all the characters in the sequence.
    // note it cannot be set to 0, or no escape sequences can ever be processed as a single sequence.
    set_escdelay(25); // a good value is 25-50 ms.
    // hide the visible cursor indicator, normally it's a vertical or horizontal line, or a block. It can blink. These
    // options are set in the Terminal's settings menu. we have no control over these via ncurses. But we can show
    // or hide it. We hide it because we control where on the screen the user can enter data, and when we want to
    // get data from the user we will set the cursor position and then enable the indicator.
    // curs_set(1) shows the cursor
    curs_set(0);

    // sets how long ncurses waits after a mouse button click before it sends us that event. It needs to wait a bit
    // to see if the user clicks a second or third time in some time window, so it can report that as a double or
    // triple button click. If the app doesn't use double-clicks then it can feel sluggish after every mouse click while
    // it waits for another click before timing out. This command sets how long it will wait. Depending on the kind of
    // app, you might adjust this number depending on where on the screen the mouse is, or what window has the focus.
    mouseinterval(25); // in ms. default is ~167ms.

    // if needed : set_mouse_enabled(true); enable_bracketed_paste(input_win)
    set_mouse_enabled(true);
    // nodelay/notimeout/keypad (or keyok) per window as needed
    // if using color, start_color(), create colors and color pairs

    GlobalState gstate{};

    // test_colors();

    if (try_start_color(&gstate)) {
        init_app_colors();
    } else {
        log_printf("start_color() failed\n");
    }


    log_printf("after init_app_colors: gstate->%d\n", gstate.color_state);

    // we don't use stdscr, but we erase and refresh it once here
    werase(stdscr);
    wrefresh(stdscr);

    log_printf("Initialization completed\n");
    // background now has "default" colors assigned
    // in Terminal, this includes support for transparency. If the default terminal colors include
    // transparency, those are honored. This doesn't work in Ghosttty though.
    wbkgd(stdscr, COLOR_PAIR(0));

    // create the Layout
    // this Layout will have:
    // menu bar region
    // main region
    // text input region
    // status bar region
    // == 4 regions
    Layout main_layout = create_layout();

    main_layout.draw();
    doupdate(); // copies contents of newscr to curscr and the Terminal screen





    WINDOW* win = main_layout.root_window.get();

    wmove( win, 10,0);
    wgetch(win);
    wprintw(win, "✓ Hello from ncurses! logfile_path=%s\n", logfile_path);
    waddwstr(win, L"This is a wide string ✓"); //U+274C
    wgetch(win); // wait for input

    // start main event loop
    main_event_loop(main_layout);


    // Normal exit: atexit(cleanup_terminal) automatically performs
    // escape code reset, calls endwin(), and does termios restoration
    return 0;
}








}
