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

#include "js_cli_manager.h"

namespace OHOS {
namespace CliMgr {
namespace JS {

/**
 * @brief 模块入口函数
 *
 * NAPI 模块初始化入口
 */
extern "C" __attribute__((visibility("default")))
void NAPI_CLI_Manager_GetJSCode(napi_env env, napi_value exports)
{
    // 初始化模块
    JSCliManager::Init(env, exports);
}

/**
 * @brief 模块导出函数
 */
static napi_value ModuleInit(napi_env env, napi_value exports)
{
    return JSCliManager::Init(env, exports);
}

/**
 * @brief 模块定义
 */
extern "C" __attribute__((visibility("default")))
void NAPI_OHOS_Cli_Manager_GetJSCode(napi_env env, napi_value exports)
{
    ModuleInit(env, exports);
}

} // namespace JS
} // namespace CliMgr
} // namespace OHOS
