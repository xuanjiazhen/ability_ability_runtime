/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_LOADER_H
#define OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_LOADER_H

#include <iremote_object.h>
#include <shared_mutex>
#include "ability_thread.h"
#include "ability_info.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief 动态创建 FAAbilityThread
 * @return Returns sptr<AbilityThread> pointing to FAAbilityThread, or nullptr if failed
 *
 * 此函数通过 dlopen 动态加载 libabilitykit_native_fa.so，
 * 并获取其中的 CreateFAAbilityThread 函数来创建 FAAbilityThread 实例。
 *
 * 特性：
 * - 线程安全
 * - 延迟加载（首次调用时才加载 FA SO）
 * - 缓存函数指针（后续调用直接使用缓存）
 */
sptr<AbilityThread> CreateFAAbilityThreadDynamic();

/**
 * @brief 检查 FA SO 是否已加载
 * @return Returns true if FA SO is loaded, false otherwise
 */
bool IsFaAbilitySoLoaded();

/**
 * @brief 卸载 FA SO（仅用于测试，生产环境不建议调用）
 * @return Returns true if successfully unloaded, false otherwise
 */
bool UnloadFaAbilitySo();

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_LOADER_H
