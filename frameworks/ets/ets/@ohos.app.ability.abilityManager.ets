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

import { ElementName } from 'bundleManager.ElementName'
import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace abilityManager {
  loadLibrary("ability_manager_ani_kit.z")

  export native function nativeGetTopAbility(callback:AsyncCallbackWrapper<ElementName>) : void;

  export function getTopAbility():Promise<ElementName> {
    let p = new Promise<ElementName>((resolve: (data: ElementName)=>void, reject:(err: BusinessError<void>)=>void):void => {
      let myCall = new AsyncCallbackWrapper<ElementName>((err: BusinessError<void>, data: ElementName)=>{
        if (err.code == 0) {
          resolve(data);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        abilityManager.nativeGetTopAbility(myCall);
      });
    });
    return p;
}

export function getTopAbility(callback: AsyncCallback<ElementName, void>): void {
    let myCall = new AsyncCallbackWrapper<ElementName>(callback);
    taskpool.execute( (): void => {
        abilityManager.nativeGetTopAbility(myCall);
    });
}
}