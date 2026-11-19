include(ExternalProject)

set(ROBLIB_SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/c-roblib/source")
set(ROBLIB_INSTALL_DIR "${CMAKE_BINARY_DIR}/dependencies/c-roblib/install")
set(ROBLIB_INCLUDE_DIR "${ROBLIB_INSTALL_DIR}/include")
set(ROBLIB_REPOSITORY "https://github.com/rob-ross/c-roblib.git" CACHE STRING "c-roblib Git repository")
set(ROBLIB_TAG "main" CACHE STRING "c-roblib Git branch, tag, or commit")
option(ROBLIB_CHECK_FOR_UPDATES "Check the c-roblib remote for a newer revision during configure" ON)

set(ROBLIB_UPDATE_DISCONNECTED TRUE)
if(ROBLIB_CHECK_FOR_UPDATES AND EXISTS "${ROBLIB_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git -C "${ROBLIB_SOURCE_DIR}" rev-parse HEAD
        RESULT_VARIABLE ROBLIB_LOCAL_REVISION_RESULT
        OUTPUT_VARIABLE ROBLIB_LOCAL_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND git ls-remote "${ROBLIB_REPOSITORY}" "${ROBLIB_TAG}"
        RESULT_VARIABLE ROBLIB_REMOTE_REVISION_RESULT
        OUTPUT_VARIABLE ROBLIB_REMOTE_REVISION_LINE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(ROBLIB_LOCAL_REVISION_RESULT EQUAL 0 AND
       ROBLIB_REMOTE_REVISION_RESULT EQUAL 0 AND
       ROBLIB_REMOTE_REVISION_LINE MATCHES "^([0-9a-fA-F]+)[ \t]")
        if(NOT ROBLIB_LOCAL_REVISION STREQUAL CMAKE_MATCH_1)
            set(ROBLIB_UPDATE_DISCONNECTED FALSE)
            message(STATUS "A newer c-roblib revision is available; it will be updated.")
        endif()
    endif()
endif()

# CMake validates imported include directories during generation, before the
# external project has had a chance to install its headers.
file(MAKE_DIRECTORY "${ROBLIB_INCLUDE_DIR}")

ExternalProject_Add(roblib_external
    GIT_REPOSITORY "${ROBLIB_REPOSITORY}"
    GIT_TAG "${ROBLIB_TAG}"
    GIT_SHALLOW TRUE
    SOURCE_DIR "${ROBLIB_SOURCE_DIR}"
    BINARY_DIR "${CMAKE_BINARY_DIR}/dependencies/c-roblib/build"
    INSTALL_DIR "${ROBLIB_INSTALL_DIR}"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target c_roblib
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR>
    BUILD_BYPRODUCTS "${ROBLIB_INSTALL_DIR}/lib/libc_roblib.a"
    UPDATE_DISCONNECTED ${ROBLIB_UPDATE_DISCONNECTED}
)

add_library(c_roblib STATIC IMPORTED GLOBAL)
set_target_properties(c_roblib PROPERTIES
    IMPORTED_LOCATION "${ROBLIB_INSTALL_DIR}/lib/libc_roblib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${ROBLIB_INCLUDE_DIR}"
#    INTERFACE_LINK_LIBRARIES CURL::libcurl
)
add_dependencies(c_roblib roblib_external)
