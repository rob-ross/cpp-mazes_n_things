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
#include <cassert>

#include "ncutils.h"
#include "colors.h"

namespace mazes_n_things {


// using namespace mazes_n_things;
constexpr char LINE_SEP_CHAR = '-';
constexpr char LINE_SEP_STR[2] = {LINE_SEP_CHAR, '\0'};


void MenuBarView::draw(const WidgetContext& context, const MenuBarModel& model) {
    WINDOW * menubar_win = context.win;
    werase(menubar_win);
    for (const auto& title : model.titles) {
        wmove(menubar_win, 0, title.range.start);
        wprintw(menubar_win, "%s", title.text.c_str());
    }
    wnoutrefresh(context.win);
    if (is_subwin(context.win)) {
        WINDOW * parent = wgetparent(context.win);
        touchwin(parent);
    }

}

/*void resize_menubar(MenuBarModel *mb) {
    int new_rows, new_cols;
    getmaxyx(stdscr, new_rows, new_cols);
    wresize(mb->win, 1, new_cols);
    draw_menubar(mb);
    touchwin(mb->win);
}*/

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


MENU * create_menu(const std::vector<MenuItemModel> &item_data, Coordinate origin) {

    // we add 1 to the size of item_data for the nullptr to terminate the items array new_menu() argument.
    const size_t items_count = item_data.size() + 1;
    ITEM **items = static_cast<ITEM **>(std::calloc(items_count, sizeof(ITEM*)));

    for (size_t i = 0; i < item_data.size(); ++i) {
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

    return menu;
}


/*bool pos_in_menubar(Coordinate pos, MenuBarModel& mb) {
    return wenclose(mb.win, pos.y, pos.x);
}*/

bool pos_in_popup_menu( const Coordinate pos, const MenuPopup& popup ) {
    return window_bounds( popup.window()).contains(pos);
}


// Returns the index of the menu title for the given pos, or -1 if no menu title at that pos.
static int menu_index_for_pos(Coordinate pos, MenuBarModel& mb) {
    const int menu_count = static_cast<int>(mb.titles.size());
    if (pos.x > mb.titles[ menu_count - 1].range.end + 1) {
        return -1; // click in menubar but not on any menu title
    }
    for (int i = 0; i < menu_count; ++i) {
        ClosedRange r = mb.titles[i].range;
        // we grow the hit area by one col on each side to make it easier to select a menu title.
        // this works because we add 2 blank cols before each title.
        r.start -= 1;
        r.end +=1;
        if (in_range(pos.x, r)) {
            // click in a menu title
            return i;
        }
    }
    return -1;
}

// Returns true if the Coordinate is on a menu title.
/*bool pos_on_menu_title(Coordinate pos, MenuBarModel *mb) {
    WINDOW * mbar_win = mb->win;
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
}*/

// draw the menu bar title with index `index` using the menu bar's CP_MENU_BAR_HIGHLIGHT color pair.
static void highlight_menubar_title( const WidgetContext& context, MenuBarModel& mb, int index) {
    const int menu_count = static_cast<int>(mb.titles.size());
    WINDOW * win = context.win;
    MenuBarView::draw(context, mb); // first draw menubar with no highlighting
    if (index >=0 && index < menu_count) {
        wattron(win, CP_MENU_BAR_HIGHLIGHT);
        mvwprintw(
            win,
            0,
            mb.titles[index].range.start - 1,
            " %s ", mb.titles[index].text.c_str() );
        wattroff( win, CP_MENU_BAR_HIGHLIGHT);
    }
    wnoutrefresh( win );
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
    post_menu(target_menu);
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

struct dropdown_state {
    int menu_index;
    int menu_id;
    // target_menu and menu_win are obtainable via a MenuPopup
    // MENU * target_menu;
    // WINDOW * menu_window;
    MenuPopup popup;

    static dropdown_state state_for_menu(int index, MenuBarModel &mb, MenuPopupManager &pm) {
        int menu_count = static_cast<int>(mb.menus.size());
        if (index < 0 && index >= menu_count) {
            log_printlnf("state_for_menu: index:%d is out of bounds (0 <= i < %d", index, menu_count);
            assert(false);
        }
        int menu_id = mb.menus[index].id;
        if ( !pm.contains_id(menu_id)) {
            log_printlnf("state_for_menu: no popup menu found for menu_index:%d, menu_id:%d", index, menu_id);
            assert(false);
        }

        MenuPopup popup = pm.popup_for_id(menu_id);
        return { .menu_index = index, .menu_id = menu_id, .popup = popup };
    }

    MENU * target_menu() const {
        return popup.menu();
    }

    WINDOW * menu_window() const {
        return popup.window();
    }

};

void activate_menubar_menu_dropdown( const WidgetContext &context,
            const int menu_index, MenuBarModel &mb, MenuPopupManager &pm) {

    dropdown_state state = dropdown_state::state_for_menu(menu_index, mb, pm);

    // create a panel for displaying the menu on top of existing Layouts
    PANEL *target_panel = new_panel( state.menu_window() );
    display_dropdown(target_panel, state.target_menu() );

    int menu_ch;
    bool in_menu = true;
    int menu_count = static_cast<int>(mb.menus.size());

    while(in_menu) {
        menu_ch = wgetch( state.menu_window() );
        if ( menu_ch < MAX_ASCII && menu_ch == CTRL('q')) {
            unget_wch(menu_ch); // pass the event to be handled up the call chain
            in_menu = false;
            continue;
        }

        switch(menu_ch) {

            case KEY_RESIZE: {
                in_menu = false;
            }
            break;

            case KEY_DOWN: {
                navigate_menu(state.target_menu(), REQ_DOWN_ITEM);
                draw_menu_separators(state.target_menu());
            }
            break;

            case KEY_UP: {
                navigate_menu(state.target_menu(), REQ_UP_ITEM);
                draw_menu_separators(state.target_menu());
            }
            break;

            case '\n': {
                // Enter key
                ITEM *cur = current_item(state.target_menu());
                if (cur != nullptr && (item_opts(cur) & O_SELECTABLE) ) {
                    const char *name = item_name(cur);
                    // (Optional) Store selection details or trigger a callback here
                    log_printlnf("menu item #%d selected: %s", item_index(cur), name );

                    blink_selected_menu_item(state.target_menu());

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
                int new_menu_index = ( state.menu_index + menu_count + step ) % menu_count;

                state = dropdown_state::state_for_menu(new_menu_index, mb, pm);
                replace_panel( target_panel, state.menu_window() );

                highlight_menubar_title(context, mb, state.menu_index );
                display_dropdown(target_panel, state.target_menu() );

                break;
            }

            case KEY_MOUSE: {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.bstate & ( BUTTON1_PRESSED | BUTTON1_CLICKED)) {
                        const Coordinate pos{.x = event.x, .y = event.y};
                        Bounds menubar_bounds = window_bounds(context.win);
                        if ( !menubar_bounds.contains(pos) &&  !pos_in_popup_menu(pos, state.popup ) ) {
                             // if click not in menu bar and not in current menu we can dismiss the menu popup.
                            in_menu = false;
                            // todo we may want to put this handled click back on the
                            // input buffer for upstream processing, e.g., user has the
                            // menubar displaying a menu, then clicks a text input box in
                            // another part of the Layout. We want to cancel the dropdown,
                            // but maybe give the input box a chance to process the click?
                            break;
                        }

                        if ( pos_in_popup_menu(pos, state.popup) ) {
                            // if the click is on a selectable item, make it the current item and dismiss the menu

                            // get the y pos of the click in the subwindow. That will be the item index.
                            // if selectable, select that item and dismiss the menu popup
                            Coordinate win_pos = { pos.x, pos.y};
                            if (wmouse_trafo( state.popup.subwindow(), &win_pos.y, &win_pos.x, false )) {
                                ITEM ** items = menu_items( state.popup.menu() );
                                ITEM * item = items[win_pos.y];
                                log_printf("KEY_MOUSE: item pos.y = %d, name=%s\n",win_pos.y, item_name(item));
                                if (item_opts(item) & O_SELECTABLE ) {
                                    set_current_item( state.popup.menu(), item);
                                    blink_selected_menu_item( state.target_menu());
                                    in_menu = false;
                                    break;
                                }
                            }

                        } else {
                            // click in menu bar.
                            int clicked_menu_index = menu_index_for_pos(pos, mb);
                            if ( clicked_menu_index == -1 ) {
                                // in menu bar but not on any menu title
                                log_printlnf("clicked_menu_index == -1");
                                in_menu = false;
                                break; // no menu was clicked on
                            }
                            if  ( state.menu_index !=  clicked_menu_index) {
                                // only draw a new popup if it's not the current popup
                                // click on a menu title
                                log_printlnf("menubar click : menu_index == %d", clicked_menu_index);

                                // switch to a different popup menu
                                hide_panel(target_panel);
                                update_panels();
                                doupdate();

                                state = dropdown_state::state_for_menu(clicked_menu_index, mb, pm);
                                highlight_menubar_title(context, mb, menu_index);

                                // replaces the panel's window with a new one
                                replace_panel( target_panel, state.menu_window() );

                                display_dropdown(target_panel, state.target_menu());
                                break;
                            }
                        }
                    }
                }
            }
            break;



            // todo - if mouse is moved in the menubar dismiss the current popup and display the next menu?
            // I'm leaning towards no. Extra complications for not much benefit.

        } // switch(menu_ch) {

        // 5. CRITICAL: Redraw the panel z-stack to render the new highlight
        update_panels();
        doupdate();
    } // while(in_menu) {

    // 6. Tear down the modal view and restore the background perfectly
    hide_panel(target_panel);
    highlight_menubar_title(context, mb, -1); // remove any highlighted title
    update_panels();
    doupdate();

    del_panel(target_panel);
}




// void activate_dropdown(MenuModel *menus, MenuBarModel& mb) {}

// enters a read loop to process interaction with the menu bar and menus.
// Returns when a menu is dismissed
// and returns the selected item, if any.
bool process_menubar_click( const WidgetContext& context, MenuBarModel& mb, const MEVENT &event, MenuPopupManager& pm) {
    log_printlnf("You clicked in the menu bar at r:%d c:%d", event.y, event.x);
    bool handled = false;
    int menu_index = menu_index_for_pos({ .x = event.x, .y = event.y}, mb);
    MenuBarView::draw(context, mb); // first draw menubar with no highlighting
    wnoutrefresh( context.win );
    if (menu_index != -1) {
        // click in a menu bar title
        log_printlnf("You clicked on the %s menu title at r:%d c:%d, menu_index=%d.",
            mb.titles[menu_index].text.c_str(), event.y, event.x, menu_index);
        const int menu_id =  mb.menus[menu_index].id;
        log_printlnf("menu_id=%d", menu_id);

        if (pm.contains_id(menu_id)) {
            const MenuPopup & popup = pm.popup_for_id(menu_id);
            highlight_menubar_title(context, mb, menu_index);
            // pm.activate_menubar_popup(
            //     menu_index, mb,
            //     {
            //         .x = mb.titles[menu_index].range.start - 1,
            //         .y = 0
            //     }
            // );
            activate_menubar_menu_dropdown( context, menu_index, mb, pm);
        }

        handled = true;
    }
    // todo return the result of the menu click, an action object, lambda, etc.
    return handled;
}

} // namespace mazes_n_things
