cmake_minimum_required(VERSION 3.25)

project(mazes_n_things VERSION 0.1.0 DESCRIPTION "Portable C++23 maze experiments" LANGUAGES CXX)

if(WIN32)
    message(FATAL_ERROR "Windows is not supported yet; use macOS or Linux.")
elseif(NOT APPLE AND NOT UNIX)
    message(FATAL_ERROR "This project currently supports only macOS and Linux.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#find_package(CURL REQUIRED)
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

include(cmake/roblib.cmake)

add_executable(mazes_n_things src/main.cpp
        src/main.cpp)
target_include_directories(mazes_n_things PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mazes_n_things PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(mazes_n_things PRIVATE c_roblib ${CURSES_LIBRARIES})

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
