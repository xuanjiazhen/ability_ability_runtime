from __future__ import annotations

import json

from .model import ValidationResult, ValidationSummary


def render_text(results: list[ValidationResult], summary: ValidationSummary) -> str:
    lines: list[str] = []
    for result in results:
        status = "PASS" if result.valid else "FAIL"
        lines.append(f"[{status}] {result.file}")
        for issue in result.issues:
            lines.append(f"  - {issue.code} {issue.path}: {issue.message}")
    lines.append(
        f"Summary: checked={summary.files_checked} valid={summary.files_valid} invalid={summary.files_invalid}"
    )
    return "\n".join(lines)


def render_json(results: list[ValidationResult], summary: ValidationSummary) -> str:
    payload = {
        "summary": {
            "files_checked": summary.files_checked,
            "files_valid": summary.files_valid,
            "files_invalid": summary.files_invalid,
        },
        "results": [
            {
                "file": result.file,
                "valid": result.valid,
                "issues": [
                    {"code": issue.code, "path": issue.path, "message": issue.message} for issue in result.issues
                ],
            }
            for result in results
        ],
    }
    return json.dumps(payload, ensure_ascii=False, indent=2)
