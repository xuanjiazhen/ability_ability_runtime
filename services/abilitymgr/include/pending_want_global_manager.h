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
#include "cpp/mutex.h"
#include "nocopyable.h"
#include "refbase.h"
#include "pending_want_record.h"

namespace OHOS {
namespace AAFwk {

class PendingWantGlobalManager : public NoCopyable {
public:
    PendingWantGlobalManager() = default;
    ~PendingWantGlobalManager() = default;

    void AddSharedRecord(sptr<PendingWantRecord> record);
    void ShrinkIfNeeded();

private:
    static constexpr float SHRINK_RATIO = 0.5f;

    std::list<sptr<PendingWantRecord>> globalRecords_;
    ffrt::mutex mutex_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PENDING_WANT_GLOBAL_MANAGER_H