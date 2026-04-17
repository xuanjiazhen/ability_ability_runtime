from __future__ import annotations

import argparse
import sys
from pathlib import Path

from utils.loader import load_json_file
from utils.model import ValidationSummary
from utils.reporter import render_json, render_text
from utils.validators import validate_config


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Validate CLI tool config.json files")
    subparsers = parser.add_subparsers(dest="command")

    validate = subparsers.add_parser("validate", help="validate one or more config files")
    validate.add_argument("paths", nargs="+", help="config file path or directory")
    validate.add_argument("--json", action="store_true", dest="json_output", help="render results as JSON")
    validate.add_argument("--recursive", action="store_true", help="recursively discover config.json in directories")
    validate.add_argument("--fail-fast", action="store_true", help="stop after the first invalid file")
    return parser


def collect_targets(paths: list[str], recursive: bool) -> tuple[list[Path], int]:
    targets: list[Path] = []
    for raw in paths:
        path = Path(raw)
        if not path.exists():
            return [], 2
        if path.is_file():
            targets.append(path)
            continue
        if path.is_dir():
            if recursive:
                targets.extend(sorted(path.rglob("config.json")))
            else:
                candidate = path / "config.json"
                if candidate.exists():
                    targets.append(candidate)
            continue
        return [], 2
    if not targets:
        return [], 2
    return targets, 0


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if args.command != "validate":
        parser.print_help()
        return 2

    targets, status = collect_targets(args.paths, args.recursive)
    if status != 0:
        if not targets:
            print(f"错误: 在路径 {args.paths} 下未找到 config.json 文件", file=sys.stderr)
        return status

    results = []
    summary = ValidationSummary()
    for target in targets:
        result, data = load_json_file(target)
        if data is not None:
            validate_config(data, result)
        result.sort_issues()
        results.append(result)
        summary.add(result)
        if args.fail_fast and not result.valid:
            break

    output = render_json(results, summary) if args.json_output else render_text(results, summary)
    print(output)
    return 0 if summary.files_invalid == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
