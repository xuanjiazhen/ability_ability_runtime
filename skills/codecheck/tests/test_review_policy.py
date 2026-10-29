import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import review_policy as policy
import export_text


def review(severity="P2"):
    return {
        "scope_key": "https://example.invalid/repo#PR-1",
        "dimensions_required": ["logic-scanner"],
        "dimensions_executed": ["logic-scanner"],
        "findings": [{"id": "LOG-01", "severity": severity,
                      "evidence_status": "Confirmed", "root_cause_key": "publish-before-write",
                      "sites": [{"path": "store.cpp", "symbol": "Store::Save",
                                 "operation": "publish-ready", "line": 10}]}],
    }


def gate(data, state=None):
    findings = policy.active_findings(data, state or policy.empty_state())
    return policy.gate_fields(findings, data["dimensions_required"], data["dimensions_executed"])


def deliver(data, state, round_number):
    return policy.update_history(data, state, "record-delivery", delivery_id=f"PR1-R{round_number}",
                                 receipt=f"chat:delivered-round-{round_number}")


class GateTests(unittest.TestCase):
    def test_p0_blocks_even_with_missing_dimension_and_p1_remains_must_fix(self):
        data = review("P0")
        other = copy.deepcopy(data["findings"][0])
        other.update(id="SEC-01", severity="P1", root_cause_key="independent-cause")
        data["findings"].append(other)
        data["dimensions_required"].append("security-scanner")
        result = gate(data)
        self.assertEqual((result["gate_decision"], result["risk_level"], result["score"]),
                         ("block", "high", 58))
        self.assertEqual(result["gate_blockers"], ["LOG-01"])
        self.assertEqual(result["must_fix"], ["SEC-01"])

    def test_missing_dimension_precedes_p1_and_keeps_its_id(self):
        data = review("P1")
        data["dimensions_executed"] = []
        result = gate(data)
        self.assertEqual(result["gate_decision"], "insufficient")
        self.assertEqual(result["must_fix"], ["LOG-01"])

    def test_p1_is_conditional(self):
        self.assertEqual(gate(review("P1"))["gate_decision"], "conditional")

    def test_many_p2_and_p3_do_not_block(self):
        data = review()
        data["findings"] = []
        for index in range(40):
            finding = review("P2" if index % 2 else "P3")["findings"][0]
            finding.update(id=f"LOG-{index}", root_cause_key=f"cause-{index}")
            data["findings"].append(finding)
        result = gate(data)
        self.assertEqual((result["score"], result["gate_decision"]), (0, "approve"))
        self.assertEqual(len(result["followups"]), 40)

    def test_zero_findings_and_no_automatic_tier_or_hotspot_upgrade(self):
        data = review("P2")
        data["findings"][0].update(tier="T1", crown_jewel=True)
        self.assertEqual(gate(data)["gate_decision"], "approve")
        data["findings"] = []
        result = gate(data)
        self.assertEqual((result["findings_total"], result["score"]), (0, 100))
        self.assertEqual(result["gate_decision"], "approve")

    def test_unproved_candidates_never_turn_into_followups(self):
        data = review("P0")
        for status in ("Likely", "Suspicious", "Excluded"):
            data["findings"][0]["evidence_status"] = status
            self.assertEqual(gate(data)["findings_total"], 0)

    def test_same_cause_merges_all_sites_but_same_line_is_not_a_cause(self):
        data = review()
        extra = copy.deepcopy(data["findings"][0])
        extra["id"] = "SEC-01"
        extra["sites"][0]["symbol"] = "Store::SaveOther"
        data["findings"].append(extra)
        merged = policy.active_findings(data, policy.empty_state())
        self.assertEqual(len(merged), 1)
        self.assertEqual(len(merged[0]["sites"]), 2)
        self.assertEqual(gate(data)["score"], 95)
        extra["root_cause_key"] = "independent-cause"
        self.assertEqual(gate(data)["findings_total"], 2)

    def test_severity_disagreement_requires_evidence_resolution(self):
        data = review()
        extra = copy.deepcopy(data["findings"][0])
        extra.update(id="SEC-01", severity="P0")
        data["findings"].append(extra)
        with self.assertRaises(ValueError):
            gate(data)


class HistoryTests(unittest.TestCase):
    def test_nondefect_advice_shares_history_without_affecting_score(self):
        data = review()
        item = data["findings"][0]
        item["kind"] = "advice"
        del item["severity"]
        state = policy.empty_state()
        self.assertEqual(len(policy.active_findings(data, state)), 1)
        self.assertEqual((gate(data, state)["findings_total"], gate(data, state)["score"]), (0, 100))
        state = deliver(data, deliver(data, state, 1), 2)
        self.assertEqual(policy.active_findings(data, state), [])

    def test_two_deliveries_then_suppress_all_severities(self):
        for severity in policy.LEVELS:
            with self.subTest(severity=severity):
                data = review(severity)
                state = policy.empty_state()
                self.assertEqual(gate(data, state)["findings_total"], 1)
                state = deliver(data, state, 1)
                self.assertEqual(gate(data, state)["findings_total"], 1)
                state = deliver(data, state, 2)
                self.assertEqual(gate(data, state)["findings_total"], 0)
                self.assertEqual(gate(data, state)["gate_decision"], "approve")

    def test_draft_does_not_record_and_exports_of_one_round_count_once(self):
        data, state = review(), policy.empty_state()
        before = copy.deepcopy(state)
        for _ in range(3):
            gate(data, state)
        self.assertEqual(state, before)
        state = deliver(data, state, 1)
        self.assertEqual(state, deliver(data, state, 1))
        self.assertEqual(gate(data, state)["findings_total"], 1)

    def test_line_title_sha_and_severity_do_not_reopen(self):
        data = review()
        state = deliver(data, deliver(data, policy.empty_state(), 1), 2)
        data["findings"][0].update(id="SEC-99", title="renamed", severity="P0", commit_id="new-sha")
        data["findings"][0]["sites"][0]["line"] = 999
        self.assertEqual(gate(data, state)["findings_total"], 0)

    def test_explicit_accept_is_immediate_and_new_site_does_not_reopen_old(self):
        data = review("P0")
        state = policy.update_history(data, policy.empty_state(), "accept", reason="User accepts this site")
        self.assertEqual(gate(data, state)["findings_total"], 0)
        new_site = dict(data["findings"][0]["sites"][0], symbol="Store::SaveNew")
        data["findings"][0]["sites"].append(new_site)
        active = policy.active_findings(data, state)
        self.assertEqual(active[0]["sites"], [new_site])

    def test_explicit_reopen_requires_reason_and_preserves_audit(self):
        data = review()
        state = deliver(data, deliver(data, policy.empty_state(), 1), 2)
        with self.assertRaises(ValueError):
            policy.update_history(data, state, "reopen", reason="")
        reopened = policy.update_history(data, state, "reopen", reason="New consumer proves persistent loss")
        self.assertEqual(gate(data, reopened)["findings_total"], 1)
        entry = next(iter(reopened["sites"].values()))
        self.assertEqual(len(entry["events"][0]["previous_deliveries"]), 2)
        self.assertEqual(gate(data, state)["findings_total"], 0)

    def test_scope_and_independent_cause_have_separate_history(self):
        data = review()
        state = policy.update_history(data, policy.empty_state(), "accept", reason="Accepted")
        other = copy.deepcopy(data)
        other["scope_key"] += "-other"
        self.assertEqual(gate(other, state)["findings_total"], 1)
        data["findings"][0]["root_cause_key"] = "different-cause"
        self.assertEqual(gate(data, state)["findings_total"], 1)

    def test_corrupt_state_fails_without_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path / "review.json").write_text(json.dumps(review()))
            state = path / "state.json"
            state.write_text('{"version": 1, "sites": []}')
            before = state.read_bytes()
            result = subprocess.run([sys.executable, str(ROOT / "scripts/review_policy.py"),
                                     "record-delivery", "--input", str(path / "review.json"),
                                     "--state", str(state), "--delivery-id", "R1", "--receipt", "chat:1"],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 2)
            self.assertEqual(state.read_bytes(), before)

    def test_cli_evaluate_and_record_round_trip(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            source, state = path / "review.json", path / "state.json"
            source.write_text(json.dumps(review("P1")))
            base = [sys.executable, str(ROOT / "scripts/review_policy.py")]
            options = ["--input", str(source), "--state", str(state)]
            result = subprocess.run(base + ["evaluate"] + options, capture_output=True, text=True, check=True)
            self.assertEqual(json.loads(result.stdout)["gate_fields"]["gate_decision"], "conditional")
            self.assertFalse(state.exists())
            subprocess.run(base + ["record-delivery"] + options + ["--delivery-id", "R1", "--receipt", "chat:1"],
                           capture_output=True, check=True)
            self.assertEqual(len(next(iter(json.loads(state.read_text())["sites"].values()))["deliveries"]), 1)


class ExportTests(unittest.TestCase):
    def test_csv_formula_prefixes_and_safe_text_roundtrip(self):
        import csv
        dangerous = ['=HYPERLINK("https://example.invalid")', '+1', '-1', '@SUM(1)',
                     '  =1', '\t=1', '\r\n+1', '\ufeff@SUM(1)', '\x00=1', '\tplain']
        safe = ['src/test.cpp:3', '中,文"证据', 'x = y', '普通说明\n第二行']
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "report.csv"
            export_text.write_csv(path, ["证据"], [[value] for value in dangerous + safe])
            with path.open(newline="", encoding="utf-8") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(rows[0], ["证据"])
            self.assertEqual([row[0] for row in rows[1:1+len(dangerous)]],
                             ["'" + text.replace("\x00", "\\x00") for text in dangerous])
            self.assertEqual([row[0] for row in rows[1+len(dangerous):]], safe)
            before = path.read_bytes()
            with self.assertRaises(FileExistsError):
                export_text.write_csv(path, ["证据"], [["new"]])
            self.assertEqual(path.read_bytes(), before)

    def test_xlsx_contains_text_not_formulas_and_preserves_layout(self):
        import openpyxl
        spec = importlib.util.spec_from_file_location(
            "security_export", ROOT / "scripts/generate_report.py")
        exporter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(exporter)
        finding = {"file_path": "=1+1", "line_number": "123", "summary": "+SUM(1)",
                   "description": '=HYPERLINK("https://example.invalid")',
                   "issue_type": "@SUM(1)", "risk_level": "一般"}
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "report.xlsx"
            exporter.build_workbook("module", [finding], path)
            book = openpyxl.load_workbook(path, data_only=False)
            sheet = book.active
            self.assertEqual([cell.value for cell in sheet[1]], exporter.HEADERS)
            self.assertEqual(sheet.max_column, 6)
            self.assertEqual(sheet.freeze_panes, "A2")
            self.assertEqual(sheet.column_dimensions["B"].width, 18)
            self.assertEqual(sheet.row_dimensions[2].height, 160)
            self.assertEqual(sheet["A2"].value, "=1+1")
            self.assertTrue(all(cell.data_type == "s" for cell in sheet[2]))
            self.assertEqual(sheet["F2"].fill.fgColor.rgb, "FFFFC000")
            book.close()


if __name__ == "__main__":
    unittest.main()
