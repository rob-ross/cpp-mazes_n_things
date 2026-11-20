# Load CMake's dependency-fetching support.
include(FetchContent)

# Keep dependency download progress visible during configuration.
set(FETCHCONTENT_QUIET OFF)
# Build Google Test with the static runtime on supported platforms.
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
# Prevent Google Test installation targets from being generated.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
# Disable Google Mock because the test suite does not use it.
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
# Prevent Google Test's own test targets from being built.
set(gtest_build_tests OFF CACHE BOOL "" FORCE)

# Declare the Google Test source repository and requested version.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
    GIT_SHALLOW TRUE
)

# Download and make Google Test available to the test targets.
FetchContent_MakeAvailable(googletest)

# Test targets will be added alongside the first testable application component.
