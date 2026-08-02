---
name: openharmony-build
description: Build OpenHarmony targets in the local ~/openharmony checkout for rk3568 using ./build.sh. Use when Codex needs to compile or verify OpenHarmony modules, shared libraries, executables, or test suites, especially ability_runtime cli_tool_framework targets, with one or more --build-target values.
---

# OpenHarmony Build

## Command

Run builds from the OpenHarmony source root:

```bash
cd /data/home/y30056898/openharmony
./build.sh -p rk3568 --build-target <target1> [target2 ...] -j64 --ccache
```

Use the user-provided target values as the only part that normally changes.

When building multiple targets in one command, list them after `--build-target` separated by spaces:

```bash
./build.sh -p rk3568 --build-target <target1> <target2> -j64 --ccache
```

## Target Format

Prefer full GN-style target paths:

```text
path/to/module:target_name
```

Examples:

```bash
./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/cli_tool_framework/test/unittest:unittest -j64 --ccache
./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/cli_tool_framework/services/climgr:climgr -j64 --ccache
```

The first example builds all tests under `foundation/ability/ability_runtime/cli_tool_framework/test/unittest`.
The second example builds the `climgr` shared library target.

## Usage Notes

- Use this command form instead of older build commands that omit `-j64 --ccache`.
- Keep all requested targets in a single `build.sh` invocation when practical.
- If the current working directory is inside the repo, still run the command from `/data/home/y30056898/openharmony` so relative build paths resolve consistently.
- If the build fails, report the first meaningful compiler, linker, or GN error. Do not treat tool setup warnings as the cause unless they stop the build.

## Recovery for GN or Output-Tree Problems

When a targeted build fails before compiling the requested target because of stale generated files, broken GN state, missing innerapi metadata, duplicate ninja rules, or other repo/output-tree inconsistencies, perform this recovery sequence before retrying the build:

1. From `/data/home/y30056898/openharmony`, remove the output tree:

```bash
rm -rf out
```

2. From `/data/home/y30056898/openharmony`, update all repositories:

```bash
repo sync -c -d --no-tags
```

3. Return to the working repository, update `master`, check out the working branch, and rebase it onto the newest `master`. Resolve conflicts if they occur.

4. Rerun the original build command from `/data/home/y30056898/openharmony`.
