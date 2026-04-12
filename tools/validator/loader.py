from __future__ import annotations

import json
from pathlib import Path

from model import ValidationResult


class DuplicateKeyDetector(dict):
    duplicates: list[str]

    def __init__(self, pairs):
        seen = set()
        duplicates = []
        for key, value in pairs:
            if key in seen:
                duplicates.append(key)
            seen.add(key)
            self[key] = value
        self.duplicates = duplicates


def load_json_file(path: Path) -> tuple[ValidationResult, dict | None]:
    result = ValidationResult(file=str(path))
    if not path.exists() or not path.is_file():
        result.add_issue("CFG001", "/", "file does not exist or is not readable")
        return result, None

    try:
        raw = path.read_bytes()
    except OSError as exc:
        result.add_issue("CFG001", "/", f"failed to read file: {exc}")
        return result, None

    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        result.add_issue("CFG005", "/", "file is not valid UTF-8")
        return result, None

    try:
        data = json.loads(text, object_pairs_hook=DuplicateKeyDetector)
    except json.JSONDecodeError as exc:
        result.add_issue("CFG002", "/", f"invalid JSON: {exc.msg} at line {exc.lineno}, column {exc.colno}")
        return result, None

    duplicate_keys = collect_duplicates(data)
    for key in duplicate_keys:
        result.add_issue("CFG006", "/", f"duplicate JSON key detected: {key}")

    if not isinstance(data, dict):
        result.add_issue("CFG003", "/", "root node must be a JSON object")
        return result, None

    if not data:
        result.add_issue("CFG004", "/", "root object must not be empty")
        return result, None

    return result, data


def collect_duplicates(node) -> list[str]:
    duplicates: list[str] = []
    if isinstance(node, DuplicateKeyDetector):
        duplicates.extend(node.duplicates)
        for value in node.values():
            duplicates.extend(collect_duplicates(value))
    elif isinstance(node, list):
        for item in node:
            duplicates.extend(collect_duplicates(item))
    return duplicates
