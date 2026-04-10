from __future__ import annotations

import re
from typing import Any

from model import (
    SCHEMA_ALLOWED_KEYS,
    SUPPORTED_SCHEMA_TYPES,
    SUPPORTED_STRING_FORMATS,
    ValidationResult,
    is_integer_value,
    json_pointer,
)


def validate_schema_node(schema: Any, path: str, result: ValidationResult) -> None:
    if not isinstance(schema, dict):
        result.add_issue("SCH001", path, "schema node must be an object")
        return

    if "type" not in schema:
        result.add_issue("SCH002", path, "schema node must define type")
        return

    schema_type = schema["type"]
    if schema_type not in SUPPORTED_SCHEMA_TYPES:
        result.add_issue("SCH003", json_pointer(path, "type"), "unsupported schema type")
        return

    for key in schema.keys():
        if key not in SCHEMA_ALLOWED_KEYS[schema_type]:
            result.add_issue("SCH004", json_pointer(path, key), f"unsupported schema keyword for {schema_type}")

    if "description" in schema and not isinstance(schema["description"], str):
        result.add_issue("SCH005", json_pointer(path, "description"), "description must be a string")

    if "default" in schema and not value_matches_schema_type(schema["default"], schema_type):
        result.add_issue("SCH006", json_pointer(path, "default"), "default type does not match schema type")
    if "enum" in schema:
        validate_enum(schema["enum"], schema_type, json_pointer(path, "enum"), result)

    if schema_type == "string":
        validate_string_schema(schema, path, result)
    elif schema_type == "number":
        validate_number_schema(schema, path, result)
    elif schema_type == "integer":
        validate_integer_schema(schema, path, result)
    elif schema_type == "boolean":
        validate_boolean_schema(schema, path, result)
    elif schema_type == "array":
        validate_array_schema(schema, path, result)
    elif schema_type == "object":
        validate_object_schema(schema, path, result)


def validate_string_schema(schema: dict, path: str, result: ValidationResult) -> None:
    min_len = schema.get("minLength")
    max_len = schema.get("maxLength")
    if min_len is not None and not (is_integer_value(min_len) and min_len >= 0):
        result.add_issue("SCH101", json_pointer(path, "minLength"), "minLength must be an integer >= 0")
    if max_len is not None and not (is_integer_value(max_len) and max_len >= 0):
        result.add_issue("SCH102", json_pointer(path, "maxLength"), "maxLength must be an integer >= 0")
    if is_integer_value(min_len) and is_integer_value(max_len) and min_len > max_len:
        result.add_issue("SCH103", path, "minLength must not be greater than maxLength")
    if "pattern" in schema:
        pattern = schema["pattern"]
        if not isinstance(pattern, str):
            result.add_issue("SCH104", json_pointer(path, "pattern"), "pattern must be a string")
        else:
            try:
                re.compile(pattern)
            except re.error as exc:
                result.add_issue("SCH104", json_pointer(path, "pattern"), f"invalid regex pattern: {exc}")
    if "format" in schema and schema["format"] not in SUPPORTED_STRING_FORMATS:
        result.add_issue("SCH105", json_pointer(path, "format"), "unsupported string format")


def validate_number_schema(schema: dict, path: str, result: ValidationResult) -> None:
    validate_numeric_bounds(schema, path, result, "SCH201", "SCH202", "SCH203", "SCH204", allow_float=True)
    multiple = schema.get("multipleOf")
    if multiple is not None and not (isinstance(multiple, (int, float)) and not isinstance(multiple, bool) and multiple > 0):
        result.add_issue("SCH205", json_pointer(path, "multipleOf"), "multipleOf must be a positive number")


def validate_integer_schema(schema: dict, path: str, result: ValidationResult) -> None:
    for key in ("minimum", "maximum", "exclusiveMinimum", "exclusiveMaximum", "multipleOf"):
        if key in schema and not is_integer_value(schema[key]):
            result.add_issue("SCH301", json_pointer(path, key), f"{key} must be an integer")
    minimum = schema.get("minimum")
    maximum = schema.get("maximum")
    if is_integer_value(minimum) and is_integer_value(maximum) and minimum > maximum:
        result.add_issue("SCH303", path, "minimum must not be greater than maximum")
    multiple = schema.get("multipleOf")
    if multiple is not None and not (is_integer_value(multiple) and multiple > 0):
        result.add_issue("SCH302", json_pointer(path, "multipleOf"), "multipleOf must be an integer > 0")


def validate_boolean_schema(schema: dict, path: str, result: ValidationResult) -> None:
    invalid_keys = set(schema.keys()) - SCHEMA_ALLOWED_KEYS["boolean"]
    for key in invalid_keys:
        result.add_issue("SCH401", json_pointer(path, key), f"{key} is not allowed for boolean schema")
    if "default" in schema and not isinstance(schema["default"], bool):
        result.add_issue("SCH402", json_pointer(path, "default"), "default must be boolean")


def validate_array_schema(schema: dict, path: str, result: ValidationResult) -> None:
    if "items" not in schema:
        result.add_issue("SCH501", path, "array schema must define items")
    else:
        validate_schema_node(schema["items"], json_pointer(path, "items"), result)
    min_items = schema.get("minItems")
    max_items = schema.get("maxItems")
    if min_items is not None and not (is_integer_value(min_items) and min_items >= 0):
        result.add_issue("SCH503", json_pointer(path, "minItems"), "minItems must be an integer >= 0")
    if max_items is not None and not (is_integer_value(max_items) and max_items >= 0):
        result.add_issue("SCH503", json_pointer(path, "maxItems"), "maxItems must be an integer >= 0")
    if is_integer_value(min_items) and is_integer_value(max_items) and min_items > max_items:
        result.add_issue("SCH504", path, "minItems must not be greater than maxItems")
    if "uniqueItems" in schema and not isinstance(schema["uniqueItems"], bool):
        result.add_issue("SCH505", json_pointer(path, "uniqueItems"), "uniqueItems must be boolean")
    if "default" in schema and not isinstance(schema["default"], list):
        result.add_issue("SCH506", json_pointer(path, "default"), "default must be an array")


def validate_object_schema(schema: dict, path: str, result: ValidationResult) -> None:
    properties = schema.get("properties")
    if properties is not None and not isinstance(properties, dict):
        result.add_issue("SCH601", json_pointer(path, "properties"), "properties must be an object")
        properties = None
    if isinstance(properties, dict):
        for key, subschema in properties.items():
            subschema_path = json_pointer(json_pointer(path, "properties"), key)
            if not isinstance(subschema, dict):
                result.add_issue("SCH602", subschema_path, "property schema must be an object")
                continue
            validate_schema_node(subschema, subschema_path, result)

    required = schema.get("required")
    if required is not None:
        if not (isinstance(required, list) and all(isinstance(item, str) for item in required)):
            result.add_issue("SCH603", json_pointer(path, "required"), "required must be a string array")
        else:
            seen = set()
            for item in required:
                if item in seen:
                    result.add_issue("SCH605", json_pointer(path, "required"), "required entries must be unique")
                seen.add(item)
                if not isinstance(properties, dict) or item not in properties:
                    result.add_issue("SCH604", json_pointer(path, "required"), f"required field '{item}' is not defined")

    if "additionalProperties" in schema and not isinstance(schema["additionalProperties"], bool):
        result.add_issue(
            "SCH606", json_pointer(path, "additionalProperties"), "additionalProperties must be boolean"
        )

    min_props = schema.get("minProperties")
    max_props = schema.get("maxProperties")
    if min_props is not None and not (is_integer_value(min_props) and min_props >= 0):
        result.add_issue("SCH607", json_pointer(path, "minProperties"), "minProperties must be an integer >= 0")
    if max_props is not None and not (is_integer_value(max_props) and max_props >= 0):
        result.add_issue("SCH607", json_pointer(path, "maxProperties"), "maxProperties must be an integer >= 0")
    if is_integer_value(min_props) and is_integer_value(max_props) and min_props > max_props:
        result.add_issue("SCH608", path, "minProperties must not be greater than maxProperties")


def validate_numeric_bounds(
    schema: dict,
    path: str,
    result: ValidationResult,
    min_code: str,
    ex_code: str,
    range_code: str,
    ex_range_code: str,
    allow_float: bool,
) -> None:
    numeric_types = (int, float) if allow_float else (int,)
    for key in ("minimum", "maximum"):
        if key in schema and not (isinstance(schema[key], numeric_types) and not isinstance(schema[key], bool)):
            result.add_issue(min_code, json_pointer(path, key), f"{key} must be numeric")
    for key in ("exclusiveMinimum", "exclusiveMaximum"):
        if key in schema and not (isinstance(schema[key], numeric_types) and not isinstance(schema[key], bool)):
            result.add_issue(ex_code, json_pointer(path, key), f"{key} must be numeric")
    minimum = schema.get("minimum")
    maximum = schema.get("maximum")
    if _is_numeric(minimum) and _is_numeric(maximum) and minimum > maximum:
        result.add_issue(range_code, path, "minimum must not be greater than maximum")
    ex_min = schema.get("exclusiveMinimum")
    ex_max = schema.get("exclusiveMaximum")
    if _is_numeric(ex_min) and _is_numeric(ex_max) and ex_min >= ex_max:
        result.add_issue(ex_range_code, path, "exclusiveMinimum must be less than exclusiveMaximum")


def _is_numeric(value: Any) -> bool:
    return isinstance(value, (int, float)) and not isinstance(value, bool)


def value_matches_schema_type(value: Any, schema_type: str) -> bool:
    if schema_type == "string":
        return isinstance(value, str)
    if schema_type == "number":
        return _is_numeric(value)
    if schema_type == "integer":
        return is_integer_value(value)
    if schema_type == "boolean":
        return isinstance(value, bool)
    if schema_type == "array":
        return isinstance(value, list)
    if schema_type == "object":
        return isinstance(value, dict)
    return False


def validate_enum(enum_value: Any, schema_type: str, path: str, result: ValidationResult) -> None:
    if not isinstance(enum_value, list) or not enum_value:
        result.add_issue("SCH004", path, "enum must be a non-empty array")
        return
    for item in enum_value:
        if not value_matches_schema_type(item, schema_type):
            result.add_issue("SCH004", path, "enum entries must match schema type")


def resolve_schema_path(input_schema: dict, path_parts: list[str]) -> dict | None:
    current = input_schema
    for part in path_parts:
        if not isinstance(current, dict):
            return None
        if current.get("type") != "object":
            return None
        properties = current.get("properties")
        if not isinstance(properties, dict) or part not in properties:
            return None
        current = properties[part]
    return current if isinstance(current, dict) else None


def resolve_schema_path_detail(input_schema: dict, path_parts: list[str]) -> tuple[str, dict | None]:
    current = input_schema
    for index, part in enumerate(path_parts):
        if not isinstance(current, dict):
            return "invalid_node", None
        if current.get("type") != "object":
            return ("intermediate_not_object", None) if index < len(path_parts) else ("invalid_node", None)
        properties = current.get("properties")
        if not isinstance(properties, dict):
            return "invalid_node", None
        if part not in properties:
            return ("leaf_missing", None) if index == len(path_parts) - 1 else ("path_missing", None)
        current = properties[part]
    return ("ok", current) if isinstance(current, dict) else ("leaf_missing", None)
