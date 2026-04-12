from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MAIN = ROOT / "validator" / "main.py"
TEST_DIR = Path(__file__).resolve().parent


def run_validator(*args: str) -> subprocess.CompletedProcess[str]:
    cmd = [sys.executable, str(MAIN), "validate", *args]
    return subprocess.run(cmd, text=True, capture_output=True, cwd=ROOT.parents[1])


def assert_equal(actual, expected, message: str) -> None:
    if actual != expected:
        raise AssertionError(f"{message}: expected={expected!r} actual={actual!r}")


def assert_in(needle: str, haystack: str, message: str) -> None:
    if needle not in haystack:
        raise AssertionError(f"{message}: missing {needle!r}\nOutput:\n{haystack}")


def test_valid_simple() -> None:
    result = run_validator(str(TEST_DIR / "valid_simple_config.json"))
    assert_equal(result.returncode, 0, "valid simple config should pass")
    assert_in("[PASS]", result.stdout, "valid simple config should print PASS")


def test_valid_subcommand_json_output() -> None:
    result = run_validator(str(TEST_DIR / "valid_subcommand_config.json"), "--json")
    assert_equal(result.returncode, 0, "valid subcommand config should pass")
    payload = json.loads(result.stdout)
    assert_equal(payload["results"][0]["valid"], True, "json output should mark config as valid")


def test_invalid_mixed_mapping() -> None:
    result = run_validator(str(TEST_DIR / "invalid_mixed_mapping.json"))
    assert_equal(result.returncode, 1, "mixed mapping config should fail")
    assert_in("MAP003", result.stdout, "mixed mapping should report MAP003")


def test_invalid_required_schema() -> None:
    result = run_validator(str(TEST_DIR / "invalid_missing_required_schema.json"))
    assert_equal(result.returncode, 1, "invalid required schema should fail")
    assert_in("SCH604", result.stdout, "missing required property should report SCH604")


def test_invalid_unknown_template_field() -> None:
    result = run_validator(str(TEST_DIR / "invalid_unknown_template_field.json"))
    assert_equal(result.returncode, 1, "unknown template field should fail")
    assert_in("MAP104", result.stdout, "unknown template field should report MAP104")


def main() -> int:
    tests = [
        test_valid_simple,
        test_valid_subcommand_json_output,
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
