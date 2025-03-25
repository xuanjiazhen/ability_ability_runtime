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

 #ifndef OHOS_ABILITY_RUNTIME_CONTEXT_MANAGER_H
 #define OHOS_ABILITY_RUNTIME_CONTEXT_MANAGER_H
 
 #include "ani.h"
 
 #include <mutex>
 #include <unordered_map>
 #include "context.h"
 
 namespace OHOS {
 namespace AbilityRuntime {
 
 class ContextManager {
 public:
     ContextManager(const ContextManager&) = delete;
 
     ContextManager& operator=(const ContextManager&) = delete;
 
     static ContextManager& GetContextManager();
 
     void AddManagerContext(ani_long nativeContextLong, std::shared_ptr<Context>& Context);
 
     std::shared_ptr<Context> GetManagerContext(ani_long nativeContextLong);
 
     void RemoveManagerContext(ani_long nativeContextLong);
 
 private:
     ContextManager();
 
     ~ContextManager();
 
     std::unordered_map<ani_long, std::shared_ptr<Context>> ContextMap_;
     std::mutex ContextMutex_;
 };
 } // namespace AbilityRuntime
 } // namespace OHOS
 #endif  // OHOS_ABILITY_RUNTIME_CONTEXT_MANAGER_H
