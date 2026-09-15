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

#ifndef OHOS_ABILITY_RUNTIME_PENDING_WANT_GLOBAL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PENDING_WANT_GLOBAL_MANAGER_H

#include <list>
#include <string>
#include "cpp/mutex.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

class PendingWantGlobalManager : public NoCopyable {
public:
    PendingWantGlobalManager() = default;
    ~PendingWantGlobalManager() = default;

    void AddSharedRecord(const std::string &bundleName, int32_t creatorPid, int32_t code);
    void ShrinkIfNeeded();
    void RemoveByCreatorPid(int32_t pid, const std::string &bundleName);
    void RemoveByBundleName(const std::string &bundleName);
    size_t GetSize();

    static constexpr uint32_t DEFAULT_THRESHOLD = 100000;
    static constexpr float SHRINK_RATIO = 0.5f;

private:
    struct GlobalPendingRecord {
        int32_t code = 0;
        int32_t creatorPid = 0;
        std::string bundleName;
        int64_t transferTime = 0;
    };

    std::list<GlobalPendingRecord> globalRecords_;
    uint32_t threshold_ = DEFAULT_THRESHOLD;
    float shrinkRatio_ = SHRINK_RATIO;
    ffrt::mutex mutex_;

    int64_t GetCurrentTimestampMs() const;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PENDING_WANT_GLOBAL_MANAGER_H