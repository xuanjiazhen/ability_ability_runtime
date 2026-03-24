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

#ifndef JS_CLI_MANAGER_H
#define JS_CLI_MANAGER_H

#include "native_api.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace CliMgr {
namespace JS {

/**
 * @brief JS CLI Manager 类
 *
 * 提供给 JS 应用使用的 CLI 管理器接口
 */
class JSCliManager {
public:
    /**
     * @brief 初始化模块导出
     * @param env NAPI 环境
     * @param exports 导出对象
     * @return 导出对象
     */
    static napi_value Init(napi_env env, napi_value exports);

    /**
     * @brief 查询所有工具
     */
    static napi_value QueryTools(napi_env env, napi_callback_info info);

    /**
     * @brief 根据名称查询工具
     */
    static napi_value QueryToolByName(napi_env env, napi_callback_info info);

    /**
     * @brief 根据权限过滤工具
     */
    static napi_value QueryToolsByPermission(napi_env env, napi_callback_info info);

    /**
     * @brief 同步执行工具
     */
    static napi_value ExecTool(napi_env env, napi_callback_info info);

    /**
     * @brief 异步执行工具
     */
    static napi_value ExecToolAsync(napi_env env, napi_callback_info info);

    /**
     * @brief 取消异步任务
     */
    static napi_value CancelTask(napi_env env, napi_callback_info info);

    /**
     * @brief 查询任务状态
     */
    static napi_value QueryTaskStatus(napi_env env, napi_callback_info info);

private:
    /**
     * @brief JS 类构造函数
     */
    static napi_value Constructor(napi_env env, napi_callback_info info);

    /**
     * @brief 异步执行上下文
     */
    struct AsyncContext {
        napi_env env;
        napi_ref callbackRef;
        napi_deferred deferred;

        std::string toolName;
        Json::Value arguments;
        uint32_t timeoutMs;
        std::string workingDir;
        std::string taskId;

        int32_t errorCode;
        std::string errorMessage;

        AsyncContext() : env(nullptr),
                        callbackRef(nullptr),
                        deferred(nullptr),
                        timeoutMs(0),
                        errorCode(0) {}
    };

    /**
     * @brief 异步执行工作函数
     */
    static void ExecAsyncWork(napi_env env, void* data);

    /**
     * @brief 异步执行完成回调
     */
    static void ExecAsyncComplete(napi_env env, napi_status status, void* data);

    /**
     * @brief 从 JS 对象读取 ExecOptions
     */
    static bool ReadExecOptions(napi_env env, napi_value options,
                               ::OHOS::CliMgr::ExecOptions& execOptions);

    /**
     * @brief 从 JS 对象读取参数
     */
    static bool ReadArguments(napi_env env, napi_value args, Json::Value& outArgs);

    /**
     * @brief 创建 JS 工具信息对象
     */
    static napi_value CreateToolInfoObject(napi_env env,
                                          const ::OHOS::CliMgr::ToolInfo& tool);

    /**
     * @brief 创建 JS 执行结果对象
     */
    static napi_value CreateExecResultObject(napi_env env,
                                            const ::OHOS::CliMgr::ExecutionResult& result);

    /**
     * @brief 创建 JS 任务状态对象
     */
    static napi_value CreateTaskStatusObject(napi_env env,
                                            const ::OHOS::CliMgr::TaskStatus& status);

    /**
     * @brief 创建 JS 错误对象
     */
    static napi_value CreateError(napi_env env, int32_t errorCode,
                                 const std::string& errorMessage);

    // 类引用
    static napi_ref sConstructor_;
};

} // namespace JS
} // namespace CliMgr
} // namespace OHOS

#endif // JS_CLI_MANAGER_H
