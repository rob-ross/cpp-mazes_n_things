//  widget.cpp
//
//  Created by Rob Ross on 11/21/26.
//
//  Copyright (c) 2026.  All rights reserved.

#include "widget.h"

namespace mazes_n_things {




void MenuBarView::draw(const WidgetContext& context, const MenuBarModel& model) {
    WINDOW * menubar_win = context.win;
    werase(menubar_win);
    for (const auto& title : model.titles) {
        wmove(menubar_win, 0, title.range.start);
        wprintw(menubar_win, "%s", title.text.c_str());
    }
    wnoutrefresh(context.win);
}

} // namespace mazes_n_things
