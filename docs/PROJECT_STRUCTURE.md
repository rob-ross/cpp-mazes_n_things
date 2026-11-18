# Project Structure

This document describes the intended source layout for `mazes_n_things`. It is
the detailed companion to the repository-level [`AGENTS.md`](../AGENTS.md)
blueprint. Update it when a directory gains a new responsibility or a major
architectural decision changes the layout.

## Current layout

```text
.
├── CMakeLists.txt             # Root build configuration and project targets
├── AGENTS.md                  # AI-facing project goals and constraints
├── docs/                      # Design, architecture, and development notes
│   └── PROJECT_STRUCTURE.md   # This document
├── include/                   # Public C++ headers owned by this project
├── src/                       # Application and private implementation code
│   └── main.cpp               # Minimal executable entry point
└── tests/                     # Test targets and test sources
    └── CMakeLists.txt         # Test-specific build configuration
```

The `include/` directory is intentionally present even though the initial
executable has no public interface yet. Headers should normally use a project
namespace subdirectory as the public API grows, for example
`include/mazes_n_things/...`.

## Planned build and dependency locations

Generated files must not be committed to the source tree. Configure builds in a
separate directory such as `build/` or `cmake-build-debug/`.

The external C library should eventually be isolated behind a dedicated CMake
dependency area. Its exact directory names will be documented here when the
library acquisition and build workflow is implemented. The intended shape is:

```text
<build directory>/
└── dependencies/<library>/  # Locally built library and exported headers
```

Platform-specific acquisition and build logic belongs in `cmake/` once needed.

## Directory responsibilities

- `src/`: Private implementation and executable code.
- `include/`: Headers intended for use by other targets; do not place generated dependency headers here.
- `tests/`: Reproducible tests and their CMake targets.
- `docs/`: Human- and AI-readable architecture and development documentation.
- `cmake/` (planned): CMake modules, capability checks, toolchain helpers, and dependency integration.
- `third_party/` (planned): Vendored source only when a dependency cannot use the configured build workflow; never generated output.

## Evolution rules

1. Add a focused subdirectory when a component has its own source, headers, and tests.
2. Isolate platform-specific implementations and record new requirements in `AGENTS.md` or `docs/`.
3. Keep generated build products and locally built dependencies outside tracked source directories.
4. Add tests with non-trivial behavior and keep the root `CMakeLists.txt` focused on composing targets.