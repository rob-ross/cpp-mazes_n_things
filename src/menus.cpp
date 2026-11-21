//  menu.cpp
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

/*
The Unicode code points for the standard Mac keyboard modifier symbols are:
• ⌘ (Command / Clover): U+2318
• ⌥ (Option / Alt): U+2325
• ⇧ (Shift): U+21E7
• ⇪ (Caps Lock): U+21EA
• ⌃ (Control): U+2303
• ⎋ (Escape): U+238B
• ⌫ (Delete / Backspace): U+232B
• ⌦ (Forward Delete): U+2326
• ↩ (Return): U+21A9
• ⌕ (Enter): U+2315
• ⇥ (Tab Right): U+21E5
• ⇤ (Tab Left): U+21E4

The equivalent Windows modifier keys do not always have standard, dedicated symbolic representations like Mac keys,
but they are frequently represented by specific Unicode symbols in technical documentation and UI design:
• ⊞ (Windows Logo Key): U+229E (Squared Plus, commonly used to represent the Windows key) or ❖ (Square Diamond): U+2756
• ⎇ (Alt Key): U+2387 (Alternative Key Symbol)
• ⇧ (Shift): U+21E7 (Upwards White Arrow — identical to Mac)
• ⌃ (Control / Ctrl): U+2303 (Up Arrowhead — identical to Mac)
• ⇪ (Caps Lock): U+21EA (Upwards White Arrow From Bar — identical to Mac)
• ⎋ (Escape / Esc): U+238B (Broken Circle with Northwest Arrow — identical to Mac)
• ⌫ (Backspace): U+232B (Leftwards Erasure Symbol — identical to Mac)
• ⌦ (Delete): U+2326 (Erase to the Right — identical to Mac)
• ↵ (Enter): U+21B5 (Downwards Arrow with Corner Leftwards) or ⌕ (Numeric Pad Enter): U+2315
• ⇥ (Tab): U+21E5 (Rightwards White Arrow to Bar — identical to Mac)
• ▤ (Menu / Application Key): U+25A4 (Square with Horizontal Fill)

Unlike macOS, which natively relies heavily on these symbols in the operating system menus,
Windows environments traditionally write out the full names textually (e.g., Ctrl + Shift + Esc)
rather than using symbols.

*/

#include "menus.h"

#include <ncurses.h>
#include <panel.h>
#include <cstdlib>

#include "ncutils.h"
#include "colors.h"

namespace mazes_n_things {

// using namespace mazes_n_things;
constexpr char LINE_SEP_CHAR = '-';
constexpr char LINE_SEP_STR[2] = {LINE_SEP_CHAR, '\0'};

void draw_menubar(MenuBarModel *mb) {
    werase(mb->menubar_win);
    int menu_count = mb->menu_count;
    WINDOW * menubar_win = mb->menubar_win;
    for (int i = 0; i < menu_count; ++i) {
        wmove( menubar_win, 0, mb->titles[i].range.start);
        wprintw( menubar_win, "%s", mb->titles[i].text.c_str());
    }
    wnoutrefresh(mb->menubar_win);
}

void resize_menubar(MenuBarModel *mb) {
    int new_rows, new_cols;
    getmaxyx(stdscr, new_rows, new_cols);
    wresize(mb->menubar_win, 1, new_cols);
    draw_menubar(mb);
    touchwin(mb->menubar_win);
}

void draw_menu_separators(MENU *menu) {
    WINDOW *sub = menu_sub(menu);
    int rows, cols;
    scale_menu(menu, &rows, &cols);

    wattron(sub, CP_MENU_BODY_DISABLED);
    int num_items = item_count(menu);
    ITEM ** items = menu_items(menu);
    for (int i = 0; i < num_items; ++i ) {
        if (item_name(items[i])[0] == LINE_SEP_CHAR) {
            wmove(sub, i, 0);
            for (int j = 0; j < cols; ++j) {
                wprintw(sub, "─");
            }
        }
    }

    wattroff(sub, CPI_MENU_BODY_DISABLED);
    touchwin(menu_win(menu));
}



MENU * create_menu(const int items_data_size, ItemNameDesc item_data[], Coordinate origin) {

    // we add 1 to the size of item_data for the nullptr to terminate the item list
    const int items_count = items_data_size + 1;
    ITEM **items = static_cast<ITEM **>(std::calloc(items_count, sizeof(ITEM*)));

    for (int i = 0; i < items_data_size; ++i) {
        const char * name;
        if ( item_data[i].name[0] == LINE_SEP_CHAR ) {
            name = LINE_SEP_STR; // we share this one instance for all line separator items. We replace it below
        } else {
            name = strdup( item_data[i].name.c_str() );
        }
        const char *desc = nullptr;
        if (item_data[i].desc[0] != '\0') {
            desc = strdup(item_data[i].desc.c_str());
        }
        items[i] = new_item( name, desc );
        if ( item_data[i].name[0] == LINE_SEP_CHAR ) {
            item_opts_off(items[i], O_SELECTABLE);
        }
    }
    items[items_count - 1] = nullptr; // item list terminator

    MENU *menu = new_menu(items);

    // set_menu_fore doesn't set the foreground. It sets the attributes for a selected item
    set_menu_fore(menu, CP_MENU_BODY_HIGHLIGHT );
    // set_menu_back doesn't set the background. It sets the attributes for a selectable but not-selected item
    set_menu_back(menu, CP_MENU_BODY );
    // set_menu_grey sets the attributes for a non-selectable item
    set_menu_grey(menu, CP_MENU_BODY_DISABLED );
    // set_menu_mark sets the character drawn before an item when it is selected
    set_menu_mark(menu, "");

    int subwin_rows, subwin_cols;
    // determine the minimum size needed for the inner menu sub window
    scale_menu(menu, &subwin_rows, &subwin_cols);

    WINDOW* menuwin = newwin(subwin_rows + 2, subwin_cols + 2, origin.y, origin.x);
    keypad(menuwin, true);  // Allow the menu window to capture arrow keys natively

    wbkgd(menuwin, CP_MENU_BODY);

    wattron(menuwin, CP_MENU_BODY_DISABLED);
    // todo allow user to specify boxes draw around menu items as a setting.
    // box(menuwin, 0, 0);
    wattroff(menuwin, CP_MENU_BODY_DISABLED);
    WINDOW *menu_sub = derwin(menuwin, subwin_rows, subwin_cols, 1, 1);
    wbkgd(menu_sub, CP_MENU_BODY);

    set_menu_win(menu, menuwin); // The container window
    set_menu_sub(menu, menu_sub); // The actual text viewport

    // Post (internally draw) the menu so it's primed and ready
    post_menu(menu);

    return menu;
}


bool pos_in_menubar(Coordinate pos, MenuBarModel *mb) {
    return wenclose(mb->menubar_win, pos.y, pos.x);
}

bool pos_in_menu(Coordinate pos, MenuModel *ms ) {
    WINDOW * mwin = menu_win(ms->menu);
    return wenclose(mwin, pos.y, pos.x);
}


// Returns the index of the menu title for the given pos, or -1 if no menu title at that pos.
int menu_index_for_pos(Coordinate pos, MenuBarModel *mb) {
    if (pos_in_menubar(pos, mb)) {
        int menu_count = mb->menu_count;
        if (pos.x >= mb->titles[ menu_count - 1].range.end) {
            return -1; // click in menubar but not on any menu title
        }
        for (int i = 0; i < menu_count; ++i) {
            ClosedRange r = mb->titles[i].range;
            if (in_range(pos.x, r)) {
                // click in a menu title
                return i;
            }
        }
    }

    return -1;
}

// Returns true if the Coordinate is on a menu title.
bool pos_on_menu_title(Coordinate pos, MenuBarModel *mb) {
    WINDOW * mbar_win = mb->menubar_win;
    if (pos_in_menubar(pos, mb)) {
        int menu_count = mb->menu_count;
        if (pos.x >= mb->titles[ menu_count - 1].range.end) {
            return false; // click in menubar but not on any menu title
        }
        for (int i = 0; i < menu_count; ++i) {
            ClosedRange r = mb->titles[i].range;
            if (in_range(pos.x, r)) {
                // click in a menu title
                return true;
            }
        }
    }
    return false;
}



void highlight_menubar_title(MenuBarModel *mb, int index) {
    // first draw all titles normally to remove any previous highlight
    draw_menubar(mb);
    if (index >=0 && index < mb->menu_count) {
        wattron(mb->menubar_win, CP_MENU_BAR_HIGHLIGHT);
        mvwprintw(
            mb->menubar_win,
            0,
            mb->titles[index].range.start,
            "%s", mb->titles[index].text.c_str() );
        wattroff(mb->menubar_win, CP_MENU_BAR_HIGHLIGHT);
    }
    wnoutrefresh(mb->menubar_win);
}

void navigate_menu(MENU *menu, int request) {
    int max_items = item_count(menu);

    if (max_items <= 0) return;

    while (true) {
        // 2. Perform the movement step
        menu_driver(menu, request);

        // 3. Check if the newly landed item is selectable
        ITEM *cur = current_item(menu);

        if (item_opts(cur) & O_SELECTABLE) {
            break; // Success! It's active, stop looping.
        }
        if (item_index(cur) <= 0 || item_index(cur) >= max_items -1 ) {
            // we've navigated up or down to the start or end of the list and it's not selectable, so we break
            break;
        }
    }
}

void display_dropdown(PANEL *target_panel, MENU *target_menu) {
    // Bring the panel to the top and draw it
    show_panel(target_panel);
    top_panel(target_panel);
    // Ensure separator rules overwrite default item text before display
    draw_menu_separators(target_menu);

    update_panels();
    doupdate();
}



void blink_selected_menu_item(MENU * target_menu) {
    static constexpr int sleep_ms = 75;
    chtype saved_fore = menu_fore(target_menu);
    set_menu_fore(target_menu, CP_MENU_BODY);
    for (int i = 0; i < 3; ++i) {
        draw_menu_separators(target_menu);
        update_panels();
        doupdate();
        napms(sleep_ms); // sleep for sleep_ms
        set_menu_fore(target_menu, saved_fore);

        draw_menu_separators(target_menu);
        update_panels();
        doupdate();
        napms(sleep_ms);  // sleep for sleep_ms
        set_menu_fore(target_menu, CP_MENU_BODY);
    }
    // restore
    set_menu_fore(target_menu, saved_fore);
}

void activate_dropdown(MenuModel *menus, MenuBarModel *mb) {
    MENU * target_menu = menus->menu;
    WINDOW *menu_window = menu_win(target_menu);
    // create the panel for displaying the menu
    PANEL *target_panel = new_panel(menu_window);
    display_dropdown(target_panel, target_menu);

    int menu_ch;
    bool in_menu = true;

    while(in_menu) {
        menu_ch = wgetch(menu_window);
        switch(menu_ch) {
            case KEY_RESIZE: {
                in_menu = false;
            }
            break;

            case KEY_DOWN: {
                navigate_menu(target_menu, REQ_DOWN_ITEM);
                draw_menu_separators(target_menu);
            }
            break;

            case KEY_UP: {
                navigate_menu(target_menu, REQ_UP_ITEM);
                draw_menu_separators(target_menu);
            }
            break;

            case '\n': {
                // Enter key
                ITEM *cur = current_item(target_menu);
                if (cur != nullptr && (item_opts(cur) & O_SELECTABLE) ) {
                    const char *name = item_name(cur);
                    // (Optional) Store selection details or trigger a callback here
                    log_printf("menu item #%d selected: %s\n", item_index(cur), name );

                    blink_selected_menu_item(target_menu);

                }
                in_menu = false;
            }
            break;

            case KEY_ESC: {
                in_menu = false;
            }
                break;

            case KEY_LEFT:
            case KEY_RIGHT: {
                // display next/previous menu, loop around at the ends
                hide_panel(target_panel);
                update_panels();
                doupdate();
                int step = menu_ch == KEY_LEFT ? -1 : 1;
                int next_menu_index = (menus->index + mb->menu_count + step ) % mb->menu_count;
                menus = &mb->menus[next_menu_index];
                target_menu = menus->menu;
                menu_window = menu_win(target_menu);
                replace_panel( target_panel, menu_window );

                highlight_menubar_title(mb, menus->index);

                display_dropdown(target_panel, target_menu);

                break;
            }

            case KEY_MOUSE: {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.bstate & ( BUTTON1_PRESSED | BUTTON1_CLICKED)) {
                        Coordinate pos{.x = event.x, .y = event.y};
                        if (!pos_in_menubar(pos, mb) && !pos_in_menu(pos, menus)) {
                            // if click not in menu bar and not in current menu we can dismiss the menu popup.
                            in_menu = false;
                            break;
                        }
                        if (pos_in_menu(pos, menus)) {
                            // if the click is on a selectable item, make it the current item and dismiss the menu

                            // get the y pos of the click in the subwindow. That will be the item index.
                            // if selectable, select that item and dismiss the menu popup
                            Coordinate win_pos = { pos.x, pos.y};
                            if (wmouse_trafo( menu_sub(menus->menu), &win_pos.y, &win_pos.x, false )) {
                                ITEM ** items = menu_items(menus->menu);
                                ITEM * item = items[win_pos.y];
                                log_printf("KEY_MOUSE: item pos.y = %d, name=%s\n",win_pos.y, item_name(item));
                                if (item_opts(item) & O_SELECTABLE ) {
                                    set_current_item(menus->menu, item);
                                    blink_selected_menu_item(target_menu);
                                    in_menu = false;
                                    break;
                                }
                            }

                        } else {
                            // click in menu bar.
                            int menu_index = menu_index_for_pos(pos, mb);
                            if ( menu_index == -1 ) {
                                log_printf("menu_index == -1");
                                in_menu = false;
                                break; // no menu was clicked on
                            }
                            if (menu_index != menus->index) {
                                // only draw a new popup if it's not the current popup
                                log_printf("menubar click : menu_index == %d\n", menu_index);

                                // switch to a different popup menu
                                hide_panel(target_panel);

                                menus = &mb->menus[menu_index];
                                target_menu = menus->menu;
                                menu_window = menu_win(target_menu);
                                highlight_menubar_title(mb, menu_index);

                                // replaces the panel's window with a new one
                                replace_panel( target_panel, menu_window );

                                display_dropdown(target_panel, target_menu);
                                break;
                            }
                        }
                    }
                }
            }
            break;



            // if mouse is moved in the menubar dismiss the current popup and display the next menu

        } // switch(menu_ch) {

        // 5. CRITICAL: Redraw the panel z-stack to render the new highlight
        update_panels();
        doupdate();
    } // while(in_menu) {

    // 6. Tear down the modal view and restore the background perfectly
    hide_panel(target_panel);
    highlight_menubar_title(mb, -1); // remove any highlighted title
    update_panels();
    doupdate();

    del_panel(target_panel);
}

// enters a read loop to process interaction with the menu bar and menus.
// Returns when a menu is dismissed
// and returns the selected item, if any.
void process_menubar_click(MenuBarModel *mb,  WINDOW * root_win, MEVENT event) {
    // todo optimization - we can immediately rule out a menu title hit if event.x < 0 or > max range value
    int main_rows, main_cols;
    getmaxyx(root_win, main_rows, main_cols);
    mvwprintw(root_win, main_rows - 2, 0, "You clicked in the menu bar at r:%d c:%d", event.y, event.x);
    bool menu_clicked = false;
    int menu_index = menu_index_for_pos({ .x = event.x, .y = event.y}, mb);
    if (menu_index != -1) {
        // click in a menu title
        wmove(root_win, main_rows - 3, 0);
        wprintw(root_win, "\nYou clicked on the %s menu title at r:%d c:%d.",
            mb->titles[menu_index].text.c_str(), event.y, event.x);
        menu_clicked = true;
        wnoutrefresh(root_win);

        highlight_menubar_title(mb, menu_index);
        doupdate();
        activate_dropdown( &mb->menus[menu_index], mb);
    }
    // todo return the result of the menu click, an action object, lambda, etc.
}

} // namespace mazes_n_things
