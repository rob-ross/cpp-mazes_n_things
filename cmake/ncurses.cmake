set(NCURSES_TARGET "")

if(APPLE)
    find_program(BREW_EXECUTABLE brew)
    if(BREW_EXECUTABLE)
        execute_process(
            COMMAND "${BREW_EXECUTABLE}" --prefix ncurses
            OUTPUT_VARIABLE HOMEBREW_NCURSES_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE HOMEBREW_NCURSES_RESULT
        )
        if(HOMEBREW_NCURSES_RESULT EQUAL 0 AND EXISTS "${HOMEBREW_NCURSES_PREFIX}")
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                set(_ncurses_pkgconfig_path "${HOMEBREW_NCURSES_PREFIX}/lib/pkgconfig")
                if(DEFINED ENV{PKG_CONFIG_PATH} AND NOT "$ENV{PKG_CONFIG_PATH}" STREQUAL "")
                    set(_ncurses_pkgconfig_path "${_ncurses_pkgconfig_path}:$ENV{PKG_CONFIG_PATH}")
                endif()
                set(_ncurses_saved_pkgconfig_path "$ENV{PKG_CONFIG_PATH}")
                set(ENV{PKG_CONFIG_PATH} "${_ncurses_pkgconfig_path}")
                pkg_check_modules(NCURSES_HOMEBREW QUIET IMPORTED_TARGET ncursesw)
                set(ENV{PKG_CONFIG_PATH} "${_ncurses_saved_pkgconfig_path}")
                if(NCURSES_HOMEBREW_FOUND)
                    set(NCURSES_TARGET PkgConfig::NCURSES_HOMEBREW)
                endif()
            endif()
        endif()
    endif()
endif()

if(NOT NCURSES_TARGET)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)
    add_library(project_ncurses INTERFACE)
    target_include_directories(project_ncurses INTERFACE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(project_ncurses INTERFACE ${CURSES_LIBRARIES})
    set(NCURSES_TARGET project_ncurses)
endif()