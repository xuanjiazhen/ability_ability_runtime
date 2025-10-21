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

export class AsyncTaskCallback {
  private callbackImpl: AsyncTaskCallbackImpl;

  constructor(impl: AsyncTaskCallbackImpl) {
    this.callbackImpl = impl;
  }

  public onAsyncTaskCompleted(startupName: string, startupResult: any): void {
    console.log('AsyncTaskCallback onAsyncTaskCompleted called, startupName: ' + startupName);
    this.callbackImpl.onAsyncTaskCompleted(startupName, startupResult);
  }
}

// 回调接口，由C++侧实现
export interface AsyncTaskCallbackImpl {
  onAsyncTaskCompleted(startupName: string, startupResult: any): void;
}