<!--
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->
---
name: repo-code-style
description: Use when the user wants to run the repo-local formatter in ability_ability_runtime for C/C++ or GN files, including sorting C/C++ includes, enforcing the 120-column C/C++ rule, and sorting GN imports, include_dirs, sources, deps, external_deps, and similar GN string lists.
---

# Repo Code Style

Use this skill when working in the `ability_ability_runtime` repository and the user wants to run or check the repo-local formatter under `tools/code_style/`.

## What This Skill Uses

- `tools/code_style/format_cpp.sh`
- `tools/code_style/format_cpp.py`
- `tools/code_style/README.md`

## Workflow

1. Confirm the current workspace is the `ability_ability_runtime` repo root before running commands.
2. Use `bash -lc` to invoke the shell wrapper, because the formatter entry point is a Bash script.
3. If the user asked to preview changes, run with `--check --diff`.
4. If the user asked to apply formatting, run without `--check`.
5. If the formatter reports remaining C/C++ lines over 120 columns, surface that clearly instead of claiming the file is fully clean.

## Commands

Preview one file:

```bash
bash -lc "tools/code_style/format_cpp.sh --check --diff path/to/file.cpp"
```

Apply to one file:

```bash
bash -lc "tools/code_style/format_cpp.sh path/to/file.cpp"
```

Preview multiple files:

```bash
bash -lc "tools/code_style/format_cpp.sh --check --diff file1.cpp file2.h path/to/BUILD.gn"
```

Apply to multiple files:

```bash
bash -lc "tools/code_style/format_cpp.sh file1.cpp file2.h path/to/BUILD.gn"
```

Preview only changed tracked C/C++ and GN files:

```bash
bash -lc 'files=$(git diff --name-only -- "*.c" "*.cc" "*.cpp" "*.cxx" "*.h" "*.hh" "*.hpp" "*.hxx" "*.inc" "*.gn" "*.gni"); if [ -n "$files" ]; then tools/code_style/format_cpp.sh --check --diff $files; else echo "no matching changed files"; fi'
```

Apply only changed tracked C/C++ and GN files:

```bash
bash -lc 'files=$(git diff --name-only -- "*.c" "*.cc" "*.cpp" "*.cxx" "*.h" "*.hh" "*.hpp" "*.hxx" "*.inc" "*.gn" "*.gni"); if [ -n "$files" ]; then tools/code_style/format_cpp.sh $files; else echo "no matching changed files"; fi'
```

## Behavior To Expect

For C/C++ files, the formatter:
- groups the leading include block into self header, system headers, and project headers
- sorts includes alphabetically within each section
- tries to keep lines within 120 characters

For GN files, the formatter:
- sorts leading `import("...")` lines alphabetically
- sorts common string-list variables such as `include_dirs`, `sources`, `deps`, `external_deps`, `configs`, `public_deps`, and `public_configs`
- places local relative paths before non-local paths in path-like lists such as `include_dirs`, `public_include_dirs`, `inputs`, and `sources`

## Notes

- Prefer the shell wrapper over calling the Python file directly.
- Run from the repo root so relative paths resolve correctly.
- If the user asks why something did not move, inspect whether the GN block contains comments or non-string expressions; those blocks are intentionally skipped.
