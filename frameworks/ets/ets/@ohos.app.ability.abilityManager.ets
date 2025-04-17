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

// import { ElementName } from 'bundleManager.ElementName';
import { ElementName } from './bundleManager/ElementName'; // 先这样写验证，等待包管理合入后改正写法

export class AsyncCallbackWrapper<T> {
  myFun_:AsyncCallback<T, void> = (err: BusinessError<void>, data: T) => {}

  constructor(myFun:AsyncCallback<T, void>){
    this.myFun_ = myFun;
  }

  invoke(err: BusinessError<void>, data: T) : void {
    this.myFun_(err, data);
  }
}

export default namespace abilityManager {

    export native function nativeGetTopAbility() : ElementName;

    export function getTopAbility():Promise<ElementName> {
        let p = new Promise<ElementName>((resolve: (data: ElementName) => void, reject: (error: Error) => void): void => {
            let executeFunc = (): ElementName => {
                return nativeGetTopAbility();
            };
            let p1 = taskpool.excecute(executeFunc);
            p1.then((e: NullishType)=>{
                let myElementName : ElementName = e as ElementName;
                resolve(myElementName);
            }, (err: Error): void => {
                reject(err);
            });
        });
        return p;
    }

    export function getTopAbility(callback: AsyncCallback<ElementName, void>): void {
        let myCall = new AsyncCallbackWrapper<ElementName>(callback);
        taskpool.execute( (): void => {
            this.nativeGetTopAbility(myCall);
        });
    }
}