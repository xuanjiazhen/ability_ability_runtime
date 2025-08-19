/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import image from '@ohos.multimedia.image';

export type OnMissionCreatedFn = (mission: int) => void;
export type OnMissionDestroyedFn = (mission: int) => void;
export type OnMissionSnapshotChangedFn = (mission: int) => void;
export type OnMissionMovedToFrontFn = (mission: int) => void;
export type OnMissionLabelUpdatedFn = (mission: int) => void;
export type OnMissionIconUpdatedFn = (mission: int, icon: image.PixelMap) => void;
export type OnMissionClosedFn = (mission: int) => void;

export interface MissionListener {
  onMissionCreated: OnMissionCreatedFn;
  onMissionDestroyed: OnMissionDestroyedFn;
  onMissionSnapshotChanged: OnMissionSnapshotChangedFn;
  onMissionMovedToFront: OnMissionMovedToFrontFn;
  onMissionLabelUpdated: OnMissionLabelUpdatedFn;
  onMissionIconUpdated: OnMissionIconUpdatedFn;
  onMissionClosed: OnMissionClosedFn;
}

class MissionListenerInner implements MissionListener {
  onMissionCreated: OnMissionCreatedFn = (mission: int) => {
  }
  onMissionDestroyed: OnMissionDestroyedFn = (mission: int) => {
  }
  onMissionSnapshotChanged: OnMissionSnapshotChangedFn = (mission: int) => {
  }
  onMissionMovedToFront: OnMissionMovedToFrontFn = (mission: int) => {
  }
  onMissionLabelUpdated: OnMissionLabelUpdatedFn = (mission: int) => {
  }
  onMissionIconUpdated: OnMissionIconUpdatedFn = (mission: int, icon: image.PixelMap) => {
  }
  onMissionClosed: OnMissionClosedFn = (mission: int) => {
  }
}

/*
export interface MissionListener {         //方法转属性
  onMissionCreated(mission: int): void;
  onMissionDestroyed(mission: int): void;
  onMissionSnapshotChanged(mission: int): void;
  onMissionMovedToFront(mission: int): void;
  onMissionLabelUpdated(mission: int): void;
  onMissionIconUpdated(mission: int, icon: image.PixelMap): void;
  onMissionClosed(mission: int): void;
}
*/