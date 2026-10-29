#!/usr/bin/env python3
"""Write CSV text without spreadsheet formula execution; preserve column order.

Usage: export_text.py --input table.json --output report.csv
Input: {"headers": [existing column names], "rows": [[cell values], ...]}
All fields are quoted, UTF-8. Refuses to overwrite an existing artifact.
Use only after evidence/history filtering and the output format's text cleanup.
"""

import argparse
import csv
import json
from pathlib import Path
import unicodedata


def csv_text(value):
    text = "" if value is None else str(value)
    # Spreadsheet importers differ in how they skip whitespace/control/format
    # prefixes. Keep the original text, but protect even concealed formulas.
    index = 0
    while index < len(text) and (text[index].isspace() or
                               unicodedata.category(text[index]) in ("Cc", "Cf")):
        index += 1
    protect = index < len(text) and text[index] in "=+-@"
    # Leading tab/CR/LF can themselves invoke spreadsheet-specific parsing.
    protect = protect or text.startswith(("\t", "\r", "\n"))
    # NUL is not portable CSV (Python 3.10 readers reject it); show otherwise
    # invisible control bytes explicitly, without changing the column layout.
    visible = "".join(char.encode("unicode_escape").decode("ascii")
                      if unicodedata.category(char) == "Cc" and char not in "\t\r\n"
                      else char for char in text)
    return ("'" if protect else "") + visible


def write_csv(path, headers, rows):
    if not headers or any(len(row) != len(headers) for row in rows):
        raise ValueError("CSV rows must match the existing header width")
    with Path(path).open("x", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, quoting=csv.QUOTE_ALL)
        writer.writerow([csv_text(cell) for cell in headers])
        writer.writerows([[csv_text(cell) for cell in row] for row in rows])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        data = json.loads(args.input.read_text(encoding="utf-8"))
        write_csv(args.output, data["headers"], data["rows"])
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(2, f"codecheck: {error}\n")


if __name__ == "__main__":
    main()
