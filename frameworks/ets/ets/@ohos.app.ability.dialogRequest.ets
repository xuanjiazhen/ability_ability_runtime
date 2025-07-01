/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit AbilityKit
 */

import Want from '@ohos.app.ability.Want';

namespace dialogRequest {
  loadLibrary("dialogrequest_ani_kit.z");
  export interface WindowRect {
    left: number;
    top: number;
    width: number;
    height: number;
  }

  export class WindowRectInner implements WindowRect {
    left: number;
    top: number;
    width: number;
    height: number;
  }

  export interface RequestInfo {
    windowRect?: WindowRect
  }

  export class RequestInfoInner implements RequestInfo {
    nativeRequestInfo: long = 0;
    windowRect?: WindowRect;
  }
  
  export enum ResultCode {
    RESULT_OK = 0,
    RESULT_CANCEL = 1
  }

  export interface RequestResult {
    result: ResultCode;
    want?: Want;
  }

  export class RequestResultInner implements RequestResult {
    result: ResultCode;
    want?:Want;
  }

  export interface RequestCallback {
    setRequestResult(result: RequestResult): void;
  }

  export class RequestCallbackInner implements RequestCallback {
    nativeRequestCallback:long = 0;
    public native setRequestResult(result: RequestResult): void;
  }

  export native function getRequestInfo(want: Want): RequestInfo;
  export native function getRequestCallback(want: Want): RequestCallback;
}

export default dialogRequest;