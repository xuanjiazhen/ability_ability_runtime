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
import { Want } from '@kit.AbilityKit';
import AppServiceExtensionAbility from '@ohos.app.ability.AppServiceExtensionAbility';
import { rpc } from '@kit.IPCKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { BusinessError } from '@kit.BasicServicesKit';
import process from '@ohos.process';

class TestRemote extends rpc.RemoteObject {
  private owner: Provider;
  constructor(owner: Provider) {
    super('ohos.appservice.proxy.test');
    this.owner = owner;
  }
  onRemoteMessageRequest(code: number, data: rpc.MessageSequence,
    reply: rpc.MessageSequence, options: rpc.MessageOption): boolean {
    if (data.readInterfaceToken() !== 'ohos.appservice.proxy.test') { return false; }
    if (code === 1) {
      reply.writeString(data.readString());
      reply.writeInt(process.uid);
      return true;
    }
    if (code === 2) {
      this.owner.startPage();
      reply.writeInt(0);
      return true;
    }
    if (code === 3) { reply.writeInt(this.owner.startResult); return true; }
    return false;
  }
}
export default class Provider extends AppServiceExtensionAbility {
  private remote: TestRemote = new TestRemote(this);
  startResult: number = -1;
  onCreate(want: Want): void { hilog.info(0x0001, 'AppServiceTest', 'onCreate uid=%{public}d', process.uid); }
  onConnect(want: Want): rpc.RemoteObject {
    hilog.info(0x0001, 'AppServiceTest', 'onConnect uid=%{public}d', process.uid);
    return this.remote;
  }
  startPage(): void {
    this.startResult = -1;
    this.context.startAbility({ bundleName: 'com.ohos.appservice.proxy.caller',
      abilityName: 'EntryAbility' }).then(() => { this.startResult = 0; })
      .catch((error: BusinessError) => { this.startResult = error.code; });
  }
}
