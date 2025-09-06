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
import { BusinessError, AsyncCallback } from '@ohos.base';
import { MissionInfo as _MissionInfo } from 'application.MissionInfo';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { MissionListener as _MissionListener } from 'application.MissionListener';
//import {MissionSnapshot as _MissionSnapshot } from 'application.MissionSnapshot';   //后续加文件

export default namespace missionManager {
  loadLibrary("missionmanager_ani.z")
  export native function nativeClearAllMissions(callback: AsyncCallbackWrapper<void>): void;
  export native function nativeGetMissionInfo(deviceId: string, missionId: int,
    callback: AsyncCallbackWrapper<MissionInfo>): void;
  export native function nativeGetMissionInfos(deviceId: string, numMax: int,     //getMissionInfos  一
    callback: AsyncCallbackWrapper<Array<MissionInfo>>): void;
  export native function nativeClearMission(missionId: int,                       //clearMission  二
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeLockMission(missionId: int,                       //lockMission  三
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeUnlockMission(missionId: int,                     //unlockMission  四
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeGetLowResolutionMissionSnapShot(deviceId: string, missionId: int,     //getLowResolutionMissionSnapShot  五
    callback: AsyncCallbackWrapper<MissionSnapshot>): void;
  export native function nativeGetMissionSnapShot(deviceId: string, missionId: int,     //getMissionSnapShot  六
    callback: AsyncCallbackWrapper<MissionSnapshot>): void;
  export native function nativeMoveMissionsToBackground(missionIds: Array<int>,    //moveMissionsToBackground  七
    callback: AsyncCallbackWrapper<Array<int>>): void;
  export native function nativeMoveMissionsToForeground(missionIds: Array<int>, topMission: int,    //moveMissionsToForeground  八
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeMoveMissionToFront(missionId: int, options: StartOptions,    //MoveMissionToFront  九
    callback: AsyncCallbackWrapper<void>): void;
  export native function nativeOn(type: 'mission', listener: MissionListener): int;  //on  十
  export native function nativeOff(type: 'mission', listenerId: int,    //off  十一
    callback: AsyncCallbackWrapper<void>): void;

  export function clearAllMissions(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeClearAllMissions(myCall);
    });
  }

  export function clearAllMissions(): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeClearAllMissions(myCall);
        });
      });
    return p;
  }

  export function getMissionInfo(deviceId: string, missionId: int, callback: AsyncCallback<MissionInfo>): void {
    let myCall = new AsyncCallbackWrapper<MissionInfo>(callback);
    taskpool.execute((): void => {
      missionManager.nativeGetMissionInfo(deviceId, missionId, myCall);
    });
  }

  export function getMissionInfo(deviceId: string, missionId: int): Promise<MissionInfo> {
    let p =
      new Promise<MissionInfo>((resolve: (data: MissionInfo) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<MissionInfo>((err: BusinessError | null, data: MissionInfo | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as MissionInfo);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeGetMissionInfo(deviceId, missionId, myCall);
        });
      });
    return p;
  }

export function getMissionInfos(deviceId: string, numMax: int, callback: AsyncCallback<Array<MissionInfo>>): void {  //getMissionInfos  一
    let myCall = new AsyncCallbackWrapper<Array<MissionInfo>>(callback);                                          //Array<MissionInfo>  注意这个？
    taskpool.execute((): void => {
      missionManager.nativeGetMissionInfos(deviceId, numMax, myCall);
    });
  }

  export function getMissionInfos(deviceId: string, numMax: int): Promise<Array<MissionInfo>> {
    let p =
      new Promise<Array<MissionInfo>>((resolve: (data: Array<MissionInfo>) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Array<MissionInfo>>((err: BusinessError | null, data: Array<MissionInfo> | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Array<MissionInfo>);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeGetMissionInfos(deviceId, numMax, myCall);
        });
      });
    return p;
  }
  
  export function clearMission(missionId: int, callback: AsyncCallback<void>): void {   //clearMission  二
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeClearMission(missionId, myCall);
    });
  }

  export function clearMission(missionId: int): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeClearMission(missionId, myCall);
        });
      });
    return p;
  }
  
  export function lockMission(missionId: int, callback: AsyncCallback<void>): void {   //lockMission  三
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeLockMission(missionId, myCall);
    });
  }

  export function lockMission(missionId: int): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeLockMission(missionId, myCall);
        });
      });
    return p;
  }

  export function unlockMission(missionId: int, callback: AsyncCallback<void>): void {   //unlockMission  四
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeUnlockMission(missionId, myCall);     //nativeUnlockMission  大小写注意一下？
    });
  }

  export function unlockMission(missionId: int): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeUnlockMission(missionId, myCall);
        });
      });
    return p;
  }


  export function getLowResolutionMissionSnapShot(deviceId: string, missionId: int, callback: AsyncCallback<MissionSnapshot>): void {  
    let myCall = new AsyncCallbackWrapper<MissionSnapshot>(callback);                          //getLowResolutionMissionSnapShot  五
    taskpool.execute((): void => {
      missionManager.nativeGetLowResolutionMissionSnapShot(deviceId, missionId, myCall);
    });
  }

  export function getLowResolutionMissionSnapShot(deviceId: string, missionId: int): Promise<MissionSnapshot> {
    let p =
      new Promise<MissionSnapshot>((resolve: (data: MissionSnapshot) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<MissionSnapshot>((err: BusinessError | null, data: MissionSnapshot| undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as MissionSnapshot);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeGetLowResolutionMissionSnapShot(deviceId, missionId, myCall);
        });
      });
    return p;
  }

  export function getMissionSnapShot(deviceId: string, missionId: int, callback: AsyncCallback<MissionSnapshot>): void {
    let myCall = new AsyncCallbackWrapper<MissionSnapshot>(callback);                        //getMissionSnapShot  六
    taskpool.execute((): void => {
      missionManager.nativeGetMissionSnapShot(deviceId, missionId, myCall);
    });
  }

  export function getMissionSnapShot(deviceId: string, missionId: number): Promise<MissionSnapshot> {
    let p =
      new Promise<MissionSnapshot>((resolve: (data: MissionSnapshot) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<MissionSnapshot>((err: BusinessError | null, data: MissionSnapshot| undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as MissionSnapshot);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeGetMissionSnapShot(deviceId, missionId, myCall);
        });
      });
    return p;
  }

  export function moveMissionsToBackground(missionIds: Array<int>, callback: AsyncCallback<Array<int>>): void {
    let myCall = new AsyncCallbackWrapper<Array<int>>(callback);                        //moveMissionsToBackground 七  
    taskpool.execute((): void => {                                                      //注意这个类型 Array<number> -> Array<number>
      missionManager.nativeMoveMissionsToBackground(missionIds, myCall);
    });
  }

  export function moveMissionsToBackground(missionIds : Array<int>): Promise<Array<int>> {
    let p =
      new Promise<Array<int>>((resolve: (data: Array<int>) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Array<int>>((err: BusinessError | null, data: Array<int>| undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Array<int>);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeMoveMissionsToBackground(missionIds, myCall);
        });
      });
    return p;
  }

  export function moveMissionsToForeground(missionIds: Array<int>, topMission: int, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);                        //moveMissionsToForeground  八  
    taskpool.execute((): void => {
      missionManager.nativeMoveMissionsToForeground(missionIds, topMission, myCall);
    });
  }

  export function moveMissionsToForeground(missionIds: Array<int>, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);                       
    taskpool.execute((): void => {
      missionManager.nativeMoveMissionsToForeground(missionIds, myCall);
    });
  }

  export function moveMissionsToForeground(missionIds: Array<int>, topMission?: int): Promise<void> {    //可选参数？
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeMoveMissionsToForeground(missionIds, topMission, myCall);
        });
      });
    return p;
  }

  export function moveMissionToFront(missionId: int, options: StartOptions, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);                               //MoveMissionToFront  九
    taskpool.execute((): void => {
      missionManager.nativeMoveMissionToFront(missionId, options, myCall);
    });
  }

  export function moveMissionToFront(missionId: int, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeMoveMissionToFront(missionId, myCall);
    });
  }

  export function moveMissionToFront(missionId: int, options?: StartOptions): Promise<void> {   //可选参数
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeMoveMissionToFront(missionId, options, myCall);
        });
      });
    return p;
  }

  export function on(type: 'mission', listener: MissionListener): number {    //on  十
    return missionManager.nativeOn('misson',listener);
  }

  export function off(type: 'mission', listenerId: int, callback: AsyncCallback<void>): void {  //off  十一
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      missionManager.nativeOff(type, listenerId, myCall);
    });
  }

  export function off(type: 'mission', listenerId: int): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError | null) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          missionManager.nativeOff(type, listenerId, myCall);
        });
      });
    return p;
  }

  export type MissionInfo = _MissionInfo;
  export type MissionListener = _MissionListener;
  export type MissionSnapshot = _MissionSnapshot;
}