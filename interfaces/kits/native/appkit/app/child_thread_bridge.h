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
 
#ifndef OHOS_ABILITY_RUNTIME_CHILD_THREAD_BRIDGE_H
#define OHOS_ABILITY_RUNTIME_CHILD_THREAD_BRIDGE_H
 
#include <map>
#include <string>
 
namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Bridge interface for child process thread functionality.
 * This provides a decoupled interface between appkit_native.so and appkit_child.so.
 * The actual implementation resides in appkit_child.so.
 */
class ChildThreadBridge {
public:
    /**
     * @brief Start the child main thread with the given file descriptors.
     * @param fds The file descriptors map for child process communication.
     */
    static void StartChild(const std::map<std::string, int32_t> &fds);
};
 
}  // namespace AppExecFwk
}  // namespace OHOS
 
#endif  // OHOS_ABILITY_RUNTIME_CHILD_THREAD_BRIDGE_H