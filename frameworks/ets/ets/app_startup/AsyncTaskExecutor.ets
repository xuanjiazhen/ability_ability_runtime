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
import { AsyncTaskCallback } from './async_task_callback';

export default class AsyncTaskExecutor {
  // 在taskpool中执行的静态方法

  private native function nativeOnTaskSuccess(callback: long, result?: Object): void;
  private native function nativeOnTaskFailure(callback: long, error: string): void;
  private static executeStartupTask(startup: StartupTask, context: common.AbilityStageContext): Promise<any> {
    console.log('executeStartupTask start.');
    // 直接调用startup.init并返回Promise
    return startup.init(context);
  }

  public asyncPushTask(startup: StartupTask, asyncCallback: AsyncTaskCallback, context: common.AbilityStageContext, startupName: string): void {
    console.log('asyncPushTask start for task: ' + startupName);
    
    try {
      // 创建taskpool任务
      let task = new taskpool.Task(AsyncTaskExecutor.executeStartupTask, startup, context);
      
      // 执行任务并处理结果
      taskpool.execute(task).then((result: any) => {
        console.log('Task completed successfully for: ' + startupName);
        asyncCallback.onAsyncTaskCompleted(startupName, result);
      }).catch((error: Error) => {
        console.error('Task failed for: ' + startupName + ', error: ' + error);
        // 传递错误信息给回调
        asyncCallback.onAsyncTaskCompleted(startupName, { 
          error: true, 
          message: error.message 
        });
      });
    } catch (error) {
      console.error('Failed to create or execute task for: ' + startupName + ', error: ' + error);
      asyncCallback.onAsyncTaskCompleted(startupName, { 
        error: true, 
        message: error.message 
      });
    }
  }

  // 主线程中执行的静态方法
  public executeStartupTaskOnMainThread(startup: StartupTask, context: common.AbilityStageContext, 
    callback: long, startupName: string): void {
    console.log('executeStartupTaskOnMainThread start for task: ' + startupName);
    // 调用startup.init并处理Promise
    startup.init(context).then((result: Object) => {
      console.log('Task completed successfully on main thread for: ' + startupName);
      // 调用native成功回调
      this.nativeRunCallback(callback, result);
    }).catch((error: Error) => {
      console.error('Task failed on main thread for: ' + startupName + ', error: ' + error);
    });
  }
}