//  entry_point_01.cpp
//
//  Created by Rob Ross on 11/19/26.
//
//  Copyright (c) 2026.  All rights reserved.

#include <ncurses.h>
#include <panel.h>
#include <menu.h>

#include <iostream>
#include <clocale>

#include "ncutils.h"

namespace entry_point_1 {

int main(int argc, const char * logfile_path) {
    init_utf8_locale();  // ensures utf-8 mode enabled
    setup_terminal_environment();

    // todo temp debug during development so we don't have to manually pass a logfile name all the time
    logfile_path = logfile_path ? logfile_path : "./logs/mazes_1_log.txt";
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
    // nodelay/notimeout/keypad (or keyok) per window as needed
    // if using color, start_color(), create colors and color pairs


    // we don't use stdscr, but we erase and refresh it once here
    werase(stdscr);
    wrefresh(stdscr);

    log_printf("Initialization completed\n");

    move(0,0);
    getch();
    printw("✓ Hello from ncurses!\n");
    addwstr(L"This is a wide string ✓"); //U+274C
    getch(); // wait for input



    // Normal exit: atexit(cleanup_terminal) automatically performs
    //    escape code reset, endwin(), and termios restoration
    return 0;
}








}
