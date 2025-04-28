/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import type appManager from '@ohos.app.ability.appManager';
import bundleManager from '@ohos.bundle.bundleManager';

export interface ProcessInformation {

  pid: int;

  uid: int;

  processName: string;

  bundleNames: Array<string>;

  state: appManager.ProcessState;

  bundleType: bundleManager.BundleType;

  appCloneIndex?: int;
}

class ProcessInformationInner implements ProcessInformation {
  pid: int = -1;

  uid: int = -1;

  processName: string = "";

  bundleNames: Array<string> = new Array<string>();

  state: appManager.ProcessState = appManager.ProcessState.STATE_DESTROY;

  bundleType: bundleManager.BundleType = bundleManager.BundleType.APP;

  appCloneIndex?: int = -1;
}