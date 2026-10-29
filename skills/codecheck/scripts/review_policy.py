#!/usr/bin/env python3
"""Internal codecheck policy; does not change the public schema 1.0.

Input JSON (stable semantic keys are assigned after reading the code/history):
  {"scope_key": "repo-url#PR-123", "dimensions_required": ["logic-scanner"],
   "dimensions_executed": ["logic-scanner"], "findings": [
     {"id": "LOG-01", "severity": "P2", "evidence_status": "Confirmed",
      "root_cause_key": "failed-write-publishes-ready",
      "sites": [{"path": "src/a.cpp", "symbol": "Store::Save",
                 "operation": "publish-ready", "line": 42,
                 "evidence": "...", "impact": "..."}]}]}

Paths/symbols/operations identify sites; lines, titles, severity and SHA do not.
Reuse existing semantic keys (also across proven renames). Do not put all site
evidence into a global finding description: render only returned sites.

  evaluate --input review.json --state /local/review-state.json
    Prints internal filtered findings and schema-1.0 gate fields, no state write.
  record-delivery --input review.json --state ... --delivery-id PR123-R2
                  --receipt <actual-delivery-reference>
    Only AFTER the final findings were actually delivered. A draft file is not
    a receipt. Re-exports of the same review use the same delivery-id.
  accept --input selected-sites.json --state ... --reason <user-decision>
  reopen --input selected-sites.json --state ... --reason <new-evidence-or-request>
    Explicit site-level events; never reopen merely because SHA/lines changed.

Use evaluate before all exports; after partial site suppression regenerate prose
from remaining evidence. Record the exact delivered final input, not an earlier
candidate set. State and this JSON are INTERNAL, not public report metadata.
For non-defect test/format advice, set kind="advice" and omit severity. Advice
shares site history but never contributes to counts, scores or gate fields.
Missing state starts empty; corrupt state fails without overwriting it.
"""

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile


LEVELS = ("P0", "P1", "P2", "P3")
WEIGHTS = dict(zip(LEVELS, (30, 12, 5, 2)))


def nonempty(value):
    if not isinstance(value, str) or not value.strip():
        raise ValueError("Expected a non-empty string")
    return value


def site_identity(scope, finding, site):
    return [nonempty(scope), nonempty(finding["root_cause_key"]),
            *[nonempty(site[k]) for k in ("path", "symbol", "operation")]]


def site_key(scope, finding, site):
    raw = json.dumps(site_identity(scope, finding, site), ensure_ascii=False)
    return hashlib.sha256(raw.encode()).hexdigest()


def empty_state():
    return {"version": 1, "sites": {}}


def validate_state(state):
    if not isinstance(state, dict) or state.get("version") != 1:
        raise ValueError("Unsupported or corrupt history; recover delivery records")
    if not isinstance(state.get("sites"), dict):
        raise ValueError("History sites must be an object")
    for key, entry in state["sites"].items():
        identity = entry["identity"]
        if not isinstance(identity, list) or len(identity) != 5:
            raise ValueError("Corrupt site identity")
        for value in identity:
            nonempty(value)
        expected = hashlib.sha256(json.dumps(identity, ensure_ascii=False).encode()).hexdigest()
        if key != expected or type(entry["accepted"]) is not bool:
            raise ValueError("Corrupt history entry")
        if not isinstance(entry["deliveries"], list) or not isinstance(entry["events"], list):
            raise ValueError("Corrupt history events")
        seen = set()
        for delivery in entry["deliveries"]:
            delivery_id = nonempty(delivery["id"])
            nonempty(delivery["receipt"])
            if delivery_id in seen:
                raise ValueError("Duplicate delivery in history")
            seen.add(delivery_id)
        for event in entry["events"]:
            if event["action"] not in ("accept", "reopen"):
                raise ValueError("Unknown history event")
            nonempty(event["reason"])
    return state


def confirmed_findings(review):
    """Merge already-established identical causes; never infer causes by line."""
    nonempty(review["scope_key"])
    groups, ids = {}, set()
    for original in review["findings"]:
        status = original["evidence_status"]
        if status not in ("Confirmed", "Likely", "Suspicious", "Excluded"):
            raise ValueError("Unknown evidence status")
        if status != "Confirmed":
            continue
        finding = copy.deepcopy(original)
        fid = nonempty(finding["id"])
        kind = finding.get("kind", "finding")
        if kind not in ("finding", "advice"):
            raise ValueError("Unknown item kind")
        if fid in ids or (kind == "finding" and finding.get("severity") not in LEVELS):
            raise ValueError("Duplicate ID or invalid severity")
        ids.add(fid)
        cause = nonempty(finding["root_cause_key"])
        if not isinstance(finding["sites"], list) or not finding["sites"]:
            raise ValueError("Confirmed finding needs sites")
        for site in finding["sites"]:
            site_identity(review["scope_key"], finding, site)
        if cause not in groups:
            finding["source_ids"] = [fid]
            groups[cause] = finding
        else:
            group = groups[cause]
            if group.get("kind", "finding") != kind or group.get("severity") != finding.get("severity"):
                raise ValueError("Resolve same-cause severity disagreement before scoring")
            group["sites"].extend(finding["sites"])
            group["source_ids"].append(fid)
    for finding in groups.values():
        sites = {}
        for site in finding["sites"]:
            key = site_key(review["scope_key"], finding, site)
            if key in sites and sites[key] != site:
                raise ValueError("Resolve conflicting evidence for the same site")
            sites[key] = site
        finding["sites"] = list(sites.values())
    return list(groups.values())


def active_findings(review, state):
    validate_state(state)
    result = []
    for finding in confirmed_findings(review):
        active = []
        for site in finding["sites"]:
            entry = state["sites"].get(site_key(review["scope_key"], finding, site))
            if entry is None or (not entry["accepted"] and len(entry["deliveries"]) < 2):
                active.append(site)
        if active:
            finding["sites"] = active
            result.append(finding)
    return result


def gate_fields(findings, required, executed):
    if any(f.get("kind", "finding") not in ("finding", "advice") for f in findings):
        raise ValueError("Unknown item kind")
    findings = [f for f in findings if f.get("kind", "finding") == "finding"]
    counts = {severity: 0 for severity in LEVELS}
    ids = set()
    for finding in findings:
        fid = nonempty(finding["id"])
        if fid in ids or finding["severity"] not in LEVELS:
            raise ValueError("Duplicate finding or invalid severity")
        ids.add(fid)
        counts[finding["severity"]] += 1
    for dimensions in (required, executed):
        if not isinstance(dimensions, list):
            raise ValueError("Dimensions must be lists")
        for dimension in dimensions:
            nonempty(dimension)
    if counts["P0"]:
        decision, risk = "block", "high"
    elif set(required) - set(executed):
        decision, risk = "insufficient", "unknown"
    elif counts["P1"]:
        decision, risk = "conditional", "medium"
    else:
        decision, risk = "approve", "low"
    return {
        "gate_decision": decision, "risk_level": risk,
        "score": max(0, 100 - sum(WEIGHTS[s] * counts[s] for s in LEVELS)),
        "findings_total": len(findings), "findings_by_severity": counts,
        "gate_blockers": [f["id"] for f in findings if f["severity"] == "P0"],
        "must_fix": [f["id"] for f in findings if f["severity"] == "P1"],
        "followups": [f["id"] for f in findings if f["severity"] in ("P2", "P3")],
    }


def update_history(review, state, action, *, delivery_id=None, receipt=None, reason=None):
    """Return a copy; explicit acceptance/reopening never follows from a diff."""
    validate_state(state)
    if action == "record-delivery":
        nonempty(delivery_id)
        nonempty(receipt)
        selected = active_findings(review, state)
    elif action in ("accept", "reopen"):
        nonempty(reason)
        selected = confirmed_findings(review)
    else:
        raise ValueError("Unknown history action")
    updated = copy.deepcopy(state)
    for finding in selected:
        for site in finding["sites"]:
            key = site_key(review["scope_key"], finding, site)
            entry = updated["sites"].setdefault(key, {
                "identity": site_identity(review["scope_key"], finding, site),
                "deliveries": [], "accepted": False, "events": [],
            })
            if action == "record-delivery":
                if delivery_id not in {d["id"] for d in entry["deliveries"]}:
                    entry["deliveries"].append({"id": delivery_id, "receipt": receipt})
            else:
                entry["events"].append({"action": action, "reason": reason,
                                        "previous_deliveries": copy.deepcopy(entry["deliveries"])})
                entry["accepted"] = action == "accept"
                if action == "reopen":
                    entry["deliveries"] = []
    return validate_state(updated)


def save_state(path, state):
    """Atomic replacement; caller serializes updates for this review scope."""
    validate_state(state)
    path.parent.mkdir(parents=True, exist_ok=True)
    name = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                         prefix=".codecheck-", delete=False) as stream:
            name = stream.name
            json.dump(state, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(name, path)
    finally:
        if name and os.path.exists(name):
            os.unlink(name)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", choices=("evaluate", "record-delivery", "accept", "reopen"))
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--state", type=Path, required=True)
    parser.add_argument("--delivery-id")
    parser.add_argument("--receipt")
    parser.add_argument("--reason")
    args = parser.parse_args()
    try:
        review = json.loads(args.input.read_text(encoding="utf-8"))
        state = (json.loads(args.state.read_text(encoding="utf-8"))
                 if args.state.exists() else empty_state())
        if args.action == "evaluate":
            findings = active_findings(review, state)
            print(json.dumps({"findings": findings, "gate_fields": gate_fields(
                findings, review["dimensions_required"], review["dimensions_executed"])},
                ensure_ascii=False, indent=2))
        else:
            updated = update_history(review, state, args.action, delivery_id=args.delivery_id,
                                     receipt=args.receipt, reason=args.reason)
            save_state(args.state, updated)
            print("History updated")
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(2, f"codecheck: {error}\n")


if __name__ == "__main__":
    main()
