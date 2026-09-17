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
import { abilityDelegatorRegistry } from '@kit.TestKit';

const TAG = 'OhosAaConfigTest';

export default function ohosAaConfigTest() {
  describe('OhosAaConfigTest', () => {
    it('ohos_aa_start_config_visibility_public', 0, async () => {
      const configPath = '/system/bin/cli_tool/configs/ohos-aa.json';
      try {
        const file = await globalThis.fs.readText(configPath);
        const config = JSON.parse(file);
        const startSub = config.subcommands?.start;
        expect(startSub).assertNotNull();
        expect(startSub.visibility).assertEqual('public');
      } catch (e) {
        console.error(`${TAG}: ${e.message}`);
        expect(false).assertTrue();
      }
    });

    it('ohos_aa_start_config_supportedDevices', 0, async () => {
      const configPath = '/system/bin/cli_tool/configs/ohos-aa.json';
      try {
        const file = await globalThis.fs.readText(configPath);
        const config = JSON.parse(file);
        const startSub = config.subcommands?.start;
        expect(startSub).assertNotNull();
        expect(Array.isArray(startSub.supportedDevices)).assertTrue();
        expect(startSub.supportedDevices.includes('default')).assertTrue();
      } catch (e) {
        console.error(`${TAG}: ${e.message}`);
        expect(false).assertTrue();
      }
    });

    it('ohos_aa_force_stop_config_no_visibility', 0, async () => {
      const configPath = '/system/bin/cli_tool/configs/ohos-aa.json';
      try {
        const file = await globalThis.fs.readText(configPath);
        const config = JSON.parse(file);
        const forceStopSub = config.subcommands?.['force-stop'];
        expect(forceStopSub).assertNotNull();
        expect(forceStopSub.visibility === undefined).assertTrue();
      } catch (e) {
        console.error(`${TAG}: ${e.message}`);
        expect(false).assertTrue();
      }
    });
  });
}
