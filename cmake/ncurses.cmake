# Find ncurses and companion libraries without assuming a particular platform.
# Discovery prefers Homebrew, then pkg-config, then the platform defaults.

set(NCURSES_CFLAGS "")
set(NCURSES_INCLUDE_DIRS "")
set(NCURSES_LIBRARY_DIRS "")
set(NCURSES_LIBRARIES "")
set(NCURSES_PACKAGE "")

find_program(BREW_EXECUTABLE brew)
if(BREW_EXECUTABLE)
    execute_process(
        COMMAND "${BREW_EXECUTABLE}" --prefix ncurses
        OUTPUT_VARIABLE HOMEBREW_NCURSES_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE HOMEBREW_NCURSES_RESULT
    )
    if(NOT HOMEBREW_NCURSES_RESULT EQUAL 0)
        set(HOMEBREW_NCURSES_PREFIX "")
    endif()
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    set(_ncurses_saved_pkgconfig_path "$ENV{PKG_CONFIG_PATH}")
    if(HOMEBREW_NCURSES_PREFIX)
        set(ENV{PKG_CONFIG_PATH}
            "${HOMEBREW_NCURSES_PREFIX}/lib/pkgconfig:${_ncurses_saved_pkgconfig_path}")
    endif()
    pkg_check_modules(NCURSES_PKG QUIET ncursesw)
    if(NCURSES_PKG_FOUND)
        set(NCURSES_PACKAGE ncursesw)
    else()
        pkg_check_modules(NCURSES_PKG QUIET ncurses)
        if(NCURSES_PKG_FOUND)
            set(NCURSES_PACKAGE ncurses)
        endif()
    endif()
    set(ENV{PKG_CONFIG_PATH} "${_ncurses_saved_pkgconfig_path}")
    if(NCURSES_PKG_FOUND)
        set(NCURSES_CFLAGS "${NCURSES_PKG_CFLAGS}")
        set(NCURSES_INCLUDE_DIRS "${NCURSES_PKG_INCLUDE_DIRS}")
        set(NCURSES_LIBRARY_DIRS "${NCURSES_PKG_LIBRARY_DIRS}")
    endif()
endif()

set(_ncurses_search_dirs "")
if(HOMEBREW_NCURSES_PREFIX AND EXISTS "${HOMEBREW_NCURSES_PREFIX}/lib")
    list(APPEND _ncurses_search_dirs "${HOMEBREW_NCURSES_PREFIX}/lib")
endif()
list(APPEND _ncurses_search_dirs ${NCURSES_LIBRARY_DIRS})

find_library(_ncurses_library NAMES ncursesw ncurses HINTS ${_ncurses_search_dirs})
if(NOT _ncurses_library)
    find_package(Curses REQUIRED)
    set(NCURSES_INCLUDE_DIRS "${CURSES_INCLUDE_DIRS}")
    find_library(_ncurses_library NAMES ncursesw ncurses HINTS ${CURSES_LIBRARY_DIRS})
endif()
if(NOT _ncurses_library)
    message(FATAL_ERROR "Unable to find ncurses or ncursesw")
endif()

get_filename_component(_ncurses_library_dir "${_ncurses_library}" DIRECTORY)
set(NCURSES_LIBRARY_DIRS "${_ncurses_library_dir}")
if(_ncurses_library MATCHES "ncursesw")
    set(_ncurses_suffix w)
    set(NCURSES_PACKAGE ncursesw)
else()
    set(_ncurses_suffix "")
    set(NCURSES_PACKAGE ncurses)
endif()

if(NOT DEFINED NCURSES_EXTRA_LIBRARIES)
    set(NCURSES_EXTRA_LIBRARIES menu panel)
endif()
set(NCURSES_LIBRARIES "")
foreach(_ncurses_extra IN LISTS NCURSES_EXTRA_LIBRARIES)
    find_library(_ncurses_extra_library_${_ncurses_extra}
        NAMES "${_ncurses_extra}${_ncurses_suffix}"
        HINTS "${_ncurses_library_dir}")
    if(NOT _ncurses_extra_library_${_ncurses_extra})
        message(FATAL_ERROR "Required ncurses companion library '${_ncurses_extra}${_ncurses_suffix}' was not found")
    endif()
    list(APPEND NCURSES_LIBRARIES "${_ncurses_extra_library_${_ncurses_extra}}")
endforeach()
list(APPEND NCURSES_LIBRARIES "${_ncurses_library}")

add_library(project_ncurses_complete INTERFACE)
target_include_directories(project_ncurses_complete INTERFACE ${NCURSES_INCLUDE_DIRS})
target_compile_options(project_ncurses_complete INTERFACE ${NCURSES_CFLAGS})
target_link_libraries(project_ncurses_complete INTERFACE ${NCURSES_LIBRARIES})
set(NCURSES_TARGET project_ncurses_complete)

message(STATUS "ncurses package: ${NCURSES_PACKAGE}")
message(STATUS "ncurses include dirs: ${NCURSES_INCLUDE_DIRS}")
message(STATUS "ncurses library dirs: ${NCURSES_LIBRARY_DIRS}")
message(STATUS "ncurses libraries (link order): ${NCURSES_LIBRARIES}")
message(STATUS "ncurses compiler flags: ${NCURSES_CFLAGS}")
