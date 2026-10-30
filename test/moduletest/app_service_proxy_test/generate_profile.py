#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0.
"""Generate an OpenHarmony engineering-test profile using the repository demo key."""
import argparse
import json
import re
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--root', required=True)
parser.add_argument('--output', required=True)
parser.add_argument('--package', required=True)
parser.add_argument('--identifier', required=True)
parser.add_argument('--signer', required=True)
parser.add_argument('--permission', action='append', default=[])
args = parser.parse_args()
dist = Path(args.root).resolve() / 'developtools/hapsigner/dist'
out = Path(args.output).resolve()
out.parent.mkdir(parents=True, exist_ok=True)
profile = json.loads((dist / 'UnsgnedReleasedProfileTemplate.json').read_text())
certs = re.findall(r'-----BEGIN CERTIFICATE-----.*?-----END CERTIFICATE-----',
                  (dist / 'OpenHarmonyApplication.pem').read_text(), re.S)
profile['uuid'] = args.identifier
profile['validity'] = {'not-before': 1735689600, 'not-after': 2493072000}
profile['app-distribution-type'] = 'enterprise_mdm'
profile['bundle-info'].update({'bundle-name': args.package, 'app-identifier': args.identifier,
    'apl': 'system_basic', 'app-feature': 'hos_normal_app', 'distribution-certificate': certs[-1] + '\n'})
profile['acls'] = {'allowed-acls': args.permission}
profile['permissions'] = {'restricted-permissions': []}
source = out.with_suffix('.json')
source.write_text(json.dumps(profile, indent=2) + '\n')
# Public repository test password, also used by the standard OHOS app signing build.
command = ['java', '-jar', args.signer, 'sign-profile', '-mode', 'localSign',
    '-keyAlias', 'openharmony application profile release', '-signAlg', 'SHA256withECDSA',
    '-profileCertFile', str(dist / 'OpenHarmonyProfileRelease.pem'),
    '-keystoreFile', str(dist / 'OpenHarmony.p12'), '-keystorePwd', '123456', '-keyPwd', '123456',
    '-inFile', str(source), '-outFile', str(out)]
subprocess.run(command, check=True)
