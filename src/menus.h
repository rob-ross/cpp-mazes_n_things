//  menu.h
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

//

#ifndef MAZES_N_THINGS_MENU_H
#define MAZES_N_THINGS_MENU_H

#include <ncurses.h>
#include <menu.h>
#include <panel.h>

#include <vector>
#include <string>
#include <unordered_map>

#include "ncutils.h"
#include "widget.h"

namespace mazes_n_things {
class MenuPopupManager;
class MenuPopup;
struct WidgetContext;


struct MenuItemModel {
    std::string name;
    std::string desc;
    int         action_id{};  // this will let us associate a menu item with an action
    int         shortcut{};

};

struct MenuModel {
    int id{}; // must be unique
    std::string title;
    std::vector<MenuItemModel> items;

};



struct MenuBarModel {
    std::vector<TextRange> titles{};
    // todo this might need to be vector<MenuModel&>
    std::vector<MenuModel>  menus{};
    int             selected_menu{};
};

struct MenuBarView {
    static void draw( const WidgetContext& context, const MenuBarModel& model) ;
};

bool process_menubar_click( const WidgetContext& context, MenuBarModel& mb, const MEVENT &event, MenuPopupManager& popup_manager);

struct MenuBarController {
    explicit MenuBarController(MenuPopupManager& pm) : popup_manager{pm} {};

    static void handle_key( MenuBarModel& model,  int key) {

    }

    bool handle_mouse_event( const WidgetContext& context, MenuBarModel& model, const MEVENT &event ) const {
        bool handled = false;
        // for now, we just handle clicks in the menubar.
        if (context.bounds.contains( {.x = event.x, .y = event.y})) {
            if (event.bstate & ( BUTTON1_PRESSED | BUTTON1_CLICKED)) {
                handled =  process_menubar_click(context, model, event, popup_manager);
            }
        }
        return handled;
    }

private:
    MenuPopupManager& popup_manager;
};

void activate_menubar_menu_dropdown(const WidgetContext &context,
                                        int menu_index, MenuBarModel &mb, MenuPopupManager &pm);
void draw_menu_separators(MENU *menu) ;

class MenuPopup {
public:
    MenuPopup(MENU * menu, MenuModel& model, Coordinate origin)
        : menu_{menu}, model_{model}, origin_{origin} {}


    ~MenuPopup() {
        // todo order??
        // at the moment, when a MenuPopup is created and added to the MenuPopupManager,
        // it has effectively static storage duration. It lives the entire program life.
        // so this destructor is not needed *in this case.* However, we want to make this more
        // portable and robust so we'll have to circle back to this. We'll need to use
        // shared_ptr for these ncurses objects we manage.
        unpost_menu(menu_);
        // we need to wrap these pointers in shared/unique pointers.
        // WINDOW * win = window();
        // WINDOW * sub = subwindow();
        // ITEM ** items = menu_items(menu_);
        // free_menu(menu_);
        // free(items);
        // delwin(sub);
        // delwin(win);
    }


    void show( Coordinate pos = { .x = 0, .y = 0 }) {
        post_menu(menu_);
        // create the panel for displaying the menu
        target_panel_ = new_panel(window());
        // Bring the panel to the top and draw it
        show_panel(target_panel_);
        top_panel(target_panel_);
        // Ensure separator rules overwrite default item text before display
        draw_menu_separators(menu_);

        update_panels();
        doupdate();

    }
    void hide();
    void handle_input(int key);

    [[nodiscard]] Coordinate origin() const {
        return origin_;
    }

    [[nodiscard]] MENU * menu() const {
        return menu_;
    }

    [[nodiscard]] WINDOW * window() const {
        return menu_win(menu_ );
    }

    [[nodiscard]] WINDOW * subwindow() const {
        return menu_sub(menu_);
    }

private:
    MENU*   menu_;
    MenuModel model_;
    Coordinate origin_;
    PANEL *target_panel_{nullptr};


};

class MenuPopupManager {
public:

    void add_popup(int id, MenuPopup popup) {
        popup_menus.emplace(id, popup);
    }

    MenuPopup popup_for_id(int id) {
        return popup_menus.at(id);
    }

    [[nodiscard]] bool contains_id(const int id) const {
        return popup_menus.contains(id);
    }

    void activate_menubar_popup(int id, MenuBarModel& mbm, Point pos) {
        MenuPopup mp = popup_for_id(id);
        mp.show(pos);


    }

    void activate_popup(int id, Point pos);

private:
    std::unordered_map<int, MenuPopup> popup_menus;
};

// todo should this be a member function of MenuPopupManager? I think so.
MENU * create_menu(const std::vector<MenuItemModel> &item_data, Coordinate origin);



} //mazes_n_things

#endif //MAZES_N_THINGS_MENU_H
