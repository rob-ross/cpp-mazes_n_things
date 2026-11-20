//  ncutils.cpp
//
//  Created by Rob Ross on 10/23/26.
//
//  Copyright (c) 2026.  All rights reserved.



#include "ncutils.h"

#include <ncurses.h>

#include <csignal>   // signal.h
#include <cstring>   // string.h
#include <cstdio>    // stdio.h
#include <cstdarg>   // stdarg.h
#include <cstdlib>   // stdlib.h
#include <cwchar>    // wchar.h
#include <cctype>    // needed for iscntrl
#include <clocale>

#include <unistd.h>
#include <signal.h>
#include <term.h>
#include <termios.h>

#include <stdarg.h>  // Added for va_list, va_start, va_end

#include <chrono>
#include <ctime>


// -----------------------------------------------------------------
//      mouse handling
// -----------------------------------------------------------------

// calling this will clobber the mouse moved settings. If you need to enable mouse moved events
// as well, call set_mouse_moved_enabled(true) after calling set_mouse_enabled(true)
void set_mouse_enabled(bool enable) {
    if (enable) {
        // 1. Enable ncurses mouse parsing
        mousemask(ALL_MOUSE_EVENTS, nullptr);
        mouseinterval(0);
        // 2. Instruct terminal emulator to capture clicks
        printf("\033[?1000h");
        fflush(stdout);
    } else {
        // 1. Disable ncurses mouse parsing
        mousemask(0, nullptr);
        // 2. Instruct terminal emulator to restore native selection
        printf("\033[?1002l\033[?1000l");
        fflush(stdout);
    }
}

void set_mouse_moved_enabled(bool enable) {
    mmask_t cur_mask;
    mousemask(ALL_MOUSE_EVENTS, &cur_mask);
    if (enable) {
        // set REPORT_MOUSE_POSITION in current mask
        mousemask( cur_mask | REPORT_MOUSE_POSITION, nullptr);
        // Enable any-event mouse tracking (Mode 1003)
        // NOTE: this must come AFTER both initscr() and mousemask(), or they will overwrite this setting when invoked.
        printf("\033[?1003h");
        fflush(stdout);
    } else {
        // clear REPORT_MOUSE_POSITION in current mask
        mousemask( cur_mask & ~REPORT_MOUSE_POSITION, nullptr);
    }
}

// -----------------------------------------------------------------
//      copy/paste
// -----------------------------------------------------------------


/*

To use Terminal-Native OSC 52 Sequences
1. Convert the wide string slice to UTF-8.
2. Base64 encode the UTF-8 bytes.
3. Emit printf("\033]52;c;%s\007", base64_data); fflush(stdout);.

Trade-off note: While OSC 52 provides optimal write throughput without subprocess overhead,
reading from the clipboard via OSC 52 is often disabled by default in terminal emulators for security reasons.

*/

bool copy_to_system_clipboard(const wchar_t *text, const size_t len) {
    if (!text || len == 0) return false;

    // 1. Determine required multibyte buffer size (UTF-8)
    // Create a temporary null-terminated wide slice
    auto *wide_slice = static_cast<wchar_t *>(std::malloc((len + 1) * sizeof(wchar_t)));
    if (!wide_slice) return false;
    std::wmemcpy(wide_slice, text, len);
    wide_slice[len] = L'\0';

    size_t mb_size = std::wcstombs(nullptr, wide_slice, 0);
    if (mb_size == static_cast<size_t>(-1)) {
        std::free( wide_slice );
        return false;
    }

    char *mb_buffer = static_cast<char *>(std::malloc(mb_size + 1));
    if (!mb_buffer) {
        std::free(wide_slice);
        return false;
    }
    std::wcstombs(mb_buffer, wide_slice, mb_size + 1);

    // 2. Open write pipe to macOS clipboard daemon
    FILE *pipe = popen("pbcopy", "w");
    if (!pipe) {
        std::free(wide_slice);
        std::free(mb_buffer);
        return false;
    }

    fwrite(mb_buffer, sizeof(char), mb_size, pipe);
    pclose(pipe);

    std::free(wide_slice);
    std::free(mb_buffer);
    return true;
}

size_t paste_from_system_clipboard(wchar_t *dest, size_t max_capacity, bool strip_ctrl_chars) {
    if (!dest || max_capacity == 0) return 0;

    FILE *pipe = popen("pbpaste", "r");
    if (!pipe) return 0;

    // Read UTF-8 stream from pbpaste
    char mb_buffer[1024];
    size_t bytes_read = fread(mb_buffer, 1, sizeof(mb_buffer) - 1, pipe);
    pclose(pipe);

    if (bytes_read == 0) return 0;
    mb_buffer[bytes_read] = '\0';

    if (strip_ctrl_chars) {
        for (size_t i = 0; i < bytes_read; ++i) {
            if (iscntrl(mb_buffer[i])) {
                mb_buffer[i] = ' ';
            }
        }
    }

    // Convert UTF-8 multibyte to wchar_t
    size_t wide_len = mbstowcs(dest, mb_buffer, max_capacity - 1);
    if (wide_len == static_cast<size_t>(-1)) {
        dest[0] = L'\0';
        return 0;
    }

    dest[wide_len] = L'\0';
    return wide_len;
}



// -----------------------------------------------------------------
//      Logging
// -----------------------------------------------------------------

static FILE *log_file = nullptr;

void init_log_file(const char * path_str) {
    // 1. Open the file for writing ("w" overwrites, "a" appends)
    // todo (rob) error checking, reporting.j
    if (path_str) {
        log_file = fopen(path_str, "w");
        fprint_current_date_and_time(log_file, true);
        fprintf(log_file, " logging started\n\n");
    }
}

void fprint_current_date_and_time(FILE* file, bool include_year) {
    const auto now = std::chrono::system_clock::now();
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    const std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
#if defined(__APPLE__) || defined(__unix__)
    localtime_r(&current_time, &local_time);
#else
    local_time = *std::localtime(&current_time);
#endif

    if (include_year) {
        std::fprintf(file, "%04d-%02d-%02d %02d:%02d:%02d:%03lld",
                    local_time.tm_year + 1900,
                    local_time.tm_mon + 1,
                    local_time.tm_mday,
                    local_time.tm_hour,
                    local_time.tm_min,
                    local_time.tm_sec,
                    static_cast<long long>(milliseconds.count()));
    } else {
        std::fprintf(file, "%02d:%02d:%02d:%03lld",
                    local_time.tm_hour,
                    local_time.tm_min,
                    local_time.tm_sec,
                    static_cast<long long>(milliseconds.count()));
    }
}
void log_printf(const char *fmt, ...) {
    if (log_file && fmt) {
        va_list args;
        va_start(args, fmt);
        fprint_current_date_and_time(log_file, false);
        fprintf(log_file, " ");
        vfprintf(log_file, fmt, args);
        va_end(args);
        fflush(log_file);  // Ensures log entries are flushed immediately
    }
}

void close_log_file() {
    if (log_file) {
        fflush(log_file);
        fclose(log_file);
    }
}


// -----------------------------------------------------------------
//      setup / config / restore ncurses environment
// -----------------------------------------------------------------

// for restoring Terminal state even when exiting abnormally
static struct termios g_orig_termios{};
static bool g_terminal_captured{};


static void cleanup_terminal() {
    // Direct write of reset sequences to avoid stdio buffer state:
    // - \033[?2004l: Disable Bracketed Paste Mode
    // - \033[?1003l: Disable Any-Event Mouse Tracking
    // - \033[?1002l: Disable Button-Motion Mouse Tracking
    // - \033[?1000l: Disable Normal Mouse Tracking
    // - \033[?25h:   Ensure cursor is visible
    // - \033[0m:     Reset all text rendition attributes
    constexpr char reset_seq[] = "\033[?2004l\033[?1003l\033[?1002l\033[?1000l\033[?25h\033[0m";
    write(STDOUT_FILENO, reset_seq, sizeof(reset_seq) - 1);
    write(STDOUT_FILENO, reset_seq, sizeof(reset_seq) - 1);

    if (!isendwin()) {
        endwin();
    }
    if (g_terminal_captured) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
        g_terminal_captured = false;
    }
}

// Sanitizer hook: Called automatically by ASan/UBSan right before process termination
// todo (rob) this doesn't seem to do anything when app exits abnormally due to asan.
void __asan_on_error() {
    // Emit escape codes and restore terminal line discipline
    cleanup_terminal();
}

static void signal_handler(int signum) {
    cleanup_terminal();
    // Re-raise or exit cleanly
    _exit(128 + signum);
}

void setup_terminal_environment() {
    // Capture pristine terminal state before ncurses modifies it
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == 0) {
        g_terminal_captured = true;
    }

    // Register cleanup to run automatically on standard exit / return from main
    atexit(cleanup_terminal);

    // Trap signals that would otherwise leave the terminal in raw/mouse mode
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);   // Ctrl+C, ignored in raw() mode
    sigaction(SIGTERM, &sa, nullptr);  // Termination request
    sigaction(SIGHUP, &sa, nullptr);   // Terminal closed
}


void disable_rep() {
    // Disable REP capability so ncurses outputs raw characters individually
    // this was an optimization in older terms where a string of the same character
    // would be combined into one terminal command. But it causes drawing artifacts
    // and modern cpus are so fast this is no longer an optimization
    if (repeat_char) {
        repeat_char = nullptr;
    }
}

// todo these are placeholder numbers for now
constexpr int KEY_PASTE_START = 0x1003;
constexpr int KEY_PASTE_END   = 0x1004;

void enable_bracketed_paste() {
    // Tell the terminal emulator to bracket pastes with \033[200~ and \033[201~
    printf("\033[?2004h");
    fflush(stdout);

    // Map the terminal escape markers to ncurses key codes
    define_key("\033[200~", KEY_PASTE_START);
    define_key("\033[201~", KEY_PASTE_END);
}

void disable_bracketed_paste() {
    // Restore default terminal behavior on exit
    printf("\033[?2004l");
    fflush(stdout);
}

bool init_utf8_locale() {
    // 1. Try to inherit the user's environment
    if ( !std::setlocale(LC_ALL, "")) {
        // 2. Fallback to standard C.UTF-8 or UTF-8 if the environment is misconfigured
        if (!std::setlocale(LC_ALL, "C.UTF-8")) {
            if ( !std::setlocale(LC_ALL, "en_US.UTF-8")) {
                return false;
            }
        }
    }
    return true;
}
