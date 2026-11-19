cmake_minimum_required(VERSION 3.25)

project(mazes_n_things VERSION 0.1.0 DESCRIPTION "Portable C++23 maze experiments" LANGUAGES C CXX)

if(WIN32)
    message(FATAL_ERROR "Windows is not supported yet; use macOS or Linux.")
elseif(NOT APPLE AND NOT UNIX)
    message(FATAL_ERROR "This project currently supports only macOS and Linux.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/ncurses.cmake)

include(cmake/roblib.cmake)

add_executable(mazes_n_things src/main.cpp

)
target_include_directories(mazes_n_things PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mazes_n_things PRIVATE ${ROBLIB_INCLUDE_DIR})
target_link_libraries(mazes_n_things PRIVATE c_roblib ${NCURSES_TARGET})

include(CTest)
if(BUILD_TESTING AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
