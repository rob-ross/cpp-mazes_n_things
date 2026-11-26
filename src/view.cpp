//  view.cpp
//
//  Created by Rob Ross on 11/20/26.
//
//  Copyright (c) 2026.  All rights reserved.

/*

Terms:

Layout : a group of regions on a screen. In a GUI framework, a "layout" is a relationship among UI widgets that
            define how they are spatially related. There may be many layouts in one window.
            In a ncurses terminal screen, the UI widgets are WINDOW objects. There is a single screen.
            We need a term to label multiple regions on a screen, which we'll call a Layout for now.
            A layout encompasses the entire terminal screen. It is the same size as stdscr, although we don't draw
            to stdscr. A program has at least one Layout, and possibly more. Only one Layout is displayable at a time.
            Thus we have Layout states Active and Offscreen. The active Layout is visible and changes to the layout
            are reflected in the terminal screen via drawing commands and refresh().
            A layout contains one or more windows. A simple Layout may have a single WINDOW the size of the screen.
            Or it may be divided into regions, for a menubar, one or more status bars at the bottom or sides,
            text areas, scrolling regions, etc.
            Since a single program may have more than one Layout, there is a mechanism for the Active layout to become
            dormant, or not displayed, and another Layout becomes Active. Thus we need a Layout manager that keeps track
            of all the Layouts in the app. (Perhaps we can make use of Panels for this, or perhaps they're not needed
            since the Layouts won't overlap. They are only displayed one at a time and fill the entire screen.
            A simple list of Layouts may suffice.

Layout display state: Active vs Dormant

Region : a non-overlapping portion of a Layout. They are smaller display areas on a layout. Like a Menubar region which
            displays the menu titles, a status bar region that displays some information, information widgets like
            check boxes, radio buttons, scrolling lists, popup menus, viewports, labels, text input.

Viewport: a Region that displays a portion of its contents at a time. THe contents can be larger than the viewport region,
            and the user manipulates the viewport via controls to scroll the viewport contents. Think of a  terrain
            map larger than even the terminal screen. Only part of that map is displayed in the viewport, and the user
            can navigate up, right, down, or left to scroll more of the map into view as the current view scrolls out.

The regions in a Layout have a spatial relationship. They are positioned at perticular coordinates in the Layout and have
a 2D extent. They have an origin point, and width and height. Regions may be moveable, growable, or shrinkable as
required. Think of a split text display with a control for moving the middle boundary to the left or right, thus one
region grows horizontally while the other shrinks. Or a tool window palette. It's basically a popup menu that stays on
the screen. If allowed to move, the origin can change as the user moves it around the screen, and the bounds can change
if they grow or shrink it.

Special consideration needs to be taken when the terminal screen changes size, when the user resizes its application
window. Some regions maybe be able to automatically grow or shrink in response, to take up the extra size created or
to get smaller based on less room. But some regions may not be resizeable, or only shrinkable to a certain amount. After
which the Layout should stop rendering that region until the screen is large enough to accommodate it again.
It's in the nature of the API that we don't have the same fidelity to lay out related components as in a full featured
GUI toolkit. So we probably won't neet the sophistication of say, Grid bag layout, etc.

So a Layout has a root WINDOW associated with it. And all other regions lie within this window. For simplicity, every
region is just a non-overlapping rectangle on the root Layout window. Each Region will probably have a WINDOW associated
with it. Maybe not directly. We've identified a handful of different UI widgets, so the Region will probably be associated
with a widget. That widget will be associated with a WINDOW. For now, it seems logical to make all Region windows on a
Layout be subwindows of the root window. They all share the same character buffer, and they don't overlap. This will
probably simplify and optimize the drawing code.

Any widget that does overlap other widgets, like popup menus, dialog boxes, tool palettes etc., is not a Region.
They will have to be Panels since they overlap and need a z-order. So at any time, we have the root window layer at the
bottom, and 0 or more floating layers above it in which panels can be drawn.

Distinct from the Layout states of Active or Dormant, are Region states

Region States: Focused, Editable, Disabled, Invisible, Animating,

Editable: the widget allows changing its data: checking a checkbox, clicking a radio button, entering text, etc.
Non-editable: the widget only displays information: a label, a status indicator.

Disabled: when a normally editable widget is currently not allowing changes/edits to be made to it

Invisible: based on some context state, some widgets may be displayed while others are not. When they are not displayed,
            they are invisible.

Animating : the widget may be undergoing some transition effect. This state is orthogonal to the others.
            examples: the selected text in a region may be blinking on/off to indicate it has been/will be changed
            a viewport may be auto-scrolling to a particular offset
            a widget may be moving across the screen to a new location
            a widget may be resizing itself and showing the progress in real time
            a progress bar that is being updated.
            Animations need their own design considerations. They introduce transition states . There may be threading
            contention to consider. Ex, one thread is generating data for the animating thread to display in real time.

Focused : only one region can be focused at a time. This is the region that receives input events like key presses or
            mouse button clicks, drags, moves, etc. The focused region should be clearly identifiable by some visual
            mechanism. Outline, change in outline, color, textual label, etc. Subtle color animation change? Slow
            gradient change? Slight blink, etc. Or just a different highlight color when focused.

The Layout needs to keep track of the Focused region, and handle navigating between regions. There must be a focus
order, so when the user hits Tab it focuses the next region in the focus list, or shift-tab focuses the previous
region in the focus list. Disabled or invisible or non-editable regions can't get the focus.

Event handling: we'll use a simple model. The focused region is given a chance to handle a key or mouse event. If it
doesn't handle it, then the Layout has a chance. If it doesn't there will be a final global handler. This will handle
for example Quit events. If the global handler doesn't handle the event, the event is dropped. Maybe logged.



*/

#include "view.h"

#include <ncurses.h>
#include <vector>

#include "ncutils.h"

namespace mazes_n_things {

Layout make_layout() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    // new window same size as stdscr, i.e, the size of the Terminal window screen.
    WindowPtr root_win{newwin(rows, cols, 0, 0)};
    Layout new_layout{ .root_window = std::move(root_win) };
    return new_layout;
}



} // namespace mazes_n_things
