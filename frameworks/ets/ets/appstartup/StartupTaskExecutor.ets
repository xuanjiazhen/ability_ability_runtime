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

import StartupTask from '@ohos.app.appstartup.StartupTask';
import common from '@ohos.app.ability.common';

export default class StartupTaskExecutor {

  private native function nativeOnTaskSuccess(callback: long, result?: Object): void;
  private native function nativeOnTaskFailure(callback: long, error: string): void;
  
  public asyncPushTask(startup: StartupTask, context: common.AbilityStageContext, callback: long): void {

    const executeAsync = async (startup: StartupTask, context: common.AbilityStageContext,
      callback: long): void => {
      try {
        const result = await startup.init(context);
        this.nativeOnTaskSuccess(callback, result);
      } catch (error) {
        this.nativeOnTaskFailure(callback, error.message);
      }
    };

    const task = new taskpool.Task(executeAsync, startup, context, callback);
    taskpool.execute(task);
  }

  public executeStartupTaskOnMainThread(startup: StartupTask, context: common.AbilityStageContext, 
    callback: long): void {
    startup.init(context).then((result: Object) => {
      this.nativeOnTaskSuccess(callback, result);
    }).catch((error: Error) => {
      this.nativeOnTaskFailure(callback, error.message);
    });
  }
}