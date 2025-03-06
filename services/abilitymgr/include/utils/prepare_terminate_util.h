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

#ifndef OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_UTIL_H
#define OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_UTIL_H

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace OHOS {
namespace AAFwk {
class PrepareTerminateUtil {
public:
    static PrepareTerminateUtil &GetInstance();
    ~PrepareTerminateUtil() = default;
    PrepareTerminateUtil(const PrepareTerminateUtil &) = delete;
    void operator=(const PrepareTerminateUtil &) = delete;

    std::shared_ptr<bool> AddPrepareTerminateResult(int32_t pid);
    void RemovePrepareTerminateResult(int32_t pid, std::shared_ptr<bool> result);
    void PutResult(int32_t pid, bool result);
protected:
    PrepareTerminateUtil() = default;
private:
    std::mutex resultListMutex_;
    std::unordered_map<int32_t, std::list<std::shared_ptr<bool>>> resultList_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_UTIL_H
