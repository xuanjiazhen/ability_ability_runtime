from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass(frozen=True)
class ValidationIssue:
    code: str
    path: str
    message: str


@dataclass
class ValidationResult:
    file: str
    valid: bool = True
    issues: list[ValidationIssue] = field(default_factory=list)

    def add_issue(self, code: str, path: str, message: str) -> None:
        self.valid = False
        self.issues.append(ValidationIssue(code=code, path=path, message=message))

    def sort_issues(self) -> None:
        self.issues.sort(key=lambda issue: (issue.path, issue.code, issue.message))


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


SUPPORTED_TOP_LEVEL_FIELDS = {
    "name",
    "version",
    "description",
    "executablePath",
    "permissions",
    "timeout",
    "eventTypes",
    "eventSchemas",
    "hasSubcommands",
    "inputSchema",
    "outputSchema",
    "argumentMapping",
    "subcommands",
}

SUPPORTED_SUBCOMMAND_FIELDS = {
    "description",
    "permissions",
    "timeout",
    "inputSchema",
    "outputSchema",
    "eventTypes",
    "eventSchemas",
    "argumentMapping",
}

SUPPORTED_SCHEMA_TYPES = {"string", "number", "integer", "boolean", "array", "object"}

SUPPORTED_STRING_FORMATS = {"ipv4", "ipv6", "hostname", "email", "uri", "date", "date-time"}

SCHEMA_ALLOWED_KEYS = {
    "string": {"type", "description", "default", "minLength", "maxLength", "pattern", "format", "enum"},
    "number": {
        "type",
        "description",
        "default",
        "minimum",
        "maximum",
        "exclusiveMinimum",
        "exclusiveMaximum",
        "multipleOf",
        "enum",
    },
    "integer": {
        "type",
        "description",
        "default",
        "minimum",
        "maximum",
        "exclusiveMinimum",
        "exclusiveMaximum",
        "multipleOf",
        "enum",
    },
    "boolean": {"type", "description", "default", "enum"},
    "array": {"type", "description", "default", "items", "minItems", "maxItems", "uniqueItems", "enum"},
    "object": {
        "type",
        "description",
        "properties",
        "required",
        "minProperties",
        "maxProperties",
        "enum",
    },
}

BUILTIN_EVENT_TYPES = {"stdout", "stderr", "exit", "error"}
RESERVED_EVENT_TYPES = BUILTIN_EVENT_TYPES | {"result"}

SUPPORTED_MAPPING_TYPES = {"flag", "positional", "flattened", "json-string"}

ALLOWED_MAPPING_KEYS = {
    "flag": {"type", "templates"},
    "positional": {"type", "order"},
    "flattened": {"type", "separator", "templates"},
    "json-string": {"type", "templates"},
}


def is_integer_value(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


def json_pointer(parent: str, child: str) -> str:
    base = parent.rstrip("/")
    escaped = child.replace("~", "~0").replace("/", "~1")
    return f"{base}/{escaped}" if base else f"/{escaped}"
