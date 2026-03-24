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

import abcRunner from '@ohos.app.ability.abcRunner';
import { BusinessError } from '@ohos.base';

/**
 * 示例1：在主 Ability 中运行 abc 文件（使用 Promise）
 * 
 * 场景：在 UIAbility 中运行 abc 文件（同进程执行）
 */
class ExampleUsage {
  
  /**
   * 运行 abc 文件的基本用法（Promise 版本）
   */
  async runAbcFile(): Promise<void> {
    try {
      // 定义运行选项
      const options = {
        path: '/data/app/el2/100/base/com.example.app/files/scripts/myscript.abc',
        params: {
          action: 'calculate',
          input: [1, 2, 3, 4, 5]
        }
      };

      // 调用 runAbc 并等待结果（同步执行，会阻塞直到 setAbcResult 被调用）
      const result = await abcRunner.runAbc(options);
      
      if (result.code === 0) {
        console.info('ABC 执行成功');
        console.info('返回数据:', JSON.stringify(result.data));
        console.info('消息:', result.message);
      } else {
        console.error('ABC 执行失败，错误码:', result.code);
        console.error('错误消息:', result.message);
      }
    } catch (error) {
      const err = error as BusinessError;
      console.error('调用 runAbc 失败:', err.code, err.message);
    }
  }

  /**
   * 运行 abc 文件（Callback 版本）
   */
  runAbcWithCallback(): void {
    abcRunner.runAbc(
      {
        path: '/data/app/el2/100/base/com.example.app/files/scripts/data_processor.abc',
        params: { source: 'database', table: 'users' }
      },
      (err, result) => {
        if (err) {
          console.error('执行失败:', err.code, err.message);
          return;
        }
        
        console.info('ABC 执行完成，结果码:', result.code);
        if (result.data) {
          this.processResult(result.data);
        }
      }
    );
  }

  private processResult(data: object): void {
    // 处理 abc 返回的数据
    console.info('处理结果:', JSON.stringify(data));
  }
}

/**
 * 示例2：ABC 文件内部定义 ScriptProcess
 * 
 * ABC 文件必须将 ScriptProcess 挂载到 globalThis 上
 * 
 * file: scripts/myscript.abc
 * 
 * ```typescript
 * // 定义 ScriptProcess 对象到 globalThis
 * globalThis.ScriptProcess = {
 *     onStart(args: object) {
 *         // args 是调用方传递的参数
 *         console.info('收到参数:', args);
 *         
 *         // 执行业务逻辑
 *         const sum = args.input.reduce((a: number, b: number) => a + b, 0);
 *         
 *         // 返回结果给调用方
 *         abcRunner.setAbcResult({
 *             code: 0,  // 0 表示成功
 *             data: { 
 *                 sum: sum,
 *                 count: args.input.length 
 *             },
 *             message: '计算完成'
 *         });
 *     }
 * };
 * ```
 */

/**
 * 示例3：实际应用场景 - 插件系统
 * 
 * 通过运行不同的 abc 文件实现动态插件加载
 */
class PluginSystemExample {
  private pluginDir: string = '/data/app/el2/100/base/com.example.app/files/plugins/';

  /**
   * 加载并执行插件
   */
  async loadPlugin(pluginName: string, pluginParams: object): Promise<void> {
    const pluginPath = `${this.pluginDir}${pluginName}.abc`;
    
    try {
      const result = await abcRunner.runAbc({
        path: pluginPath,
        params: pluginParams
      });

      if (result.code === 0) {
        console.info(`插件 ${pluginName} 加载成功`);
        // 插件返回的数据可能包含初始化信息或执行结果
        this.handlePluginResult(pluginName, result.data);
      } else {
        console.error(`插件 ${pluginName} 加载失败:`, result.message);
      }
    } catch (error) {
      const err = error as BusinessError;
      console.error(`插件 ${pluginName} 异常:`, err.code, err.message);
    }
  }

  /**
   * 执行插件特定功能
   */
  async executePluginFunction(pluginName: string, functionName: string, args: object): Promise<void> {
    try {
      const result = await abcRunner.runAbc({
        path: `${this.pluginDir}${pluginName}.abc`,
        params: {
          __action: 'executeFunction',
          __function: functionName,
          ...args
        }
      });

      console.info(`插件功能 ${functionName} 执行完成`);
      // 处理返回结果
    } catch (error) {
      console.error(`执行功能失败:`, error);
    }
  }

  private handlePluginResult(pluginName: string, data?: object): void {
    // 处理插件初始化结果
    console.info(`插件 ${pluginName} 初始化数据:`, JSON.stringify(data));
  }
}

/**
 * 示例4：数据处理工作流
 * 
 * 使用 abc 文件实现数据处理流水线
 */
class DataPipelineExample {
  private basePath: string = '/data/app/el2/100/base/com.example.app/files/pipeline/';
  
  /**
   * 执行数据处理流水线
   */
  async runDataPipeline(inputData: object): Promise<void> {
    const pipeline = [
      { stage: 'validate', path: `${this.basePath}validate.abc` },
      { stage: 'transform', path: `${this.basePath}transform.abc` },
      { stage: 'enrich', path: `${this.basePath}enrich.abc` },
      { stage: 'output', path: `${this.basePath}output.abc` }
    ];

    let currentData = inputData;

    for (const stage of pipeline) {
      try {
        console.info(`开始执行阶段: ${stage.stage}`);
        
        const result = await abcRunner.runAbc({
          path: stage.path,
          params: { data: currentData }
        });

        if (result.code === 0) {
          console.info(`阶段 ${stage.stage} 完成`);
          currentData = result.data || currentData;
        } else {
          console.error(`阶段 ${stage.stage} 失败:`, result.message);
          // 可以选择重试或终止流水线
          break;
        }
      } catch (error) {
        console.error(`阶段 ${stage.stage} 异常:`, error);
        break;
      }
    }

    console.info('流水线执行完成');
  }
}

/**
 * 示例5：批量运行多个 abc 文件
 */
class BatchRunnerExample {
  private scriptDir: string = '/data/app/el2/100/base/com.example.app/files/scripts/';
  
  /**
   * 批量运行脚本
   */
  async runMultipleScripts(): Promise<void> {
    const scripts = [
      { path: `${this.scriptDir}validate.abc`, params: { type: 'input' } },
      { path: `${this.scriptDir}process.abc`, params: { type: 'transform' } },
      { path: `${this.scriptDir}save.abc`, params: { type: 'output' } }
    ];

    const promises = scripts.map(script => 
      abcRunner.runAbc({
        path: script.path,
        params: script.params
      })
    );

    try {
      const results = await Promise.all(promises);
      
      const success = results.filter(r => r.code === 0);
      const failed = results.filter(r => r.code !== 0);
      
      console.info(`批量执行完成: 成功 ${success.length} 个, 失败 ${failed.length} 个`);
      
      if (failed.length > 0) {
        console.error('失败的脚本:', failed.map(f => f.message));
      }
    } catch (error) {
      console.error('批量执行异常:', error);
    }
  }
}

export { ExampleUsage, PluginSystemExample, DataPipelineExample, BatchRunnerExample };
