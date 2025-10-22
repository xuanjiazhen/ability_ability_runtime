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
  // 在taskpool中执行的静态方法

  private native function nativeOnTaskSuccess(callback: long, result?: Object): void;
  private native function nativeOnTaskFailure(callback: long, error: string): void;
  
  // 任务池执行方法
  public asyncPushTask(startup: StartupTask, context: common.AbilityStageContext, startupName: string): void {
    console.log('asyncPushTask start for task: ' + startupName);
    
    // 异步执行函数
    const executeAsync = async (startup: StartupTask, context: common.AbilityStageContext): Promise<any> => {
      try {
        console.log('Starting startup task in taskpool: ' + startupName);
        const result = await startup.init(context);
        console.log('Startup task completed successfully in taskpool: ' + startupName);
        return result;
      } catch (error) {
        console.error('Startup task failed in taskpool: ' + startupName + ', error: ' + error);
        throw error;
      }
    };

    // 创建并执行任务
    const task = new taskpool.Task(executeAsync, startup, context);
    
    taskpool.execute(task)
      .then((result: any) => {
        console.log('Task execution completed for: ' + startupName);
        this.nativeOnTaskPoolSuccess(startupName, result);
      })
      .catch((error: Error) => {
        console.error('Task execution failed for: ' + startupName + ', error: ' + error);
        this.nativeOnTaskPoolFailure(startupName, error.message);
      });
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