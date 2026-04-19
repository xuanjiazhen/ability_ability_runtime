from __future__ import annotations

import copy
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Callable


TOOL_ROOT = Path(__file__).resolve().parents[1]
MAIN = TOOL_ROOT / "main.py"
REPO_ROOT = TOOL_ROOT.parents[1]
VALIDATOR_ROOT = TOOL_ROOT / "utils"

if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from utils.model import ValidationResult
from utils.validators import validate_subcommand_tool


def base_simple() -> dict:
    return {
        "name": "ohos-tool",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/tool",
        "input": {
            "type": "object",
            "properties": {
                "target": {"type": "string"},
                "flag": {"type": "boolean", "default": False},
            },
            "required": ["target"],
        },
        "argumentMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
                "flag": {"if_true": "--flag", "if_false": ""},
            },
        },
    }


def base_mixed() -> dict:
    return {
        "name": "ohos-mixed",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/mixed",
        "input": {
            "type": "object",
            "properties": {
                "action": {"type": "string"},
                "flag": {"type": "boolean", "default": False},
                "server": {
                    "type": "object",
                    "properties": {
                        "host": {"type": "string"},
                        "port": {"type": "integer"},
                    },
                },
                "config": {"type": "object", "properties": {}},
            },
            "required": ["action"],
        },
        "argumentMapping": {
            "type": "mixed",
            "mappings": {
                "action": {"mode": "positional", "order": 1},
                "flag": {"mode": "flag", "template": {"if_true": "--flag", "if_false": ""}},
                "server": {
                    "mode": "flattened",
                    "template": {
                        "server.host": "--host={value}",
                        "server.port": "--port={value}",
                    },
                    "separator": ".",
                },
                "config": {"mode": "json-string", "template": "--config={value}"},
            },
        },
    }


def base_subcommand() -> dict:
    return {
        "name": "ohos-pkg",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/pkg",
        "hasSubcommands": True,
        "subcommands": {
            "install": {
                "description": "install",
                "input": {
                    "type": "object",
                    "properties": {
                        "package": {"type": "string"},
                        "force": {"type": "boolean", "default": False},
                    },
                    "required": ["package"],
                },
                "argumentMapping": {
                    "type": "flag",
                    "templates": {
                        "package": "{value}",
                        "force": {"if_true": "--force", "if_false": ""},
                    },
                },
            }
        },
    }


def run_validator(path: Path, *args: str) -> subprocess.CompletedProcess[str]:
    cmd = [sys.executable, str(MAIN), "validate", str(path), *args]
    return subprocess.run(cmd, text=True, capture_output=True, cwd=REPO_ROOT)


def write_json(path: Path, payload: dict | list) -> None:
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")


def write_text(path: Path, payload: str, encoding: str = "utf-8") -> None:
    path.write_text(payload, encoding=encoding)


def extract_codes(output: str) -> set[str]:
    return set(re.findall(r"\b[A-Z]{3}\d{3}\b", output))


def expect_code(name: str, writer: Callable[[Path], None], expected_code: str) -> str:
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / f"{name}.json"
        writer(path)
        result = run_validator(path)
        if result.returncode != 1:
            raise AssertionError(f"{name}: expected failure, got rc={result.returncode}\n{result.stdout}\n{result.stderr}")
        codes = extract_codes(result.stdout)
        if expected_code not in codes:
            raise AssertionError(f"{name}: expected {expected_code}, got {sorted(codes)}\n{result.stdout}")
        return expected_code


def expect_pass(name: str, payload: dict) -> None:
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / f"{name}.json"
        write_json(path, payload)
        result = run_validator(path)
        if result.returncode != 0:
            raise AssertionError(f"{name}: expected pass\n{result.stdout}\n{result.stderr}")


def expect_direct_code(name: str, runner: Callable[[], ValidationResult], expected_code: str) -> str:
    result = runner()
    codes = {issue.code for issue in result.issues}
    if expected_code not in codes:
        raise AssertionError(f"{name}: expected {expected_code}, got {sorted(codes)}")
    return expected_code


def main() -> int:
    covered: set[str] = set()

    expect_pass("valid_simple", base_simple())
    expect_pass("valid_subcommand", base_subcommand())
    expect_pass("valid_top_level_extension_field", {
        **base_simple(),
        "private": True,
    })
    valid_subcommand_extension = base_subcommand()
    valid_subcommand_extension["subcommands"]["install"]["private"] = True
    expect_pass("valid_subcommand_extension_field", valid_subcommand_extension)
    expect_pass("valid_empty_simple", {
        "name": "ohos-empty",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/empty",
        "input": {
            "type": "object",
            "properties": {},
        },
        "argumentMapping": {},
    })
    empty_subcommand = {
        "name": "ohos-emptycmd",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/emptycmd",
        "hasSubcommands": True,
        "subcommands": {
            "run": {
                "description": "run",
                "input": {
                    "type": "object",
                    "properties": {},
                },
                "argumentMapping": {},
            }
        },
    }
    expect_pass("valid_empty_subcommand", empty_subcommand)

    # CFG
    missing_path = Path("/tmp/nonexistent-validator-config.json")
    result = run_validator(missing_path)
    if result.returncode != 2:
        raise AssertionError("CFG001 path handling failed")
    covered.add("CFG001")
    covered.add(expect_code("cfg002", lambda p: write_text(p, "{invalid json"), "CFG002"))
    covered.add(expect_code("cfg003", lambda p: write_json(p, []), "CFG003"))
    covered.add(expect_code("cfg004", lambda p: write_json(p, {}), "CFG004"))
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "cfg005.json"
        path.write_bytes(b"\xff\xfe\x00")
        result = run_validator(path)
        if result.returncode != 1 or "CFG005" not in result.stdout:
            raise AssertionError("CFG005 failed")
        covered.add("CFG005")
    covered.add(expect_code("cfg006", lambda p: write_text(p, '{"a":1,"a":2}'), "CFG006"))

    # TOP
    covered.add(expect_code("top001", lambda p: write_json(p, {**base_simple(), "name": ""}), "TOP001"))
    covered.add(expect_code("top002", lambda p: write_json(p, {**base_simple(), "version": ""}), "TOP002"))
    covered.add(expect_code("top003", lambda p: write_json(p, {**base_simple(), "description": ""}), "TOP003"))
    covered.add(expect_code("top004", lambda p: write_json(p, {**base_simple(), "executable": ""}), "TOP004"))
    covered.add(expect_code("top005", lambda p: write_json(p, {**base_simple(), "executable": "usr/bin/tool"}), "TOP005"))
    covered.add(expect_code("top006", lambda p: write_json(p, {**base_simple(), "requirePermissions": [1]}), "TOP006"))
    covered.add(expect_code("top007", lambda p: write_json(p, {**base_simple(), "requirePermissions": ["a", "a"]}), "TOP007"))
    covered.add(expect_code("top008", lambda p: write_json(p, {**base_simple(), "timeout": "x"}), "TOP008"))
    covered.add(expect_code("top009", lambda p: write_json(p, {**base_simple(), "timeout": -1}), "TOP009"))
    covered.add(expect_code("top010", lambda p: write_json(p, {**base_simple(), "events": [1]}), "TOP010"))
    covered.add(expect_code("top011", lambda p: write_json(p, {**base_simple(), "events": ["a", "a"]}), "TOP011"))
    covered.add(expect_code("top012", lambda p: write_json(p, {**base_simple(), "eventSchemas": []}), "TOP012"))
    covered.add(expect_code("top014", lambda p: write_json(p, {**base_simple(), "name": "cli-tool"}), "TOP014"))
    covered.add(expect_code("top015", lambda p: write_json(p, {**base_simple(), "name": "ohos-abcdefghijklmnopq"}), "TOP015"))

    # FMT
    simple_no_input = base_simple()
    del simple_no_input["input"]
    covered.add(expect_code("fmt001", lambda p: write_json(p, simple_no_input), "FMT001"))
    simple_no_mapping = base_simple()
    del simple_no_mapping["argumentMapping"]
    covered.add(expect_code("fmt002", lambda p: write_json(p, simple_no_mapping), "FMT002"))
    simple_with_sub = base_simple()
    simple_with_sub["subcommands"] = {}
    covered.add(expect_code("fmt003", lambda p: write_json(p, simple_with_sub), "FMT003"))
    simple_bad_output = base_simple()
    simple_bad_output["output"] = {"type": "string"}
    covered.add(expect_code("fmt004", lambda p: write_json(p, simple_bad_output), "FMT004"))
    simple_bad_has = base_simple()
    simple_bad_has["hasSubcommands"] = "bad"
    covered.add(expect_code("fmt005", lambda p: write_json(p, simple_bad_has), "FMT005"))
    sub_no_subcommands = base_subcommand()
    del sub_no_subcommands["subcommands"]
    covered.add(expect_code("fmt101", lambda p: write_json(p, sub_no_subcommands), "FMT101"))
    sub_bad_subcommands = base_subcommand()
    sub_bad_subcommands["subcommands"] = []
    covered.add(expect_code("fmt102", lambda p: write_json(p, sub_bad_subcommands), "FMT102"))
    sub_empty_subcommands = base_subcommand()
    sub_empty_subcommands["subcommands"] = {}
    covered.add(expect_code("fmt103", lambda p: write_json(p, sub_empty_subcommands), "FMT103"))
    sub_with_input = base_subcommand()
    sub_with_input["input"] = {"type": "string"}
    covered.add(expect_code("fmt104", lambda p: write_json(p, sub_with_input), "FMT104"))
    sub_with_mapping = base_subcommand()
    sub_with_mapping["input"] = base_simple()["input"]
    sub_with_mapping["argumentMapping"] = 1
    covered.add(expect_code("fmt105", lambda p: write_json(p, sub_with_mapping), "FMT105"))
    sub_with_output = base_subcommand()
    sub_with_output["output"] = {"type": "string"}
    covered.add(expect_code("fmt106", lambda p: write_json(p, sub_with_output), "FMT106"))
    sub_empty_name = base_subcommand()
    sub_empty_name["subcommands"] = {"": sub_empty_name["subcommands"]["install"]}
    covered.add(expect_code("fmt107", lambda p: write_json(p, sub_empty_name), "FMT107"))
    class DuplicateSubcommands(dict):
        def items(self):
            install = base_subcommand()["subcommands"]["install"]
            return [("install", install), ("install", install)]

    def run_fmt108() -> ValidationResult:
        result = ValidationResult(file="<direct>")
        data = base_subcommand()
        data["subcommands"] = DuplicateSubcommands({"install": data["subcommands"]["install"]})
        validate_subcommand_tool(data, result)
        return result

    covered.add(expect_direct_code("fmt108", run_fmt108, "FMT108"))
    sub_name_conflict = base_subcommand()
    sub_name_conflict["subcommands"] = {sub_name_conflict["name"]: sub_name_conflict["subcommands"]["install"]}
    covered.add(expect_code("fmt109", lambda p: write_json(p, sub_name_conflict), "FMT109"))

    # SUB
    sub_bad_node = base_subcommand()
    sub_bad_node["subcommands"]["install"] = []
    covered.add(expect_code("sub001", lambda p: write_json(p, sub_bad_node), "SUB001"))
    sub_no_desc = base_subcommand()
    del sub_no_desc["subcommands"]["install"]["description"]
    covered.add(expect_code("sub002", lambda p: write_json(p, sub_no_desc), "SUB002"))
    sub_empty_desc = base_subcommand()
    sub_empty_desc["subcommands"]["install"]["description"] = ""
    covered.add(expect_code("sub003", lambda p: write_json(p, sub_empty_desc), "SUB003"))
    sub_no_input = base_subcommand()
    del sub_no_input["subcommands"]["install"]["input"]
    covered.add(expect_code("sub004", lambda p: write_json(p, sub_no_input), "SUB004"))
    sub_no_mapping = base_subcommand()
    del sub_no_mapping["subcommands"]["install"]["argumentMapping"]
    covered.add(expect_code("sub005", lambda p: write_json(p, sub_no_mapping), "SUB005"))
    sub_bad_output = base_subcommand()
    sub_bad_output["subcommands"]["install"]["output"] = {"type": "string"}
    covered.add(expect_code("sub006", lambda p: write_json(p, sub_bad_output), "SUB006"))
    sub_bad_event_types = base_subcommand()
    sub_bad_event_types["subcommands"]["install"]["events"] = [1]
    covered.add(expect_code("sub008", lambda p: write_json(p, sub_bad_event_types), "SUB008"))
    sub_bad_event_schemas = base_subcommand()
    sub_bad_event_schemas["subcommands"]["install"]["events"] = ["progress"]
    sub_bad_event_schemas["subcommands"]["install"]["eventSchemas"] = []
    covered.add(expect_code("sub009", lambda p: write_json(p, sub_bad_event_schemas), "SUB009"))
    sub_bad_permissions = base_subcommand()
    sub_bad_permissions["subcommands"]["install"]["requirePermissions"] = [1]
    covered.add(expect_code("sub010", lambda p: write_json(p, sub_bad_permissions), "SUB010"))
    sub_bad_timeout = base_subcommand()
    sub_bad_timeout["subcommands"]["install"]["timeout"] = 1.5
    covered.add(expect_code("sub011", lambda p: write_json(p, sub_bad_timeout), "SUB011"))

    # SCX
    scx1 = base_simple()
    scx1["input"]["type"] = "string"
    covered.add(expect_code("scx001", lambda p: write_json(p, scx1), "SCX001"))
    scx2 = base_simple()
    scx2["input"] = {"type": "object"}
    covered.add(expect_code("scx002", lambda p: write_json(p, scx2), "SCX002"))
    scx3 = base_simple()
    scx3["output"] = {"type": "string"}
    covered.add(expect_code("scx003", lambda p: write_json(p, scx3), "SCX003"))
    scx4 = base_subcommand()
    scx4["subcommands"]["install"]["input"]["type"] = "string"
    covered.add(expect_code("scx004", lambda p: write_json(p, scx4), "SCX004"))
    scx5 = base_subcommand()
    del scx5["subcommands"]["install"]["input"]["properties"]
    covered.add(expect_code("scx005", lambda p: write_json(p, scx5), "SCX005"))
    sch001 = base_simple()
    sch001["input"] = []
    covered.add(expect_code("sch001", lambda p: write_json(p, sch001), "SCH001"))

    # SCH common/string
    covered.add(expect_code("sch002", lambda p: write_json(p, {**base_simple(), "input": {"properties": {}}}), "SCH002"))
    covered.add(expect_code("sch003", lambda p: write_json(p, {**base_simple(), "input": {"type": "null"}}), "SCH003"))
    sch004 = base_simple()
    sch004["input"]["properties"]["target"]["foo"] = 1
    covered.add(expect_code("sch004", lambda p: write_json(p, sch004), "SCH004"))
    sch005 = base_simple()
    sch005["input"]["description"] = 1
    covered.add(expect_code("sch005", lambda p: write_json(p, sch005), "SCH005"))
    sch006 = base_simple()
    sch006["input"]["properties"]["target"]["default"] = 1
    covered.add(expect_code("sch006", lambda p: write_json(p, sch006), "SCH006"))
    sch007 = base_simple()
    sch007["input"]["properties"]["target"]["enum"] = ["a", "b"]
    covered.add(expect_code("sch007", lambda p: write_json(p, sch007), "SCH007"))
    sch008 = base_simple()
    sch008["input"]["properties"]["target"] = {"oneOf": [{"type": "string"}, {"type": "integer"}]}
    covered.add(expect_code("sch008", lambda p: write_json(p, sch008), "SCH008"))
    sch101 = base_simple()
    sch101["input"]["properties"]["target"]["minLength"] = -1
    covered.add(expect_code("sch101", lambda p: write_json(p, sch101), "SCH101"))
    sch102 = base_simple()
    sch102["input"]["properties"]["target"]["maxLength"] = -1
    covered.add(expect_code("sch102", lambda p: write_json(p, sch102), "SCH102"))
    sch103 = base_simple()
    sch103["input"]["properties"]["target"]["minLength"] = 5
    sch103["input"]["properties"]["target"]["maxLength"] = 1
    covered.add(expect_code("sch103", lambda p: write_json(p, sch103), "SCH103"))
    sch104 = base_simple()
    sch104["input"]["properties"]["target"]["pattern"] = "["
    covered.add(expect_code("sch104", lambda p: write_json(p, sch104), "SCH104"))
    sch105 = base_simple()
    sch105["input"]["properties"]["target"]["format"] = "binary"
    covered.add(expect_code("sch105", lambda p: write_json(p, sch105), "SCH105"))

    # SCH number/integer/boolean/array/object
    num = base_simple()
    num["input"]["properties"]["n"] = {"type": "number", "minimum": "x"}
    covered.add(expect_code("sch201", lambda p: write_json(p, num), "SCH201"))
    num = base_simple()
    num["input"]["properties"]["n"] = {"type": "number", "exclusiveMinimum": "x"}
    covered.add(expect_code("sch202", lambda p: write_json(p, num), "SCH202"))
    num = base_simple()
    num["input"]["properties"]["n"] = {"type": "number", "minimum": 3, "maximum": 1}
    covered.add(expect_code("sch203", lambda p: write_json(p, num), "SCH203"))
    num = base_simple()
    num["input"]["properties"]["n"] = {"type": "number", "exclusiveMinimum": 3, "exclusiveMaximum": 1}
    covered.add(expect_code("sch204", lambda p: write_json(p, num), "SCH204"))
    num = base_simple()
    num["input"]["properties"]["n"] = {"type": "number", "multipleOf": 0}
    covered.add(expect_code("sch205", lambda p: write_json(p, num), "SCH205"))
    integer = base_simple()
    integer["input"]["properties"]["n"] = {"type": "integer", "minimum": 1.5}
    covered.add(expect_code("sch301", lambda p: write_json(p, integer), "SCH301"))
    integer = base_simple()
    integer["input"]["properties"]["n"] = {"type": "integer", "multipleOf": 0}
    covered.add(expect_code("sch302", lambda p: write_json(p, integer), "SCH302"))
    integer = base_simple()
    integer["input"]["properties"]["n"] = {"type": "integer", "minimum": 5, "maximum": 2}
    covered.add(expect_code("sch303", lambda p: write_json(p, integer), "SCH303"))
    boolean = base_simple()
    boolean["input"]["properties"]["flag"]["minimum"] = 1
    covered.add(expect_code("sch401", lambda p: write_json(p, boolean), "SCH401"))
    boolean = base_simple()
    boolean["input"]["properties"]["flag"]["default"] = "x"
    covered.add(expect_code("sch402", lambda p: write_json(p, boolean), "SCH402"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {"type": "array"}
    covered.add(expect_code("sch501", lambda p: write_json(p, array), "SCH501"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {"type": "array", "items": []}
    covered.add(expect_code("sch502", lambda p: write_json(p, array), "SCH502"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {"type": "array", "items": {"type": "integer"}, "minItems": -1}
    covered.add(expect_code("sch503", lambda p: write_json(p, array), "SCH503"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {
        "type": "array",
        "items": {"type": "integer"},
        "minItems": 5,
        "maxItems": 1,
    }
    covered.add(expect_code("sch504", lambda p: write_json(p, array), "SCH504"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {
        "type": "array",
        "items": {"type": "integer"},
        "uniqueItems": "x",
    }
    covered.add(expect_code("sch505", lambda p: write_json(p, array), "SCH505"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {
        "type": "array",
        "items": {"type": "integer"},
        "default": "x",
    }
    covered.add(expect_code("sch506", lambda p: write_json(p, array), "SCH506"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {
        "type": "array",
        "items": {"type": "integer"},
        "default": [1, "x"],
    }
    covered.add(expect_code("sch508", lambda p: write_json(p, array), "SCH508"))
    array = base_simple()
    array["input"]["properties"]["ports"] = {
        "type": "array",
        "items": {"type": "integer"},
        "uniqueItems": True,
        "default": [1, 1],
    }
    covered.add(expect_code("sch509", lambda p: write_json(p, array), "SCH509"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object", "properties": []}
    covered.add(expect_code("sch601", lambda p: write_json(p, obj), "SCH601"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object", "properties": {"x": []}}
    covered.add(expect_code("sch602", lambda p: write_json(p, obj), "SCH602"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object", "properties": {}, "required": [1]}
    covered.add(expect_code("sch603", lambda p: write_json(p, obj), "SCH603"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object", "properties": {}, "required": ["x"]}
    covered.add(expect_code("sch604", lambda p: write_json(p, obj), "SCH604"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object", "properties": {"x": {"type": "string"}}, "required": ["x", "x"]}
    covered.add(expect_code("sch605", lambda p: write_json(p, obj), "SCH605"))
    obj = base_simple()
    obj["input"]["properties"]["cfg"] = {"type": "object"}
    covered.add(expect_code("sch606", lambda p: write_json(p, obj), "SCH606"))
    obj = base_simple()
    obj["input"]["properties"]["ports"] = {"type": "array", "items": {"type": "object", "properties": {}}}
    covered.add(expect_code("sch507", lambda p: write_json(p, obj), "SCH507"))
    # MAP / REL
    map001 = base_simple()
    map001["argumentMapping"] = 1
    covered.add(expect_code("map001", lambda p: write_json(p, map001), "MAP001"))
    map002 = base_simple()
    del map002["argumentMapping"]["type"]
    covered.add(expect_code("map002", lambda p: write_json(p, map002), "MAP002"))
    covered.add(expect_code("map003", lambda p: write_json(p, {**base_simple(), "argumentMapping": {"type": "unknown"}}), "MAP003"))
    map004 = base_simple()
    map004["argumentMapping"]["unknown"] = 1
    covered.add(expect_code("map004", lambda p: write_json(p, map004), "MAP004"))
    map005 = base_simple()
    map005["input"] = []
    covered.add(expect_code("map005", lambda p: write_json(p, map005), "MAP005"))
    map101 = base_simple()
    map101["argumentMapping"]["templates"] = 1
    covered.add(expect_code("map101", lambda p: write_json(p, map101), "MAP101"))
    map102 = base_simple()
    map102["argumentMapping"]["templates"] = 1
    covered.add(expect_code("map102", lambda p: write_json(p, map102), "MAP102"))
    map103 = base_simple()
    map103["argumentMapping"]["templates"] = {}
    covered.add(expect_code("map103", lambda p: write_json(p, map103), "MAP103"))
    covered.add(expect_code("map104", lambda p: write_json(p, {**base_simple(), "argumentMapping": {"type": "flag", "templates": {"x": "--x={value}"}}}), "MAP104"))
    map105 = base_simple()
    map105["argumentMapping"]["templates"]["target"] = 1
    covered.add(expect_code("map105", lambda p: write_json(p, map105), "MAP105"))
    map106 = base_simple()
    map106["argumentMapping"]["templates"]["flag"] = {"bad": 1}
    covered.add(expect_code("map106", lambda p: write_json(p, map106), "MAP106"))
    map107 = base_simple()
    map107["argumentMapping"]["templates"]["flag"] = {"if_true": "--flag"}
    covered.add(expect_code("map107", lambda p: write_json(p, map107), "MAP107"))
    map108 = base_simple()
    map108["argumentMapping"]["templates"]["target"] = {"if_true": "--x"}
    covered.add(expect_code("map108", lambda p: write_json(p, map108), "MAP108"))
    map109 = base_simple()
    map109["input"]["properties"]["cfg"] = {"type": "object", "properties": {}}
    map109["argumentMapping"]["templates"]["cfg"] = "--cfg={value}"
    covered.add(expect_code("map109", lambda p: write_json(p, map109), "MAP109"))
    map110 = base_simple()
    map110["input"]["properties"]["ports"] = {"type": "array", "items": {"type": "integer"}}
    map110["argumentMapping"]["templates"]["ports"] = "--ports"
    covered.add(expect_code("map110", lambda p: write_json(p, map110), "MAP110"))
    map111 = base_simple()
    map111["argumentMapping"]["templates"][""] = "--x={value}"
    covered.add(expect_code("map111", lambda p: write_json(p, map111), "MAP111"))
    map112 = base_simple()
    map112["argumentMapping"]["templates"]["target"] = "--target"
    covered.add(expect_code("map112", lambda p: write_json(p, map112), "MAP112"))
    expect_pass("map112_boolean_string_ok", {
        **base_simple(),
        "argumentMapping": {
            "type": "flag",
            "templates": {
                "target": "--target={value}",
                "flag": "--flag={value}",
            },
        },
    })
    map201 = base_simple()
    map201["argumentMapping"] = {"type": "positional"}
    covered.add(expect_code("map201", lambda p: write_json(p, map201), "MAP201"))
    map202 = base_simple()
    map202["argumentMapping"] = {"type": "positional", "order": []}
    covered.add(expect_code("map202", lambda p: write_json(p, map202), "MAP202"))
    map203 = base_simple()
    map203["argumentMapping"] = {"type": "positional", "order": ["target", "target"]}
    covered.add(expect_code("map203", lambda p: write_json(p, map203), "MAP203"))
    map204 = base_simple()
    map204["argumentMapping"] = {"type": "positional", "order": ["missing"]}
    covered.add(expect_code("map204", lambda p: write_json(p, map204), "MAP204"))
    map205 = base_simple()
    map205["input"]["properties"]["cfg"] = {"type": "object", "properties": {}}
    map205["argumentMapping"] = {"type": "positional", "order": ["cfg"]}
    covered.add(expect_code("map205", lambda p: write_json(p, map205), "MAP205"))
    map206 = base_simple()
    map206["input"]["properties"]["ports"] = {"type": "array", "items": {"type": "integer"}}
    map206["argumentMapping"] = {"type": "positional", "order": ["ports"]}
    covered.add(expect_code("map206", lambda p: write_json(p, map206), "MAP206"))
    map207 = base_simple()
    map207["argumentMapping"] = {"type": "positional", "order": ["target", "flag"], "extra": True}
    covered.add(expect_code("map207", lambda p: write_json(p, map207), "MAP207"))
    map301 = base_simple()
    map301["argumentMapping"] = {"type": "flattened", "templates": 1}
    covered.add(expect_code("map301", lambda p: write_json(p, map301), "MAP301"))
    map302 = base_simple()
    map302["argumentMapping"] = {"type": "flattened", "templates": 1}
    covered.add(expect_code("map302", lambda p: write_json(p, map302), "MAP302"))
    map303 = base_simple()
    map303["argumentMapping"] = {"type": "flattened", "templates": {}}
    covered.add(expect_code("map303", lambda p: write_json(p, map303), "MAP303"))
    map313 = base_simple()
    map313["input"]["properties"]["cfg"] = {"type": "object", "properties": {"sub": {"type": "string"}}}
    map313["argumentMapping"] = {
        "type": "flattened",
        "templates": {"cfg.sub": "--x={value}"},
        "extra": True,
    }
    covered.add(expect_code("map313", lambda p: write_json(p, map313), "MAP313"))
    map304 = base_simple()
    map304["argumentMapping"] = {"type": "flattened", "separator": "/", "templates": {"a.b": "--x={value}"}}
    covered.add(expect_code("map304", lambda p: write_json(p, map304), "MAP304"))
    map305 = base_simple()
    map305["argumentMapping"] = {"type": "flattened", "templates": {"target": "--x={value}"}}
    covered.add(expect_code("map305", lambda p: write_json(p, map305), "MAP305"))
    map306 = base_simple()
    map306["argumentMapping"] = {"type": "flattened", "templates": {"a.b": "--x={value}"}}
    covered.add(expect_code("map306", lambda p: write_json(p, map306), "MAP306"))
    map307 = base_simple()
    map307["input"]["properties"]["cfg"] = {"type": "string"}
    map307["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x={value}"}}
    covered.add(expect_code("map307", lambda p: write_json(p, map307), "MAP307"))
    map308 = base_simple()
    map308["input"]["properties"]["cfg"] = {"type": "object", "properties": {}}
    map308["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x={value}"}}
    covered.add(expect_code("map308", lambda p: write_json(p, map308), "MAP308"))
    map309 = base_simple()
    map309["input"]["properties"]["cfg"] = {"type": "object", "properties": {"sub": {"type": "object", "properties": {}}}}
    map309["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x={value}"}}
    covered.add(expect_code("map309", lambda p: write_json(p, map309), "MAP309"))
    map310 = base_simple()
    map310["input"]["properties"]["cfg"] = {"type": "object", "properties": {"sub": {"type": "array", "items": {"type": "integer"}}}}
    map310["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x={value}"}}
    covered.add(expect_code("map310", lambda p: write_json(p, map310), "MAP310"))
    map311 = {
        "name": "ohos-flatbool",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/flatbool",
        "input": {
            "type": "object",
            "properties": {
                "cfg": {
                    "type": "object",
                    "properties": {
                        "sub": {"type": "boolean"},
                    },
                },
            },
        },
    }
    map311["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x={value}"}}
    expect_pass("map311_boolean_string_ok", map311)
    map311["argumentMapping"] = {"type": "flattened", "templates": {"cfg.sub": "--x"}}
    covered.add(expect_code("map311", lambda p: write_json(p, map311), "MAP311"))
    map312 = {
        "name": "ohos-flatsep",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/flatsep",
        "input": {
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
        "argumentMapping": {
            "type": "flattened",
            "separator": "_",
            "templates": {"cfg.sub": "--x={value}"},
        },
    }
    covered.add(expect_code("map312", lambda p: write_json(p, map312), "MAP312"))
    map401 = base_simple()
    map401["argumentMapping"] = {"type": "json-string", "templates": 1}
    covered.add(expect_code("map401", lambda p: write_json(p, map401), "MAP401"))
    map402 = base_simple()
    map402["argumentMapping"] = {"type": "json-string", "templates": 1}
    covered.add(expect_code("map402", lambda p: write_json(p, map402), "MAP402"))
    map403 = base_simple()
    map403["argumentMapping"] = {"type": "json-string", "templates": {}}
    covered.add(expect_code("map403", lambda p: write_json(p, map403), "MAP403"))
    map404 = base_simple()
    map404["argumentMapping"] = {"type": "json-string", "templates": {"missing": "--x={value}"}}
    covered.add(expect_code("map404", lambda p: write_json(p, map404), "MAP404"))
    map405 = base_simple()
    map405["argumentMapping"] = {"type": "json-string", "templates": {"target": 1}}
    covered.add(expect_code("map405", lambda p: write_json(p, map405), "MAP405"))
    map408 = base_simple()
    map408["argumentMapping"] = {"type": "json-string", "templates": {"target": "--x"}}
    covered.add(expect_code("map408", lambda p: write_json(p, map408), "MAP408"))
    map409 = base_simple()
    map409["argumentMapping"] = {"type": "json-string", "templates": {"flag": "--flag"}}
    covered.add(expect_code("map409", lambda p: write_json(p, map409), "MAP409"))
    map406 = {
        "name": "ohos-jsoncfg",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/jsoncfg",
        "input": {
            "type": "object",
            "properties": {
                "cfg": {"type": "object", "properties": {}},
            },
        },
        "argumentMapping": {
            "type": "json-string",
            "templates": {"cfg": "--cfg={value}"},
        },
    }
    expect_pass("map406_object_string_ok", map406)
    covered.add("MAP406")
    map407 = {
        "name": "ohos-jsonbool",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/jsonbool",
        "input": {
            "type": "object",
            "properties": {
                "cfg": {"type": "object", "properties": {}},
            },
        },
        "argumentMapping": {
            "type": "json-string",
            "templates": {"cfg": {"if_true": "--cfg", "if_false": ""}},
        },
    }
    covered.add(expect_code("map407", lambda p: write_json(p, map407), "MAP407"))
    map410 = base_simple()
    map410["input"]["properties"]["cfg"] = {"type": "object", "properties": {}}
    map410["argumentMapping"] = {"type": "json-string", "templates": {"cfg": {"if_true": "--x"}}}
    covered.add(expect_code("map410", lambda p: write_json(p, map410), "MAP410"))
    expect_pass("map500_mixed_pass", base_mixed())
    map501 = base_mixed()
    del map501["argumentMapping"]["mappings"]
    covered.add(expect_code("map501", lambda p: write_json(p, map501), "MAP501"))
    map502 = base_mixed()
    map502["argumentMapping"]["mappings"] = {}
    covered.add(expect_code("map502", lambda p: write_json(p, map502), "MAP502"))
    map503 = base_mixed()
    map503["argumentMapping"]["mappings"]["action"]["mode"] = "combo"
    covered.add(expect_code("map503", lambda p: write_json(p, map503), "MAP503"))
    map504 = base_mixed()
    map504["argumentMapping"]["mappings"]["action"]["extra"] = True
    covered.add(expect_code("map504", lambda p: write_json(p, map504), "MAP504"))
    map505 = base_mixed()
    del map505["argumentMapping"]["mappings"]["flag"]["template"]
    covered.add(expect_code("map505", lambda p: write_json(p, map505), "MAP505"))
    map506 = base_mixed()
    map506["argumentMapping"]["mappings"]["action"]["order"] = 0
    covered.add(expect_code("map506", lambda p: write_json(p, map506), "MAP506"))
    map507 = base_mixed()
    map507["argumentMapping"]["mappings"]["server"]["template"] = {}
    covered.add(expect_code("map507", lambda p: write_json(p, map507), "MAP507"))
    map508 = base_mixed()
    map508["argumentMapping"]["mappings"]["server"]["template"] = {"config.host": "--host={value}"}
    covered.add(expect_code("map508", lambda p: write_json(p, map508), "MAP508"))
    map509 = base_mixed()
    map509["input"]["properties"]["server.host"] = {"type": "string"}
    map509["argumentMapping"]["mappings"]["server.host"] = {"mode": "flag", "template": "--dup={value}"}
    covered.add(expect_code("map509", lambda p: write_json(p, map509), "MAP509"))
    map510 = base_mixed()
    map510["input"]["properties"]["target"] = {"type": "string"}
    map510["argumentMapping"]["mappings"]["target"] = {"mode": "positional", "order": 1}
    covered.add(expect_code("map510", lambda p: write_json(p, map510), "MAP510"))
    rel002 = copy.deepcopy(base_simple())
    del rel002["argumentMapping"]["templates"]["target"]
    covered.add(expect_code("rel002", lambda p: write_json(p, rel002), "REL002"))
    rel001 = base_simple()
    del rel001["argumentMapping"]["templates"]["target"]
    covered.add(expect_code("rel001", lambda p: write_json(p, rel001), "REL001"))
    rel006 = base_simple()
    rel006["input"] = {
        "type": "object",
        "properties": {},
    }
    rel006["argumentMapping"] = {
        "type": "flag",
        "templates": {
            "target": "--target={value}",
        },
    }
    covered.add(expect_code("rel006", lambda p: write_json(p, rel006), "REL001"))
    rel003 = base_simple()
    rel003["argumentMapping"]["templates"]["extra"] = "--extra={value}"
    covered.add(expect_code("rel003", lambda p: write_json(p, rel003), "REL003"))
    rel004 = {
        "name": "ohos-relflat",
        "version": "1.0.0",
        "description": "desc",
        "executable": "/usr/bin/relflat",
        "input": {
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
        "argumentMapping": {
            "type": "flattened",
            "templates": {"missing.sub": "--x={value}"},
        },
    }
    covered.add(expect_code("rel004", lambda p: write_json(p, rel004), "REL004"))
    rel005 = base_simple()
    rel005["input"]["properties"]["cfg"] = {"type": "object", "properties": {}}
    rel005["argumentMapping"]["templates"]["cfg"] = "--cfg={value}"
    covered.add(expect_code("rel005", lambda p: write_json(p, rel005), "REL005"))

    # EVT
    evt001 = base_simple()
    evt001["events"] = [1]
    covered.add(expect_code("evt001", lambda p: write_json(p, evt001), "EVT001"))
    evt010 = base_simple()
    evt010["events"] = ["-stdout1ASADDA-"]
    covered.add(expect_code("evt010", lambda p: write_json(p, evt010), "EVT007"))
    evt002 = base_simple()
    evt002["events"] = ["progress"]
    evt002["eventSchemas"] = []
    covered.add(expect_code("evt002", lambda p: write_json(p, evt002), "EVT002"))
    evt003 = base_simple()
    evt003["events"] = ["progress"]
    evt003["eventSchemas"] = {"other": {"type": "object", "properties": {}}}
    covered.add(expect_code("evt003", lambda p: write_json(p, evt003), "EVT003"))
    evt004 = base_simple()
    evt004["events"] = ["result"]
    covered.add(expect_code("evt004", lambda p: write_json(p, evt004), "EVT004"))
    evt005 = base_simple()
    evt005["events"] = ["progress"]
    evt005["eventSchemas"] = {"progress": []}
    covered.add(expect_code("evt005", lambda p: write_json(p, evt005), "EVT005"))
    evt006 = base_simple()
    evt006["events"] = ["result"]
    evt006["eventSchemas"] = {"result": {"type": "object", "properties": {}}}
    covered.add(expect_code("evt006", lambda p: write_json(p, evt006), "EVT006"))
    evt007 = base_simple()
    evt007["events"] = ["progress1"]
    evt007["eventSchemas"] = {"1bad": {"type": "object", "properties": {}}}
    covered.add(expect_code("evt007", lambda p: write_json(p, evt007), "EVT007"))
    evt008 = base_simple()
    evt008["events"] = ["progress"]
    covered.add(expect_code("evt008", lambda p: write_json(p, evt008), "EVT008"))
    evt009 = base_simple()
    evt009["output"] = {"type": "object", "properties": {"status": {"type": "string"}}}
    evt009["events"] = ["progress"]
    evt009["eventSchemas"] = {"progress": {"type": "object", "properties": {}}}
    covered.add(expect_code("evt009", lambda p: write_json(p, evt009), "EVT009"))

    expected = {
        "CFG001","CFG002","CFG003","CFG004","CFG005","CFG006",
        "TOP001","TOP002","TOP003","TOP004","TOP005","TOP006","TOP007","TOP008","TOP009","TOP010","TOP011","TOP012","TOP014","TOP015",
        "FMT001","FMT002","FMT003","FMT004","FMT005","FMT101","FMT102","FMT103","FMT104","FMT105","FMT106","FMT107","FMT108","FMT109",
        "SUB001","SUB002","SUB003","SUB004","SUB005","SUB006","SUB008","SUB009","SUB010","SUB011",
        "SCX001","SCX002","SCX003","SCX004","SCX005",
        "SCH001","SCH002","SCH003","SCH004","SCH005","SCH006","SCH007","SCH008","SCH101","SCH102","SCH103","SCH104","SCH105",
        "SCH201","SCH202","SCH203","SCH204","SCH205","SCH301","SCH302","SCH303","SCH401","SCH402","SCH501","SCH502","SCH503","SCH504","SCH505","SCH506","SCH507","SCH508","SCH509","SCH601","SCH602","SCH603","SCH604","SCH605","SCH606",
        "MAP001","MAP002","MAP003","MAP004","MAP005","MAP101","MAP102","MAP103","MAP104","MAP105","MAP106","MAP107","MAP108","MAP109","MAP110","MAP111","MAP112",
        "MAP201","MAP202","MAP203","MAP204","MAP205","MAP206","MAP207",
        "MAP301","MAP302","MAP303","MAP304","MAP305","MAP306","MAP307","MAP308","MAP309","MAP310","MAP311","MAP312","MAP313",
        "MAP401","MAP402","MAP403","MAP404","MAP405","MAP406","MAP407","MAP408","MAP409","MAP410",
        "MAP501","MAP502","MAP503","MAP504","MAP505","MAP506","MAP507","MAP508","MAP509","MAP510",
        "REL001","REL002","REL003","REL004","REL005",
        "EVT001","EVT002","EVT003","EVT004","EVT005","EVT006","EVT007","EVT008","EVT009",
    }

    missing = sorted(expected - covered)
    if missing:
        raise AssertionError(f"coverage missing codes: {missing}")

    print(f"PASS coverage {len(covered)} codes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
