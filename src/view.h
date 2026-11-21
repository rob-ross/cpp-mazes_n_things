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

namespace mazes_n_things {


enum class WidgetType {
    NONE, CHECKBOX, RB_GROUP, LABEL, INPUT_TEXT, PAD_VIEWPORT
};

struct Widget {
    WINDOW *    win{};
    WidgetType  type{WidgetType::NONE};
    bool        is_focused{};
    bool        is_editable{};
    bool        is_disabled{};
    bool        is_invisible{};
    bool        is_animating{};
};

struct Region {
    Bounds      bounds{};
    Widget      widget{};
};

struct Layout {
    WINDOW *            root_win{};
    bool                is_active{};
    std::vector<Region> regions{};
};


} // namespace mazes_n_things
#endif //MAZES_N_THINGS_VIEW_H
