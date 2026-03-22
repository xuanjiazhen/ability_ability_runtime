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

# C++ and GN Formatting Helper

This directory provides a small repo-local formatter for C/C++ sources and GN build files.

## What It Does

For C/C++ files:
- Reorders the leading `#include` block into 3 sections:
  - the file's own header
  - system headers like `<vector>`
  - project headers like `"ability_util.h"`
- Sorts includes alphabetically within each section
- Tries to keep lines within 120 characters
- Reports long lines it cannot safely rewrite

For GN files:
- Sorts leading `import("...")` statements alphabetically
- Sorts common string-list variables alphabetically, including:
  - `configs`
  - `deps`
  - `external_deps`
  - `include_dirs`
  - `public_configs`
  - `public_deps`
  - `sources`
  - and similar list variables handled by the script
- For path-like GN lists such as `include_dirs`, `public_include_dirs`, `inputs`, and `sources`, local relative paths are placed before non-local paths
- Skips GN blocks with comments or complex expressions instead of rewriting them unsafely

## Files

- `format_cpp.py`: Python formatter implementation for C/C++ and GN files
- `format_cpp.sh`: Git Bash wrapper that invokes the Python formatter

## Usage

Run on one C++ file:

```bash
tools/code_style/format_cpp.sh path/to/file.cpp
```

Run on one GN file:

```bash
tools/code_style/format_cpp.sh path/to/BUILD.gn
```

Run on multiple files:

```bash
tools/code_style/format_cpp.sh file1.cpp file2.h path/to/BUILD.gn
```

Preview changes without writing:

```bash
tools/code_style/format_cpp.sh --check path/to/file.cpp path/to/BUILD.gn
```

Preview changes with a diff:

```bash
tools/code_style/format_cpp.sh --check --diff path/to/file.cpp path/to/BUILD.gn
```

Skip C/C++ include regrouping:

```bash
tools/code_style/format_cpp.sh --no-organize-includes path/to/file.cpp
```

Skip C/C++ line wrapping:

```bash
tools/code_style/format_cpp.sh --no-wrap-lines path/to/file.cpp
```

Skip GN sorting:

```bash
tools/code_style/format_cpp.sh --no-sort-gn path/to/BUILD.gn
```

Call the Python entry point directly:

```bash
python3 tools/code_style/format_cpp.py --check --diff path/to/file.cpp path/to/BUILD.gn
```

## Exit Codes

- `0`: no error
- `1`: `--check` found files that would be changed
- `2`: some C/C++ lines still exceed the limit and were left unchanged

## Notes

- The script only reorganizes the leading include block near the top of a C/C++ file.
- It preserves the 3-section include style used by `agent_manager_service.cpp`.
- GN sorting is limited to recognized `import(...)` statements and simple string lists.
- Long C/C++ lines are rewritten conservatively. If the script cannot split a line safely, it reports the line instead of guessing.
