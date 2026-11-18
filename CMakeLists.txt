cmake_minimum_required(VERSION 3.25)

project(mazes_n_things VERSION 0.1.0 DESCRIPTION "Portable C++23 maze experiments" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mazes_n_things src/main.cpp)
target_include_directories(mazes_n_things PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()