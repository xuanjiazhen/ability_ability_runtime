from __future__ import annotations

import json
import subprocess
import sys
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


def test_timeout_number_allowed() -> None:
    simple_payload = {
        "name": "ohos-timeout",
        "version": "1.0.0",
        "description": "timeout number",
        "executable": "/usr/bin/timeout-tool",
        "timeout": 0.5,
        "input": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argumentMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
            },
        },
    }
    with open(CONFIG_DIR / "tmp_timeout_number.json", "w", encoding="utf-8") as f:
        json.dump(simple_payload, f, ensure_ascii=False, indent=2)
    try:
        result = run_validator(str(CONFIG_DIR / "tmp_timeout_number.json"))
        assert_equal(result.returncode, 0, "number timeout should pass")
        assert_in("[PASS]", result.stdout, "number timeout should print PASS")
    finally:
        (CONFIG_DIR / "tmp_timeout_number.json").unlink(missing_ok=True)


def test_custom_event_requires_schema() -> None:
    simple_payload = {
        "name": "ohos-eventcheck",
        "version": "1.0.0",
        "description": "custom event requires schema",
        "executable": "/usr/bin/event-tool",
        "events": ["progress"],
        "input": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
            },
        },
        "argumentMapping": {
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


def test_invalid_mixed_mapping() -> None:
    result = run_validator(str(CONFIG_DIR / "invalid_mixed_mapping.json"))
    assert_equal(result.returncode, 1, "mixed mapping config should fail")
    assert_in("MAP003", result.stdout, "mixed mapping should report MAP003")


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
        test_timeout_number_allowed,
        test_custom_event_requires_schema,
        test_invalid_mixed_mapping,
        test_invalid_required_schema,
        test_invalid_unknown_template_field,
    ]
    for test in tests:
        test()
    print(f"PASS {len(tests)} tests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
