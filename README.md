# mazes_n_things

Portable C++23 maze experiments built with CMake.

## Requirements

- CMake 3.25 or newer
- A C23 and C++23 compiler
- `ncurses`
- macOS or Linux

Windows is not supported yet. On the first configure, CMake fetches and builds
`c-roblib` from its `main` branch. Later builds reuse the staged dependency in
the build directory unless it is removed.

On macOS, if Homebrew and its keg-only `ncurses` formula are installed, the
build automatically prefers Homebrew's wide-character `ncursesw` through
`pkg-config`. Otherwise it falls back to CMake's portable `Curses` discovery.
The Homebrew path supplies `_DARWIN_C_SOURCE` and the required include and
linker settings without requiring project-wide compiler flags.

## Configure and build

From the project root, configure a Debug build with:

```shell
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
```

Build the project with:

```shell
cmake --build cmake-build-debug
```

For a Release build, use a separate build directory (NOTE: no release build supported yet):

```shell
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
```

The executable is written to the selected build directory as
`mazes_n_things`.

Google Test is fetched only for Debug configurations. CMake caches the
checkout under `cmake-build-debug/_deps/`; it is reused across normal rebuilds
and reconfiguration. Removing `cmake-build-debug` also removes that checkout,
so the next configure will download it again.

## Run

Run the Debug build from the project root with:

```shell
./cmake-build-debug/mazes_n_things
```

For the Release build, use:

```shell
./cmake-build-release/mazes_n_things
```

## Test

Run the configured tests with:

```shell
ctest --test-dir cmake-build-debug --output-on-failure
```

## Clean

To remove one generated build directory, use:

```shell
rm -rf cmake-build-debug
```

For the Release build, use:

```shell
rm -rf cmake-build-release
```

Removing a build directory also removes its locally staged `c-roblib` source,
archive, and headers. The next configure will therefore require network access
to fetch the dependency again.
