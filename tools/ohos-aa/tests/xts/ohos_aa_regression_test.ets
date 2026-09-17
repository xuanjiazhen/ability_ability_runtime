/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { describe, it, expect } from 'hypium';
import cliManager from '@ohos.app.cli.cliManager';

const TAG = 'OhosAaRegressionTest';

export default function ohosAaRegressionTest() {
  describe('OhosAaRegressionTest', () => {
    it('ohos_aa_start_unchanged', 0, async () => {
      try {
        const tools: Array<cliManager.ToolInfo> = await cliManager.queryTools();
        const ohosAa = tools.find((t) => t.name === 'ohos-aa');
        expect(ohosAa).assertNotNull();
        expect(ohosAa.version).assertEqual('1.0.0');
        expect(ohosAa.executablePath).assertEqual('/system/bin/cli_tool/executable/ohos-aa');
        expect(ohosAa.hasSubCommand).assertTrue();
        const startSub = ohosAa.subcommands?.['start'];
        expect(startSub).assertNotNull();
        expect(startSub.description).assertContain('Start an ability');
        expect(startSub.requirePermissions).assertContain('ohos.permission.cli.START_ABILITY');
      } catch (e) {
        console.error(`${TAG}: ${e.message}`);
        expect(false).assertTrue();
      }
    });

    it('ohos_aa_force_stop_unchanged_and_private', 0, async () => {
      try {
        const tools: Array<cliManager.ToolInfo> = await cliManager.queryTools();
        const ohosAa = tools.find((t) => t.name === 'ohos-aa');
        expect(ohosAa).assertNotNull();
        const forceStopSub = ohosAa.subcommands?.['force-stop'];
        if (forceStopSub !== undefined) {
          expect(forceStopSub.description).assertContain('Stop an application');
          expect(forceStopSub.requirePermissions).assertContain(
            'ohos.permission.cli.KILL_APP_PROCESSES'
          );
          expect(forceStopSub.visibility === undefined ||
                 forceStopSub.visibility === 'private').assertTrue();
        }
      } catch (e) {
        console.error(`${TAG}: ${e.message}`);
        expect(false).assertTrue();
      }
    });
  });
}
