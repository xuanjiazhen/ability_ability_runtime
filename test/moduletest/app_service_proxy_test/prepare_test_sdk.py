#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0.
"""Create an isolated SDK view with this source tree's permission definitions."""
import argparse
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--sdk', required=True)
parser.add_argument('--permissions', required=True)
parser.add_argument('--output', required=True)
parser.add_argument('--resources', required=True)
args = parser.parse_args()
source = Path(args.sdk).resolve()
target = Path(args.output).resolve()
relative = Path('26.0.1/toolchains/lib/PermissionDefinitions.json')
if not (source / relative).is_file():
    raise RuntimeError('The test HAPs require OpenHarmony SDK 26.0.1')
# Materialize only ancestors of the two permission catalogs, keeping the rest read-only.
resource = Path('26.0.1/previewer/common/resources/module.json')
def link_tree(src, dst, paths):
    if dst.is_symlink():
        dst.unlink()
    dst.mkdir(parents=True, exist_ok=True)
    for entry in src.iterdir():
        dest = dst / entry.name
        selected = [p for p in paths if p.parts[0] == entry.name]
        if selected:
            nested = [Path(*p.parts[1:]) for p in selected if len(p.parts) > 1]
            if nested:
                link_tree(entry, dest, nested)
        elif not dest.exists() and not dest.is_symlink():
            dest.symlink_to(entry, target_is_directory=entry.is_dir())
link_tree(source, target, [relative, resource])
(target / resource).write_bytes(Path(args.resources).read_bytes())
data = json.loads(Path(args.permissions).read_text())
data['definePermissions'] = [p for p in data['definePermissions'] if p['availableType'] != 'SERVICE']
(target / relative).write_text(json.dumps(data, indent=2) + '\n')
(target / 'ready.stamp').write_text('Source permission catalog installed in isolated test SDK\n')
