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
struct Layout;
struct MenuBarModel;
struct TextRange;

using Point = Coordinate;

// TODO context needs to support more than just WINDOW. e.g., MENU, maybe PAD, FORM, PANEL?
// maybe use variants for this?
struct WidgetContext {
    Bounds bounds;
    WINDOW* win{};
    // void * state{nullptr};
};

struct IWidget {
    virtual ~IWidget() = default;
    virtual void handle_key(int key) = 0;
    virtual void draw(const WidgetContext&) = 0;
    virtual bool handle_mouse_event(const WidgetContext& context, MEVENT event ) = 0;
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
        controller.handle_key( model, key );
    }

    bool handle_mouse_event(const WidgetContext& context, MEVENT event) override {
        return controller.handle_mouse_event( context, model, event );
    }

    void draw( const WidgetContext& context) override {
        view.draw(context, model);
    }
};

struct Region {
    WindowPtr window;
    std::unique_ptr<IWidget> widget;
    // Use region.layout.lock() when temporarily accessing the parent, and check whether the resulting shared_ptr is non-null.
    std::weak_ptr<Layout> layout;
    Bounds bounds{};

    Region(
        WindowPtr win,
        std::unique_ptr<IWidget> w,
        std::weak_ptr<Layout> l,
        const Bounds region_bounds )
          : window(std::move(win)),
            widget(std::move(w)),
            layout(std::move(l)),
            bounds(region_bounds) { }

    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;
    Region(Region&&) noexcept = default;
    Region& operator=(Region&&) noexcept = default;

    [[nodiscard]] bool contains(Point p) const {
        return bounds.contains(p);
    }

    [[nodiscard]] WidgetContext make_context() const {
        return { .bounds = bounds, .win = window.get() };
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
                region.widget->draw( region.make_context() );
            }
        }
    }

    [[nodiscard]] bool handle_mouse_event(MEVENT event) const {
        bool handled = false;
        for (auto& region: regions ) {
            if (region.contains( {.x = event.x, .y = event.y } )) {
                if (region.widget) {
                    handled =  region.widget->handle_mouse_event( region.make_context(), event );
                    if (handled) {
                        wnoutrefresh(root_window.get());
                    }
                }
                break;
            }
        }

        return handled;
    }
};


} // namespace mazes_n_things

#endif //MAZES_N_THINGS_WIDGET_H


