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

 #include "context_manager.h"

 #include "hilog_tag_wrapper.h"
 
 namespace OHOS {
 namespace AbilityRuntime {

ContextManager::ContextManager()
{}

ContextManager::~ContextManager()
{
    std::lock_guard<std::mutex> lock(ContextMutex_);
    for (auto &iter : ContextMap_) {
        iter.second.reset();
    }
}
ContextManager& ContextManager::GetContextManager()
 {
    static ContextManager contextManager;
    return contextManager;
 }
 
 void ContextManager::AddManagerContext(ani_long nativeContextLong, std::shared_ptr<Context>& Context)
 {
    std::lock_guard<std::mutex> lock(ContextMutex_);

    auto iter = ContextMap_.find(nativeContextLong);
    if (iter == ContextMap_.end()) {
    ContextMap_[nativeContextLong] = Context;
        return;
    }

    if (iter->second != nullptr) {
        iter->second.reset();
        iter->second = nullptr;
    }

    iter->second = Context;
 }
 
 std::shared_ptr<Context> ContextManager::GetManagerContext(ani_long nativeContextLong)
 {

    std::lock_guard<std::mutex> lock(ContextMutex_);
    return ContextMap_[nativeContextLong];
 }
 
 void ContextManager::RemoveManagerContext(ani_long nativeContextLong)
 {

    std::lock_guard<std::mutex> lock(ContextMutex_);
    auto iter = ContextMap_.find(nativeContextLong);
    if (iter != ContextMap_.end() && iter->second != nullptr) {
        iter->second.reset();
        iter->second = nullptr;
        ContextMap_.erase(nativeContextLong);
    }
 }
 }  // namespace AbilityRuntime
 }  // namespace OHOS
 
