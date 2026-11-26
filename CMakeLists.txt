cmake_minimum_required(VERSION 3.28)

project(mazes_n_things VERSION 0.1.0 DESCRIPTION "Portable C++23 maze experiments" LANGUAGES C CXX)

if(WIN32)
    message(FATAL_ERROR "Windows is not supported yet; use macOS or Linux.")
elseif(NOT APPLE AND NOT UNIX)
    message(FATAL_ERROR "This project currently supports only macOS and Linux.")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Define semantic paths to avoid confusion with CMAKE_CURRENT_SOURCE_DIR
# CMakeList.txt considers its current directory to be "CMAKE_CURRENT_SOURCE_DIR", which is confusing as there is
# normally a nested 'src' directory at this level. So we create a variable 'LIB_ROOT_DIR' to serve as the
# name of the project root directory
set(PROJ_ROOT_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJ_INCLUDE_DIR ${PROJ_ROOT_DIR}/include)
set(PROJ_SRC_DIR     ${PROJ_ROOT_DIR}/src)
set(PROJ_TEST_DIR    ${PROJ_ROOT_DIR}/test)


# -fsanitize=leak doesn't work on some Mac OS.... todo investigate
# -fsanitize=address -fsanitize=leak
#set(SAN_FLAGS "-fsanitize=address" )
#if(APPLE
#        AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
#        AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
#    message(STATUS "Disabling AddressSanitizer for GCC on Apple Silicon: libasan is unavailable")
#    set(SAN_FLAGS "")
#elseif(APPLE)
#    set(SAN_FLAGS "-fsanitize=address -fsanitize=leak")
#else()
#    set(SAN_FLAGS "")
#endif()

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    message(STATUS "Disabling AddressSanitizer on Apple Silicon: libasan is unavailable")
    set(SAN_FLAGS "")
elseif (APPLE)
    message(STATUS "Enabling AddressSanitizer on Apple Intel")
    set(SAN_FLAGS "-fsanitize=address -fsanitize=leak")
else()
    message(STATUS "Disabling AddressSanitizer on this platform")
    set(SAN_FLAGS "")
endif()

# Helpful for debugging (CMake usually does some of this in Debug, but explicit is fine)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SAN_FLAGS} \
    -O0 -g -D_XOPEN_SOURCE_EXTENDED=1 \
    -Wall -Wpedantic -Wextra -Werror -Wshadow -Wimplicit-fallthrough \
    -fno-omit-frame-pointer \
    -Wno-unknown-attributes \
    -Wno-unused-const-variable \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
")


include(cmake/ncurses.cmake)
include(cmake/roblib.cmake)

add_executable(mazes_n_things
        src/main.cpp
        src/entry_point_01.cpp
        src/ncutils.cpp
        src/view.cpp
        src/menus.cpp
        src/keys.cpp
        src/colors.cpp
        src/widget.cpp

)
target_include_directories(mazes_n_things PRIVATE ${PROJ_INCLUDE_DIR})
target_include_directories(mazes_n_things PRIVATE ${ROBLIB_INCLUDE_DIR})
target_link_libraries(mazes_n_things PRIVATE c_roblib ${NCURSES_TARGET})

include(CTest)
if(BUILD_TESTING AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
