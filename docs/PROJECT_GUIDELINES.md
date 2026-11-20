# Project Guidelines

This document records project decisions, conventions, protocols, and standards that should be followed as the project evolves. Update it when an important decision is made or an existing convention changes.

## Git Branch Naming

### General Format

Use the following format:

```text
<type>/<short-description>
```

Branch names should be lowercase, use hyphens to separate words, and describe the work rather than a specific implementation detail. Keep names reasonably short and avoid spaces, punctuation, and personal names.

Examples:

```text
feature/maze-generation
fix/invalid-start-position
refactor/console-rendering
```

### Branch Types

Use these prefixes to communicate the purpose of a branch:

| Prefix | Use for | Example |
| --- | --- | --- |
| `feature/` | A new user-visible capability or significant new behavior | `feature/dfs-maze-generator` |
| `fix/` | A correction to existing behavior | `fix/overflow-in-grid-index` |
| `refactor/` | Internal restructuring without intentionally changing behavior | `refactor/separate-platform-code` |
| `docs/` | Documentation-only changes | `docs/update-build-requirements` |
| `test/` | Adding or improving tests without a corresponding feature or fix | `test/cover-maze-validation` |
| `build/` | Build-system, dependency, or toolchain changes | `build/update-curses-detection` |
| `ci/` | Continuous integration and automation changes | `ci/add-linux-build` |
| `chore/` | Maintenance work that does not fit another category | `chore/update-editor-settings` |
| `release/` | Preparing a versioned release | `release/0.1.0` |
| `integration/` | Combining or stabilizing a major cross-component effort | `integration/c-roblib-support` |

Choose the most specific prefix. For example, use `fix/` for a bug fix that also adds a regression test, rather than creating a separate `test/` branch.

### Working Practices

- Start new work from the current default branch, unless the work explicitly depends on another feature branch.
- Keep a branch focused on one related change or objective.
- Delete merged branches when they are no longer needed.
- Do not commit generated build output, local IDE state, or machine-specific files.
- Branch names should not include issue numbers unless the project adopts an issue-tracking convention that requires them. If issue numbers are later adopted, place them after the prefix, for example `fix/123-invalid-start-position`.

### Reserved Branches

- `main` is the stable integration branch and should remain buildable.
- `develop` is not currently used. Introduce it only if the project adopts a release workflow that requires a long-lived development branch.
- Temporary experimental branches should use `experiment/` and should not be treated as production-ready.
