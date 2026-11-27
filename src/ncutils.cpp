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
#include <random>
#include <stdexcept>

namespace mazes_n_things {
    // -----------------------------------------------------------------
    //      mouse handling
    // -----------------------------------------------------------------

    void set_mouse_enabled(bool enable) {
        if (enable) {
            // 1. Enable ncurses mouse parsing
            mousemask(ALL_MOUSE_EVENTS, nullptr);
            mouseinterval(25);
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

/*
Shape names: Modern terminals follow standard CSS cursor names such as default, pointer, text, crosshair,
wait, grab, grabbing, not-allowed, and resize arrows like ew-resize or ns-resize
*/
void set_mouse_pointer(const char *shape) {
        printf("\033]22;%s\033\\", shape);
        fflush(stdout);
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
            if (!log_file) {
                fprintf(stdout, "fopen failed for '%s'\n", path_str);
                fflush(stdout);
                return;
            }
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
    static void log_vprintf(const char *fmt, va_list args, const bool append_newline) {
        fprint_current_date_and_time(log_file, false);
        fprintf(log_file, " ");
        vfprintf(log_file, fmt, args);
        if (append_newline) {
        }
        fprintf(log_file, "\n");
        fflush(log_file);  // Ensures log entries are flushed immediately
    }

    void log_printf(const char *fmt, ...) {
        if (log_file && fmt) {
            va_list args;
            va_start(args, fmt);
            log_vprintf(fmt, args, false);
            va_end(args);
        }
    }

    void log_printlnf(const char *fmt, ...) {
        if (log_file && fmt) {
            va_list args;
            va_start(args, fmt);
            log_vprintf(fmt, args, true);
            va_end(args);
        }
    }

    void log_printlnf() {
        fprintf(log_file, "\n");
        fflush(log_file);
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
    static volatile sig_atomic_t g_terminal_captured{};
    static volatile sig_atomic_t g_signal_terminal_restored{};

    // Forward declaration: sanitizer failure handling must not enter ncurses.
    static void restore_terminal_from_signal();

    static void cleanup_terminal() {
        // Direct write of reset sequences to avoid stdio buffer state:
        // - \033[?2004l: Disable Bracketed Paste Mode
        // - \033[?1003l: Disable Any-Event Mouse Tracking
        // - \033[?1002l: Disable Button-Motion Mouse Tracking
        // - \033[?1000l: Disable Normal Mouse Tracking
        // - \033[?25h:   Ensure cursor is visible
        // - \033[0m:     Reset all text rendition attributes
        //  \033]22;\033\\ Restore default mouse pointer
        // - \033[?1049l: Leave the terminal alternate screen used by ncurses
        //

        constexpr char reset_seq[] = "\033[?2004l\033[?1003l\033[?1002l\033[?1000l\033[?25h\033[0m\033]22;\033\\\033[?1049l";
        write(STDOUT_FILENO, reset_seq, sizeof(reset_seq) - 1);

        if (!isendwin()) {
            endwin();
        }
        if (g_terminal_captured) {
            tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
            g_terminal_captured = false;
        }
    }

    // Sanitizer hook: called before ASan prints its diagnostic.  Do not call
    // endwin() here: ncurses can emit a second screen transition after ASan
    // starts writing, which makes the diagnostic disappear from the terminal.
    void __asan_on_error() {
        restore_terminal_from_signal();
    }

    // Keep sanitizer diagnostics available after ncurses restores the terminal.
    // ASan appends the process id to this path, so concurrent runs do not
    // overwrite one another's reports.
    extern "C" const char *__asan_default_options() {
        return "log_path=asan_report:detect_leaks=1";
    }

    // Only async-signal-safe operations belong in this path.  In particular,
    // do not call ncurses (endwin/isendwin) after a fatal signal.
    static void restore_terminal_from_signal() {
        constexpr char reset_seq[] = "\033[?2004l\033[?1003l\033[?1002l\033[?1000l\033[?25h\033[0m\033]22;\033\\\033[?1049l";
        const int saved_errno = errno;

        // ASan invokes this hook before printing and may subsequently raise
        // SIGABRT.  Do not leave the alternate screen a second time: that
        // would restore the saved screen and hide the diagnostic just printed.
        if (g_signal_terminal_restored) {
            errno = saved_errno;
            return;
        }
        g_signal_terminal_restored = 1;

        (void) write(STDOUT_FILENO, reset_seq, sizeof(reset_seq) - 1);
        if (g_terminal_captured) {
            (void) tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
            g_terminal_captured = 0;
        }
        errno = saved_errno;
    }

    static void signal_handler(int signum) {
        restore_terminal_from_signal();

        // Preserve the original signal disposition and re-raise the signal so
        // ASan can finish its report and the shell receives the real status.
        signal(signum, SIG_DFL);
        raise(signum);
    }


    void setup_terminal_environment() {
        // Capture pristine terminal state before ncurses modifies it
        if (tcgetattr(STDIN_FILENO, &g_orig_termios) == 0) {
            g_terminal_captured = true;
        }
        g_signal_terminal_restored = 0;

        // Register cleanup to run automatically on standard exit / return from main
        atexit(cleanup_terminal);

        // Trap signals that would otherwise leave the terminal in raw/mouse mode
        struct sigaction sa{};
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESETHAND;

        sigaction(SIGINT, &sa, nullptr);   // Ctrl+C, ignored in raw() mode
        sigaction(SIGTERM, &sa, nullptr);  // Termination request
        sigaction(SIGHUP, &sa, nullptr);   // Terminal closed
        sigaction(SIGABRT, &sa, nullptr);  // ASan and abort() termination
        sigaction(SIGSEGV, &sa, nullptr);  // invalid memory access
        sigaction(SIGBUS, &sa, nullptr);
        sigaction(SIGILL, &sa, nullptr);
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


    // misc

    bool in_range(int value, const HalfOpenRange range) {
        return value >= range.start && value < range.end;
    }

    bool in_range(int value, const ClosedRange range) {
        return value >= range.start && value <= range.end;
    }

    int random_int(int min_inclusive, int max_inclusive) {
        if (min_inclusive > max_inclusive) {
            throw std::invalid_argument("min_inclusive must not exceed max_inclusive");
        }

        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution{min_inclusive, max_inclusive};
        return distribution(generator);
    }

    Bounds window_bounds( const WINDOW * win) {
        return {
            .position = { .x = getbegx(win), .y = getbegy(win) },
            .size = { .width = getmaxx(win), .height = getmaxy(win) }
        };
    }

} // namespace mazes_n_things
