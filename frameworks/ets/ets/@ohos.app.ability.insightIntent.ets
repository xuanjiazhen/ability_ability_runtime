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

import Want from '@ohos.app.ability.Want';
import wantConstant from '@ohos.app.ability.wantConstant';
import { RecordData } from '@ohos.base';

namespace insightIntent {

  export enum ExecuteMode {
    UI_ABILITY_FOREGROUND = 0,
    UI_ABILITY_BACKGROUND = 1,
    UI_EXTENSION_ABILITY = 2,
    SERVICE_EXTENSION_ABILITY = 3,
  }

  export interface ExecuteResult {
    code: int;
    result?: Record<string, RecordData>;
    uris?: Array<string>;
    flags?: int;
  }

  export interface IntentResult<T> {
    code: int;
    result?: T;
  }

  export class ExecuteResultInner implements ExecuteResult{
    code: int;
    result?: Record<string, RecordData>;
    uris?: Array<string>;
    flags?: int;
  }

  export enum ReturnMode {  
    CALLBACK = 0,
    FUNCTION = 1
  }

  export enum QueryType {
    ALL = 'all',
    BY_PROPERTY = 'byProperty'
  }

  export interface QueryEntityParam {
    queryType: QueryType;
    parameters?: Record<string, RecordData>;
  }

  interface IntentEntity {
    entityId: string;
  }

  abstract class AppIntentEntity<T> implements IntentEntity {
    displayName: string;
    abstract onQueryEntity(params: QueryEntityParam): Promise<Array<T>>;

    public nativeExecutor: long = 0;
    public native nativeOnQueryEntityResult(data: Array<Record<string, RecordData>>): void;
    public native nativeOnQueryEntityError(err: Error): void;

    public callPromise(p: Promise<Any>): void {
      p.then((result: Any): void => {
        let recordArr = result as Array<Record<string, RecordData>>;
        this.nativeOnQueryEntityResult(recordArr);
      }).catch((err: Error): void => {
        this.nativeOnQueryEntityError(err);
      });
    }
  }
}

export default insightIntent;