//  ncutils.h
//
//  Created by Rob Ross on 10/23/26.
//
//  Copyright (c) 2026.  All rights reserved.

// utility functions for working with ncurses

#ifndef MAZES_N_THINGS_NCUTILS_H
#define MAZES_N_THINGS_NCUTILS_H

#include <ncurses.h>

// macros to translate a printable ASCII value to/from its control-modified key code
#ifndef CTRL
#define CTRL(x) ( (x) & 0x1F )
#endif

#ifndef UNCTRL
#define UNCTRL(c) ((c) + 0x40 )   // '@'
#endif

#ifndef KEY_ESC
#define KEY_ESC 033 // decimal 27, hex 0x1B, octal 033
#endif

typedef struct {
    // represents the closed interval [start,end]
    int start; // inclusive
    int end;   // inclusive
} ClosedRange;

typedef struct {
    int start; // inclusive
    int end;   // exclusive
} HalfOpenRange;


// -----------------------------------------------------------------
//      mouse handling
// -----------------------------------------------------------------

// calling this will clobber the mouse moved settings. If you need to enable mouse moved events
// as well, call set_mouse_moved_enabled(true) after calling set_mouse_enabled(true)
void set_mouse_enabled(bool enable);

// this enables mouse-moved event reporting, including drag events
void set_mouse_moved_enabled(bool enable);


// -----------------------------------------------------------------
//      copy/paste
// -----------------------------------------------------------------

bool copy_to_system_clipboard(const wchar_t *text, size_t len);
size_t paste_from_system_clipboard(wchar_t *dest, size_t max_capacity, bool strip_ctrl_chars);


// -----------------------------------------------------------------
//      Logging
// -----------------------------------------------------------------


void init_log_file(const char * log_filename);
void close_log_file();
void log_printf(const char *fmt, ...);
void fprint_current_date_and_time(FILE* file, bool include_year);

// -----------------------------------------------------------------
//      setup / config / restore ncurses environment
// -----------------------------------------------------------------

void setup_terminal_environment();
// must be called *after* initscr()
void disable_rep();
void enable_bracketed_paste();

// the cleanup exit handler will call this, so you only would need to call this yourself
// if you wanted to temporarily disable this mode during program execution
void disable_bracketed_paste();

// returns true if a utf-8 locale was set, false if not
bool init_utf8_locale();



#endif //MAZES_N_THINGS_NCUTILS_H
