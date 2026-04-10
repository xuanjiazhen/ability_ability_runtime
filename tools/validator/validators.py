from __future__ import annotations

from typing import Any

from model import (
    ALLOWED_MAPPING_KEYS,
    SUPPORTED_EVENT_TYPES,
    SUPPORTED_MAPPING_TYPES,
    SUPPORTED_SUBCOMMAND_FIELDS,
    SUPPORTED_TOP_LEVEL_FIELDS,
    ValidationResult,
    is_integer_value,
    json_pointer,
)
from schema_utils import resolve_schema_path_detail, validate_schema_node


def validate_config(data: dict[str, Any], result: ValidationResult) -> None:
    validate_top_level(data, result)
    if not result.valid and not isinstance(data, dict):
        return

    has_subcommands = data.get("hasSubcommands", False)
    if has_subcommands is True:
        validate_subcommand_tool(data, result)
    elif has_subcommands is False or "hasSubcommands" not in data:
        validate_simple_tool(data, result)
    else:
        result.add_issue("FMT005", "/hasSubcommands", "hasSubcommands must be boolean")


def validate_top_level(data: dict[str, Any], result: ValidationResult) -> None:
    for key in data.keys():
        if key not in SUPPORTED_TOP_LEVEL_FIELDS:
            result.add_issue("TOP013", json_pointer("", key), "unknown top-level field")

    _require_non_empty_string(data, "name", "TOP001", result)
    _require_non_empty_string(data, "version", "TOP002", result)
    _require_non_empty_string(data, "description", "TOP003", result)
    _require_non_empty_string(data, "executablePath", "TOP004", result)

    executable_path = data.get("executablePath")
    if isinstance(executable_path, str) and not executable_path.startswith("/"):
        result.add_issue("TOP005", "/executablePath", "executablePath must be an absolute path")

    permissions = data.get("permissions")
    if permissions is not None:
        if not _is_string_list(permissions):
            result.add_issue("TOP006", "/permissions", "permissions must be a string array")
        elif len(set(permissions)) != len(permissions):
            result.add_issue("TOP007", "/permissions", "permissions must not contain duplicates")

    timeout = data.get("timeout")
    if timeout is not None:
        if not is_integer_value(timeout):
            result.add_issue("TOP008", "/timeout", "timeout must be an integer")
        elif timeout < 0:
            result.add_issue("TOP009", "/timeout", "timeout must be >= 0")

    event_types = data.get("eventTypes")
    if event_types is not None:
        if not _is_string_list(event_types):
            result.add_issue("TOP010", "/eventTypes", "eventTypes must be a string array")
        elif len(set(event_types)) != len(event_types):
            result.add_issue("TOP011", "/eventTypes", "eventTypes must not contain duplicates")

    if "eventSchemas" in data and not isinstance(data["eventSchemas"], dict):
        result.add_issue("TOP012", "/eventSchemas", "eventSchemas must be an object")


def validate_simple_tool(data: dict[str, Any], result: ValidationResult) -> None:
    if "inputSchema" not in data:
        result.add_issue("FMT001", "/inputSchema", "simple tool must define inputSchema")
        return
    if "argumentMapping" not in data:
        result.add_issue("FMT002", "/argumentMapping", "simple tool must define argumentMapping")
        return
    if "subcommands" in data:
        result.add_issue("FMT003", "/subcommands", "simple tool must not define subcommands")
    if "hasSubcommands" in data and not isinstance(data["hasSubcommands"], bool):
        result.add_issue("FMT005", "/hasSubcommands", "hasSubcommands must be boolean")

    validate_input_schema(data["inputSchema"], "/inputSchema", "SCX001", "SCX002", result)
    if "outputSchema" in data:
        validate_output_schema(data["outputSchema"], "/outputSchema", result)
    validate_argument_mapping(data.get("argumentMapping"), data["inputSchema"], "/argumentMapping", result)
    validate_mapping_relationships(data["inputSchema"], data.get("argumentMapping"), "/argumentMapping", result)
    validate_events(data.get("eventTypes"), data.get("eventSchemas"), "outputSchema" in data, "/", result)


def validate_subcommand_tool(data: dict[str, Any], result: ValidationResult) -> None:
    if "subcommands" not in data:
        result.add_issue("FMT101", "/subcommands", "subcommand tool must define subcommands")
        return
    subcommands = data["subcommands"]
    if not isinstance(subcommands, dict):
        result.add_issue("FMT102", "/subcommands", "subcommands must be an object")
        return
    if not subcommands:
        result.add_issue("FMT103", "/subcommands", "subcommands must not be empty")
    if "inputSchema" in data:
        result.add_issue("FMT104", "/inputSchema", "subcommand tool must not define top-level inputSchema")
    if "argumentMapping" in data:
        result.add_issue("FMT105", "/argumentMapping", "subcommand tool must not define top-level argumentMapping")
    if "outputSchema" in data:
        result.add_issue("FMT106", "/outputSchema", "subcommand tool must not define top-level outputSchema")

    seen_names: set[str] = set()
    for name, subcommand in subcommands.items():
        path = json_pointer("/subcommands", name)
        if not isinstance(name, str) or not name:
            result.add_issue("FMT107", path, "subcommand name must be a non-empty string")
            continue
        if name in seen_names:
            result.add_issue("FMT108", path, "duplicate subcommand name")
        seen_names.add(name)
        validate_subcommand(name, subcommand, path, result)

    validate_events(data.get("eventTypes"), data.get("eventSchemas"), False, "/", result)


def validate_subcommand(name: str, subcommand: Any, path: str, result: ValidationResult) -> None:
    if not isinstance(subcommand, dict):
        result.add_issue("SUB001", path, "subcommand definition must be an object")
        return

    for key in subcommand.keys():
        if key not in SUPPORTED_SUBCOMMAND_FIELDS:
            result.add_issue("SUB007", json_pointer(path, key), "unknown subcommand field")

    _require_non_empty_string(subcommand, "description", "SUB003", result, parent=path)
    if "description" not in subcommand:
        result.add_issue("SUB002", json_pointer(path, "description"), "subcommand must define description")
    if "inputSchema" not in subcommand:
        result.add_issue("SUB004", json_pointer(path, "inputSchema"), "subcommand must define inputSchema")
        return
    if "argumentMapping" not in subcommand:
        result.add_issue("SUB005", json_pointer(path, "argumentMapping"), "subcommand must define argumentMapping")
        return

    validate_input_schema(
        subcommand["inputSchema"],
        json_pointer(path, "inputSchema"),
        "SCX004",
        "SCX005",
        result,
    )
    if "outputSchema" in subcommand:
        validate_output_schema(subcommand["outputSchema"], json_pointer(path, "outputSchema"), result)
    validate_argument_mapping(
        subcommand["argumentMapping"], subcommand["inputSchema"], json_pointer(path, "argumentMapping"), result
    )
    validate_mapping_relationships(
        subcommand["inputSchema"], subcommand["argumentMapping"], json_pointer(path, "argumentMapping"), result
    )
    validate_events(
        subcommand.get("eventTypes"),
        subcommand.get("eventSchemas"),
        "outputSchema" in subcommand,
        path,
        result,
    )


def validate_input_schema(
    input_schema: Any, path: str, type_code: str, props_code: str, result: ValidationResult
) -> None:
    validate_schema_node(input_schema, path, result)
    if not isinstance(input_schema, dict):
        return
    if input_schema.get("type") != "object":
        result.add_issue(type_code, json_pointer(path, "type"), "inputSchema root type must be object")
    if not isinstance(input_schema.get("properties"), dict):
        result.add_issue(props_code, json_pointer(path, "properties"), "inputSchema must define properties object")


def validate_output_schema(output_schema: Any, path: str, result: ValidationResult) -> None:
    validate_schema_node(output_schema, path, result)
    if isinstance(output_schema, dict) and output_schema.get("type") != "object":
        result.add_issue("SCX003", json_pointer(path, "type"), "outputSchema root type must be object")


def validate_argument_mapping(mapping: Any, input_schema: Any, path: str, result: ValidationResult) -> None:
    if not isinstance(mapping, dict):
        result.add_issue("MAP001", path, "argumentMapping must be an object")
        return
    if not isinstance(input_schema, dict):
        result.add_issue("MAP005", path, "argumentMapping must be validated with a matching inputSchema")
        return
    mapping_type = mapping.get("type")
    if mapping_type is None:
        result.add_issue("MAP002", json_pointer(path, "type"), "argumentMapping.type is required")
        return
    if mapping_type not in SUPPORTED_MAPPING_TYPES:
        result.add_issue(
            "MAP003",
            json_pointer(path, "type"),
            "argumentMapping.type must be one of flag, positional, flattened, json-string",
        )
        return
    for key in mapping.keys():
        if key not in ALLOWED_MAPPING_KEYS[mapping_type]:
            result.add_issue("MAP004", json_pointer(path, key), f"unknown field for {mapping_type} mapping")

    if mapping_type == "flag":
        _validate_flag_mapping(mapping, input_schema, path, result)
    elif mapping_type == "positional":
        _validate_positional_mapping(mapping, input_schema, path, result)
    elif mapping_type == "flattened":
        _validate_flattened_mapping(mapping, input_schema, path, result)
    elif mapping_type == "json-string":
        _validate_json_string_mapping(mapping, input_schema, path, result)


def validate_mapping_relationships(input_schema: Any, mapping: Any, path: str, result: ValidationResult) -> None:
    if not isinstance(input_schema, dict) or not isinstance(mapping, dict):
        return
    properties = input_schema.get("properties")
    if not isinstance(properties, dict):
        return

    referenced = collect_mapping_references(mapping)
    if not referenced:
        result.add_issue("REL006", path, "argumentMapping does not reference any inputSchema field")
        return

    required = input_schema.get("required", [])
    if isinstance(required, list):
        for key in required:
            if isinstance(key, str) and key not in referenced and key not in {ref.split(".")[0] for ref in referenced}:
                result.add_issue("REL002", path, f"required field '{key}' is not mapped")


def validate_events(
    event_types: Any, event_schemas: Any, has_output_schema: bool, base_path: str, result: ValidationResult
) -> None:
    event_types_path = json_pointer(base_path, "eventTypes") if base_path != "/" else "/eventTypes"
    event_schemas_path = json_pointer(base_path, "eventSchemas") if base_path != "/" else "/eventSchemas"

    if event_types is not None:
        if not _is_string_list(event_types):
            result.add_issue("EVT001", event_types_path, "eventTypes must be a string array")
            return
        invalid_types = [value for value in event_types if value not in SUPPORTED_EVENT_TYPES]
        for invalid in invalid_types:
            result.add_issue("EVT001", event_types_path, f"unsupported event type '{invalid}'")

    if event_schemas is not None and not isinstance(event_schemas, dict):
        result.add_issue("EVT002", event_schemas_path, "eventSchemas must be an object")
        return

    if isinstance(event_schemas, dict):
        for key, schema in event_schemas.items():
            if not isinstance(event_types, list) or key not in event_types:
                result.add_issue("EVT003", json_pointer(event_schemas_path, key), "event schema key must exist in eventTypes")
            if key in {"stdout", "stderr", "exit", "error"}:
                result.add_issue("EVT006", json_pointer(event_schemas_path, key), f"{key} must not be declared in eventSchemas")
            validate_schema_node(schema, json_pointer(event_schemas_path, key), result)

    if isinstance(event_types, list) and "result" in event_types and not has_output_schema:
        result.add_issue("EVT004", event_types_path, "result event requires outputSchema")
    if isinstance(event_types, list) and "progress" in event_types:
        if not isinstance(event_schemas, dict) or "progress" not in event_schemas:
            result.add_issue("EVT007", event_schemas_path, "progress event requires eventSchemas.progress")


def _validate_flag_mapping(mapping: dict, input_schema: Any, path: str, result: ValidationResult) -> None:
    templates = mapping.get("templates")
    if not isinstance(templates, dict):
        result.add_issue("MAP101", json_pointer(path, "templates"), "flag mapping requires templates object")
        return
    if not templates:
        result.add_issue("MAP103", json_pointer(path, "templates"), "templates must not be empty")
    properties = _schema_properties(input_schema)
    for key, value in templates.items():
        key_path = json_pointer(json_pointer(path, "templates"), key)
        if not key:
            result.add_issue("MAP111", key_path, "template key must not be empty")
            continue
        if key not in properties:
            result.add_issue("MAP104", key_path, f"template key '{key}' is not defined in inputSchema.properties")
            continue
        schema = properties[key]
        if not isinstance(schema, dict):
            continue
        schema_type = schema.get("type")
        if isinstance(value, dict):
            if set(value.keys()) - {"if_true", "if_false"}:
                result.add_issue("MAP106", key_path, "flag template object only allows if_true and if_false")
            if "if_true" not in value and "if_false" not in value:
                result.add_issue("MAP107", key_path, "flag template object must define if_true or if_false")
            if schema_type != "boolean":
                result.add_issue("MAP108", key_path, "if_true/if_false can only be used with boolean fields")
        elif isinstance(value, str):
            if schema_type == "object":
                result.add_issue("MAP109", key_path, "object field must not use flag mapping")
            if schema_type == "array" and "{value}" not in value:
                result.add_issue("MAP110", key_path, "array flag template must contain {value}")
            if schema_type != "boolean" and "{value}" not in value:
                result.add_issue("MAP112", key_path, "non-boolean flag template must contain {value}")
        else:
            result.add_issue("MAP105", key_path, "template value must be string or object")


def _validate_positional_mapping(mapping: dict, input_schema: Any, path: str, result: ValidationResult) -> None:
    order = mapping.get("order")
    order_path = json_pointer(path, "order")
    if not (isinstance(order, list) and order and all(isinstance(item, str) and item for item in order)):
        result.add_issue("MAP202", order_path, "order must be a non-empty string array")
        return
    seen = set()
    properties = _schema_properties(input_schema)
    for item in order:
        item_path = json_pointer(order_path, item)
        if item in seen:
            result.add_issue("MAP203", order_path, f"duplicate positional field '{item}'")
            continue
        seen.add(item)
        if item not in properties:
            result.add_issue("MAP204", item_path, f"positional field '{item}' is not defined")
            continue
        schema = properties[item]
        if not isinstance(schema, dict):
            continue
        schema_type = schema.get("type")
        if schema_type == "object":
            result.add_issue("MAP205", item_path, "object field cannot be used in positional mapping")
        if schema_type == "array":
            result.add_issue("MAP206", item_path, "array field cannot be used in positional mapping")


def _validate_flattened_mapping(mapping: dict, input_schema: Any, path: str, result: ValidationResult) -> None:
    templates = mapping.get("templates")
    if not isinstance(templates, dict):
        result.add_issue("MAP301", json_pointer(path, "templates"), "flattened mapping requires templates object")
        return
    if not templates:
        result.add_issue("MAP303", json_pointer(path, "templates"), "templates must not be empty")
    separator = mapping.get("separator", ".")
    if not isinstance(separator, str) or not separator:
        result.add_issue("MAP304", json_pointer(path, "separator"), "separator must be a non-empty string")
        separator = "."

    for key, value in templates.items():
        key_path = json_pointer(json_pointer(path, "templates"), key)
        if not isinstance(value, (str, dict)):
            result.add_issue("MAP105", key_path, "template value must be string or object")
        if separator not in key:
            result.add_issue("MAP305", key_path, "flattened template key must be a nested path")
            continue
        parts = key.split(separator)
        status, schema = resolve_schema_path_detail(input_schema, parts)
        if status == "path_missing":
            result.add_issue("MAP306", key_path, f"flattened path '{key}' is not defined")
            continue
        if status == "intermediate_not_object":
            result.add_issue("MAP307", key_path, "flattened path intermediate nodes must be object")
            continue
        if status == "leaf_missing":
            result.add_issue("MAP308", key_path, "flattened path leaf field must be declared")
            continue
        if status != "ok" or schema is None:
            result.add_issue("MAP306", key_path, f"flattened path '{key}' is not defined")
            continue
        schema_type = schema.get("type")
        if schema_type == "object":
            result.add_issue("MAP309", key_path, "flattened leaf field must not be object")
        elif schema_type == "array":
            result.add_issue("MAP310", key_path, "flattened leaf field must not be array")
        elif isinstance(value, dict) and schema_type != "boolean":
            result.add_issue("MAP311", key_path, "boolean template object can only be used on boolean leaf fields")


def _validate_json_string_mapping(mapping: dict, input_schema: Any, path: str, result: ValidationResult) -> None:
    templates = mapping.get("templates")
    if not isinstance(templates, dict):
        result.add_issue("MAP401", json_pointer(path, "templates"), "json-string mapping requires templates object")
        return
    if not templates:
        result.add_issue("MAP403", json_pointer(path, "templates"), "templates must not be empty")

    properties = _schema_properties(input_schema)
    for key, value in templates.items():
        key_path = json_pointer(json_pointer(path, "templates"), key)
        if key not in properties:
            result.add_issue("MAP404", key_path, f"template key '{key}' is not defined in inputSchema.properties")
            continue
        schema = properties[key]
        if not isinstance(schema, dict):
            continue
        schema_type = schema.get("type")
        if isinstance(value, dict):
            if set(value.keys()) - {"if_true", "if_false"}:
                result.add_issue("MAP405", key_path, "json-string template object only allows if_true and if_false")
            if schema_type not in {"boolean"}:
                result.add_issue("MAP410", key_path, "object/array fields must not use if_true/if_false")
        elif isinstance(value, str):
            if schema_type == "boolean" and "{value}" not in value:
                result.add_issue(
                    "MAP409",
                    key_path,
                    "boolean json-string template must contain {value} when using string form",
                )
            elif "{value}" not in value:
                result.add_issue("MAP408", key_path, "template must use the supported {value} placeholder")
        else:
            result.add_issue("MAP405", key_path, "template value must be string or object")


def _schema_properties(input_schema: Any) -> dict[str, dict]:
    if isinstance(input_schema, dict) and isinstance(input_schema.get("properties"), dict):
        return input_schema["properties"]
    return {}


def collect_mapping_references(mapping: dict) -> set[str]:
    mapping_type = mapping.get("type")
    if mapping_type == "positional":
        order = mapping.get("order", [])
        return {item for item in order if isinstance(item, str)}
    templates = mapping.get("templates", {})
    if isinstance(templates, dict):
        return {key for key in templates.keys() if isinstance(key, str)}
    return set()


def _require_non_empty_string(
    data: dict[str, Any], field: str, code: str, result: ValidationResult, parent: str = ""
) -> None:
    path = json_pointer(parent, field) if parent else json_pointer("", field)
    value = data.get(field)
    if not isinstance(value, str) or not value:
        result.add_issue(code, path, f"{field} must be a non-empty string")


def _is_string_list(value: Any) -> bool:
    return isinstance(value, list) and all(isinstance(item, str) for item in value)
