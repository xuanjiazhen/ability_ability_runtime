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

import LaunchServiceDescriptorInner, { LaunchServiceDescriptor } from './LaunchServiceDescriptor';

export interface LaunchServiceStateData {
  descriptor: LaunchServiceDescriptor;
  state: int;
}

export default class LaunchServiceStateDataInner implements LaunchServiceStateData {
  descriptor: LaunchServiceDescriptor = new LaunchServiceDescriptorInner();
  state: int = 0;
}

export interface LaunchServiceProcessStateData {
  descriptor: LaunchServiceDescriptor;
  pid: int;
  state: int;
}

export class LaunchServiceProcessStateDataInner implements LaunchServiceProcessStateData {
  descriptor: LaunchServiceDescriptor = new LaunchServiceDescriptorInner();
  pid: int = 0;
  state: int = 0;
}

export interface LaunchServiceObserver {
  onLaunchServiceStatusChanged(stateData: LaunchServiceStateData): void;
  onLaunchServiceProcessStateChanged(processStateData: LaunchServiceProcessStateData): void;
}
