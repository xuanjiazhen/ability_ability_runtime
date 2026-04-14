from __future__ import annotations

from dataclasses import dataclass
from _build_validation import ensure_build_validation_import_path


ensure_build_validation_import_path()

from validation_model import *  # noqa: F401,F403


@dataclass
class ValidationSummary:
    files_checked: int = 0
    files_valid: int = 0
    files_invalid: int = 0

    def add(self, result: ValidationResult) -> None:
        self.files_checked += 1
        if result.valid:
            self.files_valid += 1
        else:
            self.files_invalid += 1
