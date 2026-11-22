//  widget.h
//
//  Created by Rob Ross on 11/21/26.
//
//  Copyright (c) 2026.  All rights reserved.

/*

Design overview:

Layout owns regions and focus.
Region owns Bounds, an IWidget, and the associated drawing window.
WidgetContext temporarily communicates those values during rendering.
View renders within the supplied rectangle.
Widget delegates to its view and controller.

MenuBarModel, TextInputModel, etc.: pure state.
Widget views: ncurses rendering and ncurses resource adapters.
Widget controllers: input-to-model logic.
*/

#ifndef MAZES_N_THINGS_WIDGET_H
#define MAZES_N_THINGS_WIDGET_H

#include <ncurses.h>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "ncutils.h"

namespace mazes_n_things {
struct TextRange;

using Point = Coordinate;

// TODO context needs to support more than just WINDOW. e.g., MENU, maybe PAD, FORM, PANEL?
// maybe use variants for this?
struct WidgetContext {
    Bounds bounds;
    WINDOW* win{};
};

struct IWidget {
    virtual ~IWidget() = default;
    virtual void handle_key(int key) = 0;
    virtual void draw(const WidgetContext&) = 0;
};

struct WindowDeleter {
    void operator()(WINDOW* window) const noexcept {
        if (window != nullptr) {
            delwin(window);
        }
    }
};

using WindowPtr = std::unique_ptr<WINDOW, WindowDeleter>;

template<class Model, class View, class Controller>
class Widget final : public IWidget {
public:
    Model model;
    View view;
    Controller controller;
    // public for now. Should these be access methods?
    bool is_focused{};
    bool is_editable{};
    bool is_disabled{};
    bool is_invisible{};
    bool is_animating{};

    Widget() = default;

    Widget(const Model &model, const View &view, const Controller &controller)
        : model(model),
          view(view),
          controller(controller) {
    }

    void handle_key(int key) override {
        controller.handle_key(key, model);
    }

    void draw(const WidgetContext& context) override {
        view.draw(context, model);
    }
};

struct Region {
    WindowPtr window;
    std::unique_ptr<IWidget> widget;
    Bounds bounds{};

    Region(WindowPtr win, std::unique_ptr<IWidget> w, const Bounds region_bounds)
        : window(std::move(win)), widget(std::move(w)), bounds(region_bounds) {
    }

    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;
    Region(Region&&) noexcept = default;
    Region& operator=(Region&&) noexcept = default;

    [[nodiscard]] bool contains(Point p) const {
        return bounds.contains(p);
    }
};

struct Layout {
    WindowPtr root_window;
    std::vector<Region> regions{};
    std::size_t focused_region{};
    bool is_active{};

    void handle_key(int key) {
        if (focused_region < regions.size() && regions[focused_region].widget)
            regions[focused_region].widget->handle_key(key);
    }

    void draw() {
        for (auto& region : regions){
            if (region.widget) {
                region.widget->draw({ .bounds = region.bounds, .win = region.window.get() } );
            }
        }
    }
};


// -----------------------------------------------------------------
//   MENUS
// -----------------------------------------------------------------

struct MenuItemModel {
    std::string name;
    std::string description;
    int shortcut{};
};

struct MenuModel {
    std::string title;
    std::vector<MenuItemModel> items;
    std::size_t selected_item{};
};

struct MenuBarModel {
    std::vector<TextRange> titles{};
    std::vector<MenuModel>  menus{};
    int             selected_menu{};
};

struct MenuBarView {
    static void draw(const WidgetContext& context, const MenuBarModel& model) ;
};

struct MenuBarController {
    static void handle_key(int key, MenuBarModel& model) {

    }
};



} // namespace mazes_n_things

#endif //MAZES_N_THINGS_WIDGET_H

/*
Questions:
2. I could not get your code to work when I was using a unique_ptr in Region for the Widget. I had to revert to a
    raw pointer. There are several issues I have. First is not completely understanding the lifecycle and copy
    mechanisms on C++. I don't know what containers copy elements, or if they do a deep or shallow copy. I want to
    factor the Layout creation code into another function called from main(). But that introduces a big memory complication.
    The Layout will become a large graph with many Regions and Widgets all with their various state models and ncurses
    view components. So if I create this as I had previously in make_layout(), I was returning a Layout by value.
    I don't even know how that would work with a complicated object graph. Would my Layout object be completely copied
    on the stack for the caller to use after the method call? Or would it leave dangling pointers in the graph?
    And even if it *does* do deep copies, it's incredibly wasteful of stack space and inefficient.
    So I'm not sure what the best implementation is for a layout creation function. I currently create an empty Layout
    in main() and pass it by reference to init_layout() which populates it. I'm still doing copies I think when I push_back
    a Region on the vector, with its complex object graph of a Widget with 3 sub objects, etc.

    I could implement it as a "create_layout" function, and create everything on the heap and store everything by pointer
    in the various MVC structures. (Raw pointers for now, which I would need to migrate to smart pointers for
    memory management). Then I wouldn't need to worry about copying. Everything is created once on the heap and the same
    graph is returned to the caller. But, that's a lot of little allocated objects to keep track of. This is another
    reason why I need to learn more about object lifecycles in C++, so I can pick the best path of stack vs heap
    creation to take advantage of the automagical copying of C++ behind the scenes.



*/
