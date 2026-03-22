#!/usr/bin/env python3
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import annotations

import argparse
import difflib
import pathlib
import re
import sys
from dataclasses import dataclass
from typing import Iterable


INCLUDE_RE = re.compile(r"^(?P<indent>\s*)#include\s*(?P<target><[^>]+>|\"[^\"]+\")\s*$")
COMMENT_LINE_RE = re.compile(r"^(?P<indent>\s*)// ?(?P<body>.*)$")
BLOCK_COMMENT_RE = re.compile(r"^(?P<indent>\s*)\*(?: ?(?P<body>.*))?$")
IMPORT_RE = re.compile(r'^(?P<indent>\s*)import\("(?P<path>[^\"]+)"\)\s*$')
GN_LIST_START_RE = re.compile(r'^(?P<indent>\s*)(?P<name>[A-Za-z0-9_]+)\s*(?P<op>\+?=)\s*\[\s*$')
GN_INLINE_LIST_RE = re.compile(r'^(?P<indent>\s*)(?P<name>[A-Za-z0-9_]+)\s*(?P<op>\+?=)\s*\[(?P<body>.*)\]\s*$')
GN_STRING_ITEM_RE = re.compile(r'^(?P<indent>\s*)"(?P<value>[^\"]+)"(?P<trailing>,?)\s*$')

SORTABLE_GN_LISTS = {
    "allowlist",
    "args",
    "cflags",
    "cflags_c",
    "cflags_cc",
    "configs",
    "defines",
    "deps",
    "external_deps",
    "frameworks",
    "include_dirs",
    "inputs",
    "libs",
    "ldflags",
    "output_extension",
    "public_configs",
    "public_deps",
    "public_external_deps",
    "public_include_dirs",
    "sources",
    "testonly_deps",
    "visibility",
}

CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inc"}
GN_SUFFIXES = {".gn", ".gni"}


@dataclass(frozen=True)
class LongLineViolation:
    path: pathlib.Path
    line_no: int
    length: int
    line: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Format C/C++ and GN files by regrouping #include lines, constraining line width, "
            "and alphabetizing common GN imports and list variables."
        )
    )
    parser.add_argument("files", nargs="+", help="Files to process.")
    parser.add_argument("--line-length", type=int, default=120, help="Maximum allowed line length for C/C++ files.")
    parser.add_argument("--check", action="store_true", help="Report changes instead of writing files.")
    parser.add_argument(
        "--diff",
        action="store_true",
        help="Show a unified diff in --check mode.",
    )
    parser.add_argument("--no-organize-includes", action="store_true", help="Skip C/C++ include regrouping.")
    parser.add_argument("--no-wrap-lines", action="store_true", help="Skip C/C++ line reflow.")
    parser.add_argument("--no-sort-gn", action="store_true", help="Skip GN import/list sorting.")
    return parser.parse_args()


def detect_newline(text: str) -> str:
    if "\r\n" in text:
        return "\r\n"
    return "\n"


def split_lines(text: str) -> list[str]:
    return text.splitlines()


def join_lines(lines: Iterable[str], newline: str, ends_with_newline: bool) -> str:
    text = newline.join(lines)
    if ends_with_newline:
        text += newline
    return text


def is_leading_banner_line(line: str, in_block_comment: bool) -> tuple[bool, bool]:
    stripped = line.strip()
    if in_block_comment:
        return True, not ("*/" in stripped)
    if not stripped:
        return True, False
    if stripped.startswith("/*"):
        return True, not ("*/" in stripped)
    if stripped.startswith("//"):
        return True, False
    if stripped == "#pragma once":
        return True, False
    return False, False


def classify_include(target: str, file_path: pathlib.Path) -> tuple[int, str]:
    stem = file_path.stem
    own_headers = {
        f'"{stem}.h"',
        f'"{stem}.hpp"',
        f'"{stem}.hh"',
        f'"{stem}.inc"',
    }
    if target in own_headers:
        return 0, target.lower()
    if target.startswith("<"):
        return 1, target.lower()
    return 2, target.lower()


def include_sort_key(target: str) -> str:
    return target[1:-1].lower()


def organize_includes(text: str, file_path: pathlib.Path) -> str:
    newline = detect_newline(text)
    lines = split_lines(text)
    if not lines:
        return text

    start = 0
    in_block_comment = False
    while start < len(lines):
        keep_scanning, in_block_comment = is_leading_banner_line(lines[start], in_block_comment)
        if not keep_scanning:
            break
        start += 1

    while start < len(lines) and not lines[start].strip():
        start += 1

    if start >= len(lines):
        return text

    if not INCLUDE_RE.match(lines[start]):
        return text

    end = start
    parsed: list[tuple[int, str, str]] = []
    while end < len(lines):
        line = lines[end]
        if not line.strip():
            end += 1
            continue
        match = INCLUDE_RE.match(line)
        if not match:
            break
        target = match.group("target")
        group_id, _ = classify_include(target, file_path)
        parsed.append((group_id, include_sort_key(target), target))
        end += 1

    if not parsed:
        return text

    parsed.sort(key=lambda item: (item[0], item[1]))

    grouped_lines: list[str] = []
    current_group = None
    for group_id, _, target in parsed:
        if current_group is not None and group_id != current_group:
            grouped_lines.append("")
        grouped_lines.append(f"#include {target}")
        current_group = group_id

    before = lines[:start]
    after = lines[end:]
    updated = before + grouped_lines
    if after and after[0].strip():
        updated.append("")
    updated.extend(after)
    return join_lines(updated, newline, text.endswith(("\n", "\r\n")))


def split_comment_from_code(line: str) -> int | None:
    in_single = False
    in_double = False
    escaped = False
    for index in range(len(line) - 1):
        ch = line[index]
        nxt = line[index + 1]
        if escaped:
            escaped = False
            continue
        if ch == "\\" and (in_single or in_double):
            escaped = True
            continue
        if ch == "'" and not in_double:
            in_single = not in_single
            continue
        if ch == '"' and not in_single:
            in_double = not in_double
            continue
        if not in_single and not in_double and ch == "/" and nxt == "/":
            return index
    return None


def wrap_comment_line(line: str, limit: int) -> list[str] | None:
    match = COMMENT_LINE_RE.match(line)
    if not match:
        return None
    indent = match.group("indent")
    words = match.group("body").split()
    prefix = f"{indent}// "
    if not words:
        return [f"{indent}//"]
    return wrap_words(words, prefix, prefix, limit)


def wrap_block_comment_line(line: str, limit: int) -> list[str] | None:
    stripped = line.strip()
    if stripped.startswith("/*") or stripped.endswith("*/"):
        return None
    match = BLOCK_COMMENT_RE.match(line)
    if not match:
        return None
    indent = match.group("indent")
    body = (match.group("body") or "").split()
    prefix = f"{indent} * "
    if not body:
        return [f"{indent} *"]
    return wrap_words(body, prefix, prefix, limit)


def wrap_words(words: list[str], first_prefix: str, continuation_prefix: str, limit: int) -> list[str]:
    lines: list[str] = []
    current = first_prefix
    for word in words:
        candidate = word if current.endswith(" ") else f" {word}"
        if len(current) + len(candidate) <= limit:
            current += candidate
            continue
        lines.append(current.rstrip())
        current = continuation_prefix + word
    lines.append(current.rstrip())
    return lines


def find_break_position(line: str, limit: int) -> int | None:
    in_single = False
    in_double = False
    escaped = False
    candidates: list[int] = []
    for index, ch in enumerate(line):
        if escaped:
            escaped = False
            continue
        if ch == "\\" and (in_single or in_double):
            escaped = True
            continue
        if ch == "'" and not in_double:
            in_single = not in_single
            continue
        if ch == '"' and not in_single:
            in_double = not in_double
            continue
        if in_single or in_double:
            continue
        if ch == ",":
            candidates.append(index + 1)
            continue
        two = line[index:index + 2]
        if two in {"&&", "||", "<<", ">>"}:
            candidates.append(index)
            continue
    candidates = [candidate for candidate in candidates if 0 < candidate <= limit]
    if not candidates:
        return None
    return max(candidates)


def wrap_code_line(line: str, limit: int) -> list[str] | None:
    if len(line) <= limit:
        return [line]
    if line.lstrip().startswith("#"):
        return None

    comment_index = split_comment_from_code(line)
    if comment_index is not None:
        code = line[:comment_index].rstrip()
        comment = line[comment_index:]
        if len(code) <= limit:
            wrapped_comment = wrap_comment_line(comment.strip(), limit - len(code) - 1)
            if wrapped_comment and wrapped_comment[0].startswith("//"):
                comment_lines = [f"{code} {wrapped_comment[0]}"]
                indent = " " * (len(code) + 1)
                for extra in wrapped_comment[1:]:
                    comment_lines.append(f"{indent}{extra}")
                if all(len(item) <= limit for item in comment_lines):
                    return comment_lines

    indent_len = len(line) - len(line.lstrip(" "))
    indent = line[:indent_len]
    continuation = indent + " " * 4
    result: list[str] = []
    remaining = line.rstrip()

    while len(remaining) > limit:
        cut = find_break_position(remaining, limit)
        if cut is None:
            return None
        left = remaining[:cut].rstrip()
        right = remaining[cut:].lstrip()
        if not left or not right:
            return None
        result.append(left)
        remaining = continuation + right

    result.append(remaining)
    if any(len(item) > limit for item in result):
        return None
    return result


def wrap_long_lines(
    text: str,
    file_path: pathlib.Path,
    limit: int,
) -> tuple[str, list[LongLineViolation]]:
    newline = detect_newline(text)
    lines = split_lines(text)
    updated: list[str] = []
    violations: list[LongLineViolation] = []

    for original_index, line in enumerate(lines, start=1):
        if len(line) <= limit:
            updated.append(line)
            continue

        wrapped = wrap_comment_line(line, limit)
        if wrapped is None:
            wrapped = wrap_block_comment_line(line, limit)
        if wrapped is None:
            wrapped = wrap_code_line(line, limit)

        if wrapped is None:
            violations.append(LongLineViolation(file_path, original_index, len(line), line))
            updated.append(line)
            continue

        updated.extend(wrapped)
        for new_line in wrapped:
            if len(new_line) > limit:
                violations.append(LongLineViolation(file_path, original_index, len(new_line), new_line))

    return join_lines(updated, newline, text.endswith(("\n", "\r\n"))), violations


def find_leading_banner_end(lines: list[str]) -> int:
    index = 0
    in_block_comment = False
    while index < len(lines):
        keep_scanning, in_block_comment = is_leading_banner_line(lines[index], in_block_comment)
        if not keep_scanning:
            break
        index += 1
    return index


def sort_gn_imports(lines: list[str]) -> list[str]:
    start = find_leading_banner_end(lines)
    while start < len(lines) and not lines[start].strip():
        start += 1

    end = start
    imports: list[tuple[str, str]] = []
    while end < len(lines):
        line = lines[end]
        if not line.strip():
            end += 1
            continue
        match = IMPORT_RE.match(line)
        if not match:
            break
        imports.append((match.group("path").lower(), line.strip()))
        end += 1

    if len(imports) < 2:
        return lines

    sorted_lines = [line for _, line in sorted(imports, key=lambda item: item[0])]
    updated = lines[:start] + sorted_lines
    if end < len(lines) and lines[end].strip():
        updated.append("")
    updated.extend(lines[end:])
    return updated


def parse_gn_inline_items(body: str) -> list[str] | None:
    stripped = body.strip()
    if not stripped:
        return []
    if "#" in stripped or "//" in stripped:
        return None
    parts = [part.strip() for part in stripped.split(",")]
    values: list[str] = []
    for part in parts:
        if not part:
            continue
        if len(part) < 2 or not part.startswith('"') or not part.endswith('"'):
            return None
        values.append(part[1:-1])
    return values


def sort_gn_inline_list(line: str) -> str | None:
    match = GN_INLINE_LIST_RE.match(line)
    if not match:
        return None
    name = match.group("name")
    if name not in SORTABLE_GN_LISTS:
        return None
    values = parse_gn_inline_items(match.group("body"))
    if values is None:
        return None
    sorted_values = sorted(values, key=lambda item: item.lower())
    if not sorted_values:
        return f'{match.group("indent")}{name} {match.group("op")} []'
    body = ", ".join(f'"{value}"' for value in sorted_values)
    return f'{match.group("indent")}{name} {match.group("op")} [ {body} ]'


def parse_gn_multiline_list(lines: list[str], start: int) -> tuple[int, list[str], bool] | None:
    values: list[str] = []
    end = start + 1
    trailing_comma = True
    while end < len(lines):
        stripped = lines[end].strip()
        if stripped == "]":
            return end, values, trailing_comma
        if not stripped:
            end += 1
            continue
        if stripped.startswith("#") or stripped.startswith("//"):
            return None
        match = GN_STRING_ITEM_RE.match(lines[end])
        if not match:
            return None
        values.append(match.group("value"))
        trailing_comma = bool(match.group("trailing"))
        end += 1
    return None


def rebuild_gn_multiline_list(header: str, values: list[str], trailing_comma: bool) -> list[str]:
    indent = header[: len(header) - len(header.lstrip(" "))]
    item_indent = indent + "  "
    result = [header]
    if not values:
        result.append(f"{indent}]")
        return result
    sorted_values = sorted(values, key=lambda item: item.lower())
    for index, value in enumerate(sorted_values):
        comma = ","
        if index == len(sorted_values) - 1 and not trailing_comma:
            comma = ""
        result.append(f'{item_indent}"{value}"{comma}')
    result.append(f"{indent}]")
    return result


def sort_gn_lists(lines: list[str]) -> list[str]:
    updated: list[str] = []
    index = 0
    while index < len(lines):
        inline = sort_gn_inline_list(lines[index])
        if inline is not None:
            updated.append(inline)
            index += 1
            continue

        start_match = GN_LIST_START_RE.match(lines[index])
        if start_match and start_match.group("name") in SORTABLE_GN_LISTS:
            parsed = parse_gn_multiline_list(lines, index)
            if parsed is not None:
                end, values, trailing_comma = parsed
                rebuilt = rebuild_gn_multiline_list(lines[index], values, trailing_comma)
                updated.extend(rebuilt)
                index = end + 1
                continue

        updated.append(lines[index])
        index += 1
    return updated


def sort_gn_file(text: str) -> str:
    newline = detect_newline(text)
    lines = split_lines(text)
    lines = sort_gn_imports(lines)
    lines = sort_gn_lists(lines)
    return join_lines(lines, newline, text.endswith(("\n", "\r\n")))


def format_cpp_file(path: pathlib.Path, text: str, args: argparse.Namespace) -> tuple[str, list[LongLineViolation]]:
    updated = text
    if not args.no_organize_includes:
        updated = organize_includes(updated, path)
    violations: list[LongLineViolation] = []
    if not args.no_wrap_lines:
        updated, violations = wrap_long_lines(updated, path, args.line_length)
    return updated, violations


def format_gn_file(path: pathlib.Path, text: str, args: argparse.Namespace) -> tuple[str, list[LongLineViolation]]:
    del path
    if args.no_sort_gn:
        return text, []
    return sort_gn_file(text), []


def format_file(path: pathlib.Path, args: argparse.Namespace) -> tuple[str, list[LongLineViolation]]:
    original = path.read_text(encoding="utf-8")
    suffix = path.suffix.lower()
    if suffix in GN_SUFFIXES:
        return format_gn_file(path, original, args)
    if suffix in CPP_SUFFIXES:
        return format_cpp_file(path, original, args)
    return original, []


def print_diff(original: str, updated: str, path: pathlib.Path) -> None:
    diff = difflib.unified_diff(
        original.splitlines(),
        updated.splitlines(),
        fromfile=f"{path}",
        tofile=f"{path}",
        lineterm="",
    )
    for line in diff:
        print(line)


def main() -> int:
    args = parse_args()
    had_changes = False
    had_violations = False

    for file_name in args.files:
        path = pathlib.Path(file_name)
        original = path.read_text(encoding="utf-8")
        updated, violations = format_file(path, args)
        if violations:
            had_violations = True
            for violation in violations:
                print(
                    f"{violation.path}:{violation.line_no}: line length {violation.length} exceeds "
                    f"{args.line_length} and could not be safely rewritten",
                    file=sys.stderr,
                )
        if original == updated:
            continue

        had_changes = True
        if args.check:
            print(f"would update {path}")
            if args.diff:
                print_diff(original, updated, path)
            continue

        path.write_text(updated, encoding="utf-8")
        print(f"updated {path}")

    if args.check and not had_changes:
        print("no changes needed")

    if had_violations:
        return 2
    if args.check and had_changes:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

