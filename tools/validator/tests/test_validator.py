from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path


TOOL_ROOT = Path(__file__).resolve().parents[1]
MAIN = TOOL_ROOT / "main.py"
TEST_DIR = Path(__file__).resolve().parent
CONFIG_DIR = TEST_DIR / "configs"
REPO_ROOT = TOOL_ROOT.parents[1]


def run_validator(*args: str) -> subprocess.CompletedProcess[str]:
    cmd = [sys.executable, str(MAIN), "validate", *args]
    return subprocess.run(cmd, text=True, capture_output=True, cwd=REPO_ROOT)


def assert_equal(actual, expected, message: str) -> None:
    if actual != expected:
        raise AssertionError(f"{message}: expected={expected!r} actual={actual!r}")


def assert_in(needle: str, haystack: str, message: str) -> None:
    if needle not in haystack:
        raise AssertionError(f"{message}: missing {needle!r}\nOutput:\n{haystack}")


def test_valid_simple() -> None:
    result = run_validator(str(CONFIG_DIR / "valid_simple_config.json"))
    assert_equal(result.returncode, 0, "valid simple config should pass")
    assert_in("[PASS]", result.stdout, "valid simple config should print PASS")


def test_valid_subcommand_json_output() -> None:
    result = run_validator(str(CONFIG_DIR / "valid_subcommand_config.json"), "--json")
    assert_equal(result.returncode, 0, "valid subcommand config should pass")
    payload = json.loads(result.stdout)
    assert_equal(payload["results"][0]["valid"], True, "json output should mark config as valid")
    assert_equal(payload["results"][0]["issues"], [], "valid subcommand config should not have issues")


def test_collect_all_json_in_directory() -> None:
    simple_payload = {
        "name": "ohos-dirscan",
        "version": "1.0.0",
        "description": "directory scan",
        "executablePath": "/usr/bin/dirscan-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with tempfile.TemporaryDirectory() as tmp:
        tmp_dir = Path(tmp)
        with open(tmp_dir / "first.json", "w", encoding="utf-8") as f:
            json.dump(simple_payload, f, ensure_ascii=False, indent=2)
        with open(tmp_dir / "second.json", "w", encoding="utf-8") as f:
            json.dump({**simple_payload, "name": "ohos-dirscan2"}, f, ensure_ascii=False, indent=2)

        result = run_validator(str(tmp_dir), "--all-json")
        assert_equal(result.returncode, 0, "directory all-json scan should pass")
        assert_in("[PASS]", result.stdout, "directory all-json scan should print PASS entries")
        assert_in("checked=2", result.stdout, "directory all-json scan should validate all json files")


def test_timeout_integer_required() -> None:
    simple_payload = {
        "name": "ohos-timeout",
        "version": "1.0.0",
        "description": "timeout integer",
        "executablePath": "/usr/bin/timeout-tool",
        "timeout": 0.5,
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_timeout_integer.json", "w", encoding="utf-8") as f:
        json.dump(simple_payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_timeout_integer.json"))
        assert_equal(result.returncode, 1, "float timeout should fail")
        assert_in("TOP008", result.stdout, "float timeout should report TOP008")
    finally:
        (CONFIG_DIR / "tmp_timeout_integer.json").unlink(missing_ok=True)


def test_empty_input_and_mapping_allowed() -> None:
    simple_payload = {
        "name": "ohos-empty",
        "version": "1.0.0",
        "description": "empty input and mapping",
        "executablePath": "/usr/bin/empty-tool",
        "inputSchema": {
            "type": "object",
            "properties": {},
        },
        "argMapping": {},
    }
    with open(CONFIG_DIR / "tmp_empty_input_mapping.json", "w", encoding="utf-8") as f:
        json.dump(simple_payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_empty_input_mapping.json"))
        assert_equal(result.returncode, 0, "empty input and mapping should pass")
        assert_in("[PASS]", result.stdout, "empty input and mapping should print PASS")
    finally:
        (CONFIG_DIR / "tmp_empty_input_mapping.json").unlink(missing_ok=True)


def test_extension_fields_rejected() -> None:
    payload = {
        "name": "ohos-private",
        "version": "1.0.0",
        "description": "extension fields",
        "executablePath": "/usr/bin/private-tool",
        "private": True,
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_extension_fields_allowed.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_extension_fields_allowed.json"))
        assert_equal(result.returncode, 1, "top-level extension field should fail")
        assert_in("TOP013", result.stdout, "top-level extension field should report TOP013")

        payload = {
            "name": "ohos-subprivate",
            "version": "1.0.0",
            "description": "subcommand extension fields",
            "executablePath": "/usr/bin/subprivate-tool",
            "hasSubcommands": True,
            "subcommands": {
                "run": {
                    "description": "run",
                    "private": True,
                    "inputSchema": {
                        "type": "object",
                        "properties": {},
                    },
                    "argMapping": {},
                },
            },
        }
        with open(CONFIG_DIR / "tmp_extension_fields_allowed.json", "w", encoding="utf-8") as f:
            json.dump(payload, f, ensure_ascii=False, indent=2)
        result = run_validator(str(CONFIG_DIR / "tmp_extension_fields_allowed.json"))
        assert_equal(result.returncode, 1, "subcommand extension field should fail")
        assert_in("SUB007", result.stdout, "subcommand extension field should report SUB007")
    finally:
        (CONFIG_DIR / "tmp_extension_fields_allowed.json").unlink(missing_ok=True)


def test_custom_event_requires_schema() -> None:
    simple_payload = {
        "name": "ohos-eventcheck",
        "version": "1.0.0",
        "description": "custom event requires schema",
        "executablePath": "/usr/bin/event-tool",
        "eventTypes": ["progress"],
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_custom_event_requires_schema.json", "w", encoding="utf-8") as f:
        json.dump(simple_payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_custom_event_requires_schema.json"))
        assert_equal(result.returncode, 1, "custom event without schema should fail")
        assert_in("EVT008", result.stdout, "custom event without schema should report EVT008")
    finally:
        (CONFIG_DIR / "tmp_custom_event_requires_schema.json").unlink(missing_ok=True)


def test_builtin_events_allowed_in_events_list() -> None:
    payload = {
        "name": "ohos-builtin-events",
        "version": "1.0.0",
        "description": "builtin events allowed",
        "executablePath": "/usr/bin/builtin-events-tool",
        "eventTypes": ["stdout", "stderr", "exit", "error"],
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_builtin_events_allowed.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_builtin_events_allowed.json"))
        assert_equal(result.returncode, 0, "builtin events in events list should pass")
        assert_in("[PASS]", result.stdout, "builtin events in events list should print PASS")
    finally:
        (CONFIG_DIR / "tmp_builtin_events_allowed.json").unlink(missing_ok=True)


def test_builtin_events_must_not_be_declared_in_event_schemas() -> None:
    payload = {
        "name": "ohos-builtin-event-schema",
        "version": "1.0.0",
        "description": "builtin events must not be declared in eventSchemas",
        "executablePath": "/usr/bin/builtin-event-schema-tool",
        "eventTypes": ["stdout"],
        "eventSchemas": {
            "stdout": {
                "type": "object",
                "properties": {}
            }
        },
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_builtin_events_schema_invalid.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_builtin_events_schema_invalid.json"))
        assert_equal(result.returncode, 1, "builtin events in eventSchemas should fail")
        assert_in("EVT006", result.stdout, "builtin events in eventSchemas should report EVT006")
    finally:
        (CONFIG_DIR / "tmp_builtin_events_schema_invalid.json").unlink(missing_ok=True)


def test_custom_event_name_allows_underscore_dash_dot_after_first_char() -> None:
    payload = {
        "name": "ohos-custom-event-name",
        "version": "1.0.0",
        "description": "custom event name characters",
        "executablePath": "/usr/bin/custom-event-name-tool",
        "eventTypes": ["progress_update-1.ok"],
        "eventSchemas": {
            "progress_update-1.ok": {
                "type": "object",
                "properties": {}
            }
        },
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"}
            }
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}"
            }
        }
    }
    with open(CONFIG_DIR / "tmp_custom_event_name_chars.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_custom_event_name_chars.json"))
        assert_equal(result.returncode, 0, "custom event name with _, -, . after first char should pass")
        assert_in("[PASS]", result.stdout, "custom event name with _, -, . should print PASS")
    finally:
        (CONFIG_DIR / "tmp_custom_event_name_chars.json").unlink(missing_ok=True)


def test_unsupported_event_name_in_events_reports_evt007() -> None:
    payload = {
        "name": "ohos-invalid-event-name",
        "version": "1.0.0",
        "description": "unsupported event name",
        "executablePath": "/usr/bin/invalid-event-name-tool",
        "eventTypes": ["-stdout1ASADDA-"],
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"}
            }
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}"
            }
        }
    }
    with open(CONFIG_DIR / "tmp_invalid_event_name_evt010.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_invalid_event_name_evt010.json"))
        assert_equal(result.returncode, 1, "unsupported event name should fail")
        assert_in("EVT007", result.stdout, "unsupported event name should report EVT007")
    finally:
        (CONFIG_DIR / "tmp_invalid_event_name_evt010.json").unlink(missing_ok=True)


def test_boolean_string_templates_allowed() -> None:
    simple_payload = {
        "name": "ohos-booltmpl",
        "version": "1.0.0",
        "description": "boolean string templates",
        "executablePath": "/usr/bin/bool-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "flag": {"type": "boolean", "default": False},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "flag": "--flag={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_boolean_string_templates.json", "w", encoding="utf-8") as f:
        json.dump(simple_payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_boolean_string_templates.json"))
        assert_equal(result.returncode, 0, "boolean string template should pass")
        assert_in("[PASS]", result.stdout, "boolean string template should print PASS")
    finally:
        (CONFIG_DIR / "tmp_boolean_string_templates.json").unlink(missing_ok=True)


def test_flag_boolean_object_requires_both_branches() -> None:
    payload = {
        "name": "ohos-flagbool",
        "version": "1.0.0",
        "description": "boolean object template branches",
        "executablePath": "/usr/bin/flagbool-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "flag": {"type": "boolean", "default": False},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "flag": {"if_true": "--flag"},
            },
        },
    }
    with open(CONFIG_DIR / "tmp_flag_boolean_requires_both.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_flag_boolean_requires_both.json"))
        assert_equal(result.returncode, 1, "flag boolean object template missing branch should fail")
        assert_in("MAP107", result.stdout, "flag boolean object template missing branch should report MAP107")
    finally:
        (CONFIG_DIR / "tmp_flag_boolean_requires_both.json").unlink(missing_ok=True)


def test_array_items_must_be_simple_type() -> None:
    payload = {
        "name": "ohos-arrayitems",
        "version": "1.0.0",
        "description": "array items simple type",
        "executablePath": "/usr/bin/arrayitems-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "values": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {},
                    },
                },
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "values": "--value={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_array_items_simple_type.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_array_items_simple_type.json"))
        assert_equal(result.returncode, 1, "array items object type should fail")
        assert_in("SCH507", result.stdout, "array items object type should report SCH507")
    finally:
        (CONFIG_DIR / "tmp_array_items_simple_type.json").unlink(missing_ok=True)


def test_array_default_items_must_match_items_type() -> None:
    payload = {
        "name": "ohos-arraydefault",
        "version": "1.0.0",
        "description": "array default items type",
        "executablePath": "/usr/bin/arraydefault-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "values": {
                    "type": "array",
                    "items": {
                        "type": "integer"
                    },
                    "default": [1, "x"]
                }
            }
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "values": "--value={value}"
            }
        }
    }
    with open(CONFIG_DIR / "tmp_array_default_items_type.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_array_default_items_type.json"))
        assert_equal(result.returncode, 1, "array default item type mismatch should fail")
        assert_in("SCH508", result.stdout, "array default item type mismatch should report SCH508")
    finally:
        (CONFIG_DIR / "tmp_array_default_items_type.json").unlink(missing_ok=True)


def test_array_default_must_respect_unique_items() -> None:
    payload = {
        "name": "ohos-arrayunique",
        "version": "1.0.0",
        "description": "array default unique items",
        "executablePath": "/usr/bin/arrayunique-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "values": {
                    "type": "array",
                    "items": {
                        "type": "integer"
                    },
                    "uniqueItems": True,
                    "default": [1, 1]
                }
            }
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "values": "--value={value}"
            }
        }
    }
    with open(CONFIG_DIR / "tmp_array_default_unique_items.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_array_default_unique_items.json"))
        assert_equal(result.returncode, 1, "array default duplicates with uniqueItems true should fail")
        assert_in("SCH509", result.stdout, "array default duplicates with uniqueItems true should report SCH509")
    finally:
        (CONFIG_DIR / "tmp_array_default_unique_items.json").unlink(missing_ok=True)


def test_object_schema_must_define_properties() -> None:
    payload = {
        "name": "ohos-objectprops",
        "version": "1.0.0",
        "description": "object schema must define properties",
        "executablePath": "/usr/bin/objectprops-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "cfg": {
                    "type": "object"
                }
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "cfg": "--cfg={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_object_schema_properties.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_object_schema_properties.json"))
        assert_equal(result.returncode, 1, "object schema without properties should fail")
        assert_in("SCH606", result.stdout, "object schema without properties should report SCH606")
    finally:
        (CONFIG_DIR / "tmp_object_schema_properties.json").unlink(missing_ok=True)


def test_flattened_only_allows_specific_fields() -> None:
    payload = {
        "name": "ohos-flattened",
        "version": "1.0.0",
        "description": "flattened allowed fields",
        "executablePath": "/usr/bin/flattened-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "cfg": {
                    "type": "object",
                    "properties": {
                        "sub": {"type": "string"},
                    },
                },
            },
        },
        "argMapping": {
            "type": "flattened",
            "separator": ".",
            "templates": {
                "cfg.sub": "--cfg-sub={value}",
            },
            "extra": True,
        },
    }
    with open(CONFIG_DIR / "tmp_flattened_allowed_fields.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_flattened_allowed_fields.json"))
        assert_equal(result.returncode, 1, "flattened mapping extra field should fail")
        assert_in("MAP313", result.stdout, "flattened mapping extra field should report MAP313")
    finally:
        (CONFIG_DIR / "tmp_flattened_allowed_fields.json").unlink(missing_ok=True)


def test_output_schema_without_result_event_is_allowed() -> None:
    payload = {
        "name": "ohos-outputevent",
        "version": "1.0.0",
        "description": "output schema without result event is allowed",
        "executablePath": "/usr/bin/outputevent-tool",
        "eventTypes": ["progress"],
        "eventSchemas": {
            "progress": {
                "type": "object",
                "properties": {
                    "status": {"type": "string"},
                },
            },
        },
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "status": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_output_without_result_event.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_output_without_result_event.json"))
        assert_equal(result.returncode, 0, "output without result event should pass")
        assert_in("[PASS]", result.stdout, "output without result event should print PASS")
    finally:
        (CONFIG_DIR / "tmp_output_without_result_event.json").unlink(missing_ok=True)


def test_result_event_requires_event_schema() -> None:
    payload = {
        "name": "ohos-resultevent",
        "version": "1.0.0",
        "description": "result event requires eventSchemas.result",
        "executablePath": "/usr/bin/resultevent-tool",
        "eventTypes": ["result"],
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "status": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_result_event_requires_schema.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_result_event_requires_schema.json"))
        assert_equal(result.returncode, 1, "result event without event schema should fail")
        assert_in("EVT008", result.stdout, "result event without event schema should report EVT008")
    finally:
        (CONFIG_DIR / "tmp_result_event_requires_schema.json").unlink(missing_ok=True)


def test_invalid_mixed_mapping() -> None:
    result = run_validator(str(CONFIG_DIR / "invalid_mixed_mapping.json"))
    assert_equal(result.returncode, 1, "mixed mapping config should fail")
    assert_in("MAP501", result.stdout, "mixed mapping without mappings should report MAP501")


def test_valid_mixed_mapping() -> None:
    result = run_validator(str(CONFIG_DIR / "valid_mixed_mapping.json"))
    assert_equal(result.returncode, 0, "valid mixed mapping should pass")
    assert_in("[PASS]", result.stdout, "valid mixed mapping should print PASS")


def test_mixed_duplicate_positional_order() -> None:
    payload = {
        "name": "ohos-mixeddup",
        "version": "1.0.0",
        "description": "duplicate positional order",
        "executablePath": "/usr/bin/mixeddup-tool",
        "inputSchema": {
            "type": "object",
            "properties": {
                "action": {"type": "string"},
                "target": {"type": "string"},
            },
        },
        "argMapping": {
            "type": "mixed",
            "mappings": {
                "action": {"mode": "positional", "order": 1},
                "target": {"mode": "positional", "order": 1},
            },
        },
    }
    with open(CONFIG_DIR / "tmp_mixed_duplicate_positional_order.json", "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_mixed_duplicate_positional_order.json"))
        assert_equal(result.returncode, 1, "duplicate positional order should fail")
        assert_in("MAP510", result.stdout, "duplicate positional order should report MAP510")
    finally:
        (CONFIG_DIR / "tmp_mixed_duplicate_positional_order.json").unlink(missing_ok=True)


def test_invalid_required_schema() -> None:
    result = run_validator(str(CONFIG_DIR / "invalid_missing_required_schema.json"))
    assert_equal(result.returncode, 1, "invalid required schema should fail")
    assert_in("SCH604", result.stdout, "missing required property should report SCH604")


def test_invalid_unknown_template_field() -> None:
    result = run_validator(str(CONFIG_DIR / "invalid_unknown_template_field.json"))
    assert_equal(result.returncode, 1, "unknown template field should fail")
    assert_in("MAP104", result.stdout, "unknown template field should report MAP104")


def main() -> int:
    tests = [
        test_valid_simple,
        test_valid_subcommand_json_output,
        test_collect_all_json_in_directory,
        test_timeout_integer_required,
        test_empty_input_and_mapping_allowed,
        test_extension_fields_rejected,
        test_custom_event_requires_schema,
        test_boolean_string_templates_allowed,
        test_flag_boolean_object_requires_both_branches,
        test_array_items_must_be_simple_type,
        test_flattened_only_allows_specific_fields,
        test_output_schema_without_result_event_is_allowed,
        test_result_event_requires_event_schema,
        test_invalid_mixed_mapping,
        test_valid_mixed_mapping,
        test_mixed_duplicate_positional_order,
        test_invalid_required_schema,
        test_invalid_unknown_template_field,
    ]
    for test in tests:
        test()
    print(f"PASS {len(tests)} tests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
