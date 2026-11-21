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

#include <vector>
#include <string>

#include "ncutils.h"

namespace mazes_n_things {

struct TextRange {
    std::string text;
    ClosedRange range;
};

struct MenuModel {
    int           index{};
    MENU *        menu{};        // contains the menu ITEMs
    std::string   menu_title{};
    HalfOpenRange menu_title_pos{}; // start and end column (x) positions of each menu title in the menu bar
};


struct MenuBarModel {
    WINDOW *                    menubar_win{};
    int                         menu_count{};
    std::vector<MenuModel>      menus{};
    std::vector<TextRange>      titles;
};

constexpr int ITEM_NAME_MAX_LEN = 80;
constexpr int ITEM_DESC_MAX_LEN = 20;
struct ItemNameDesc{
    std::string name;
    std::string desc;
    unsigned    control_shortcut; // currently only for CTRL+key combinations
};




} //mazes_n_things

#endif //MAZES_N_THINGS_MENU_H
