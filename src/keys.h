//  keys.h
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.


/*

int ret = wget_wch(root_win, &ch);

ret: ERR, OK, or KEY_CODE_YES

ERR:
    no input, ignore event
OK:
    ASCII value of a pressed key. (Only reports keys mapped to ASCII characters)
    also reports ctrl key down + ASCII char, e.g., ctrl-A
KEY_CODE_YES:
    Pseudo-character tokens outside ASCII range.
    E.g., arrow keys, function keys, keypad keys, etc.
    You only get these events if you have called keypad(win, true).
*/


#ifndef MAZES_N_THINGS_KEYS_H
#define MAZES_N_THINGS_KEYS_H
#include <ncurses.h>

namespace mazes_n_things {



// ncurses provides these kinds of keyboard shortcuts
// ctrl + lowercase ASCII char.
//  Char in range of 0x20 - 0x1F (exclusive). Does not distinguish between upper and lower case.
// meta key + ASCII char or key code.
//  Defined by calling define_key() with an escape sequence and a unique key code > KEY_MAX
//  the meta key is ALT or OPTION. Example sequence : "\033a" For option + a (lowercase a).
//  NOTE: This must be enabled in the user's terminal by selecting "Use Option/Alt key as Meta key"
//  case is distinguished, so option-a is distinct from option-A. When enabled, and the user presses
//  meta (option/alt) + 'a',  the terminal encodes this as the sequence "\033a". If keypad(true) has been called,
//  ncurses intercepts the multibyte sequence and translates it into a higher level semantic event with the
//  keycode used to define this sequence via define_keys("\033a", KEY_ALT_A), for example.
// ctrl + meta + lowercase ASCII char
//  example,  ctrl + option + A.  lowercase only
// in KEYBOARD_SHORTCUT, some combinations are not allowed:
// ctrl + shift + char : not allowed
// ctrl + option + shift + char  : not allowed

/*
    Mnemonic shortcuts are displayed after the menu item.
    Possible values are:
    ⌥ (Option / Alt): U+2325
    ⇧ (Shift): U+21E7
    ⌃ (Control): U+2303
    The display order is Shift, Control, Option, Letter.
    At most, 2 modifier keys are used
    Examples:
    ⌃Z, ⌥Z, ⌃⌥A, ⇧⌥B
 */
struct KEYBOARD_SHORTCUT {
    // these are mutually exclusive, suggests a scoped enum
    bool is_ctrl;
    bool is_ctrl_alt;
    bool is_alt;
    bool is_shift_alt;

    // key:
    // ASCII code of char if 0x20 >= char < 0x7F
    // control code (char & 0x1F) if key is ctrl + char, where 0x41 >= char < 0x60
    // or a keycode if char > 0x1F.
    int  key;
    unsigned control_shortcut;
};


#ifndef KEY_ESC
#define KEY_ESC 27 // 0x1B, octal 033
#endif

constexpr int KEY_ALT_F2 = KEY_F(7); // todo temp this is what Terminal uses for some reason



} // namespace mazes_n_things
#endif //MAZES_N_THINGS_KEYS_H
