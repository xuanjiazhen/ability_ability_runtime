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

#include "prepare_terminate_util.h"

namespace OHOS {
namespace AAFwk {
PrepareTerminateUtil &PrepareTerminateUtil::GetInstance()
{
    static PrepareTerminateUtil instance;
    return instance;
}

std::shared_ptr<bool> PrepareTerminateUtil::AddPrepareTerminateResult(int32_t pid)
{
    std::lock_guard lock(resultListMutex_);
    auto &list = resultList_[pid];
    auto result = std::make_shared<bool>(false);
    list.push_back(result);
    return result;
}

void PrepareTerminateUtil::RemovePrepareTerminateResult(int32_t pid, std::shared_ptr<bool> result)
{
    std::lock_guard lock(resultListMutex_);
    auto it = resultList_.find(pid);
    if (it == resultList_.end()) {
        return;
    }
    it->second.remove(result);
    if (it->second.empty()) {
        resultList_.erase(it);
    }
}

void PrepareTerminateUtil::PutResult(int32_t pid, bool result)
{
    std::lock_guard lock(resultListMutex_);
    auto it = resultList_.find(pid);
    if (it == resultList_.end() || it->second.empty()) {
        return;
    }
    for (auto &item : it->second) {
        *item = result;
    }
}
}  // namespace AAFwk
}  // namespace OHOS

