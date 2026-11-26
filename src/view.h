//  view.h
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.


#ifndef MAZES_N_THINGS_VIEW_H
#define MAZES_N_THINGS_VIEW_H

#include <ncurses.h>
#include <vector>

#include "ncutils.h"
#include "widget.h"

namespace mazes_n_things {


enum class WidgetType {
    NONE, MENUBAR, CHECKBOX, RB_GROUP, LABEL, INPUT_TEXT, PAD_VIEWPORT
};









// make a new Layout with the same size as the current stdscr and return by value.
Layout make_layout();

} // namespace mazes_n_things
#endif //MAZES_N_THINGS_VIEW_H
