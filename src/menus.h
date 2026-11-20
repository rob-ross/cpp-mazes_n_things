//  menu.h
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

//

#ifndef MAZES_N_THINGS_MENU_H
#define MAZES_N_THINGS_MENU_H

#include <list>
#include <ncurses.h>
#include <menu.h>
#include <string>

#include "ncutils.h"

struct MenuModel {
    int           index{};
    MENU *        menu{};  // contains the menu ITEMs
    std::string   menu_title{};
    HalfOpenRange menubar_pos{}; // start and end column (x) positions of each menu title
} ;

struct MenuBar {
    WINDOW *                  menubar_win{};
    int                       menu_count{};
    std::list<std::string>    menu_titles{};
    std::list<MenuModel>      menus{};
    std::list<HalfOpenRange>  menu_title_x_positions{};
} ;

constexpr int ITEM_NAME_MAX_LEN = 80;
constexpr int ITEM_DESC_MAX_LEN = 20;
struct ItemNameDesc{
    std::string name;
    std::string des;
    unsigned    control_shortcut; // currently only for CTRL+key combinations
};

// index constants for color methods. These don't define colors, they just reserve indexes.
constexpr int COLOR_GREY         = 10;
constexpr int COLOR_DARK_GREY    = 11;
constexpr int COLOR_LIGHT_GREY   = 12;
constexpr int COLOR_LIGHTER_GREY = 13;
constexpr int COLOR_DARKER_GREY  = 14;

constexpr int CPI_MENU_BAR            = 10;
constexpr int CPI_MENU_BAR_HIGHLIGHT  = 11;
constexpr int CPI_MENU_BODY           = 12;
constexpr int CPI_MENU_BODY_HIGHLIGHT = 13;
constexpr int CPI_MENU_BODY_DISABLED  = 14;
constexpr int CPI_MAIN_WIN            = 15;

constexpr unsigned CP_MENU_BAR = COLOR_PAIR(CPI_MENU_BAR);
constexpr unsigned CP_MENU_BAR_HIGHLIGHT = COLOR_PAIR(CPI_MENU_BAR_HIGHLIGHT);

constexpr unsigned CP_MENU_BODY = COLOR_PAIR(CPI_MENU_BODY);
constexpr unsigned CP_MENU_BODY_HIGHLIGHT = COLOR_PAIR(CPI_MENU_BAR_HIGHLIGHT);
constexpr unsigned CP_MENU_BODY_DISABLED = COLOR_PAIR(CPI_MENU_BODY_DISABLED);
constexpr unsigned CP_MAIN_WIN = COLOR_PAIR(CPI_MAIN_WIN);




#endif //MAZES_N_THINGS_MENU_H
