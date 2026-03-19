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

import AsyncCallbackWrapper from './utils/AbilityUtils';
import { BusinessError } from '@ohos.base';

const ERROR_CODE_INVALID_PARAM: int = 401;

export default namespace modularObjectManager {
    loadLibrary("modular_object_manager_ani.z")

export interface ModularObjectExtensionInfo {
    bundleName: string;
    moduleName: string;
    abilityName: string;
    appIndex: int;
    isSystemDisabled: boolean;
}

class ModularObjectExtensionInfoInner implements ModularObjectExtensionInfo {
    bundleName: string = '';
    moduleName: string = '';
    abilityName: string = '';
    appIndex: int = 0;
    isSystemDisabled: boolean = false;
}

export native function nativeQueryAllModularObjectExtension(callback?: AsyncCallbackWrapper<Array<ModularObjectExtensionInfo>>): void;
export function queryAllModularObjectExtension(): Promise<Array<ModularObjectExtensionInfo>> {
    let p = new Promise<Array<ModularObjectExtensionInfo>>((resolve:(data:Array<ModularObjectExtensionInfo>)=>void, reject:(err: BusinessError)=>void):void => {
        let myCall = new AsyncCallbackWrapper<Array<ModularObjectExtensionInfo>>((err: BusinessError | null, data: Array<ModularObjectExtensionInfo> | undefined)=>{
            if (err == null || err.code == 0) {
                resolve(data as Array<ModularObjectExtensionInfo>);
            } else {
                reject(err);
            }
        });
        taskpool.execute((): void => {
            modularObjectManager.nativeQueryAllModularObjectExtension(myCall);
        });
    });
    return p;
}

export native function nativeSetModularObjectDisabledState(bundleName: string, moduleName: string, abilityName: string, appIndex: int, isDisabled: boolean,
    callback: AsyncCallbackWrapper<void>) : void;
export function setModularObjectDisabledState(bundleName: string, moduleName: string, abilityName: string, appIndex: int, isDisabled: boolean): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
            resolve(undefined);
        } else {
            reject(err);
        }
        });
        taskpool.execute((): void => {
            modularObjectManager.nativeSetModularObjectDisabledState(bundleName, moduleName, abilityName, appIndex, isDisabled, myCall);
        });
    });
    return p;
}

export native function nativeSetModularObjectGlobalDisabledState(isDisabled: boolean, callback: AsyncCallbackWrapper<void>) : void;
export function setModularObjectGlobalDisabledState(isDisabled: boolean): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
            resolve(undefined);
        } else {
            reject(err);
        }
        });
        taskpool.execute((): void => {
            modularObjectManager.nativeSetModularObjectGlobalDisabledState(isDisabled, myCall);
        });
    });
    return p;
}

export native function nativeGetModularObjectGlobalDisabledState(callback: AsyncCallbackWrapper<boolean>) : void;
export function getModularObjectGlobalDisabledState(): Promise<boolean> {
    let p = new Promise<boolean>((resolve:(data:boolean)=>void, reject:(err: BusinessError)=>void):void => {
        let myCall = new AsyncCallbackWrapper<boolean>((err: BusinessError | null, data: boolean | undefined)=>{
        if (err == null || err.code == 0) {
            resolve(data as boolean);
        } else {
            reject(err);
        }
        });
        taskpool.execute((): void => {
            modularObjectManager.nativeGetModularObjectGlobalDisabledState(myCall);
        });
    });
    return p;
}
} // namespace modularObjectManager
