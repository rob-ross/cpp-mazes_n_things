# Project Blueprint

## Purpose

This repository will contain a portable C++23 project intended for publication on GitHub. The initial development environment is macOS on x86_64, but the build should be designed to support macOS on both x86_64 and ARM, Linux, and—over time—Windows.

## Platform and dependency goals

- Use C++23.
- Use POSIX APIs and `ncurses` where appropriate.
- Treat Windows support as an incremental goal: it will require platform-specific adaptations and external library acquisition, especially for POSIX functionality and `ncurses`.
- Preserve portability by isolating platform-specific code and documenting platform requirements as they are introduced.

## External C library

The project depends on a C library maintained separately by the project owner. The build process must eventually:

1. Download or otherwise acquire the library source.
2. Build the library locally as part of this project's build workflow.
3. Place the resulting static `.a` library and public headers in a standard, project-local dependency location.
4. Make that library and its headers available to this project's C++ targets.

The C library has currently only been built and tested on macOS x86_64. Its portability, including macOS ARM compatibility, must be evaluated and improved as this project evolves. Its possible POSIX- or macOS-specific implementation details should be treated as known portability risks.

## Build-system direction

Use a cross-platform CMake-based structure suitable for CLion and command-line builds. The build should make platform capability checks and dependency requirements explicit, avoid committing generated artifacts, and leave room for separate platform toolchains and dependency strategies. Windows support may initially be incomplete, but the build should fail clearly when required platform dependencies are unavailable.

## Current status and next steps

The initial repository layout and CMake skeleton are now established. The detailed layout is documented in [`docs/PROJECT_STRUCTURE.md`](docs/PROJECT_STRUCTURE.md), which should be updated when directories gain new responsibilities or architectural constraints change.

The build now checks for macOS/Linux, rejects Windows explicitly, detects `ncurses` and `CURL`, and fetches `c-roblib` from its `main` branch when no staged copy exists. The external library is built only through its `c_roblib` target and staged under the build tree; its current upstream CMake configuration may still download Google Test during configure, but does not build the library tests. Network access is only needed when the staged archive and headers are absent; pinning a release or commit remains future work.

When important architectural decisions, constraints, or unfinished work arise, update this document or add a clearly linked top-level project document so future AI interactions can use it as authoritative context.