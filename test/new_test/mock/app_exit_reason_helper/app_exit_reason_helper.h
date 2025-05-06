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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER
#define MOCK_OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER

#include <memory>
#include <mutex>

#include "exit_reason.h"
#include "sub_managers_helper.h"

namespace OHOS {
namespace AAFwk {
class AppExitReasonHelper {
public:
    explicit AppExitReasonHelper(std::shared_ptr<SubManagersHelper> subManagersHelper) {}
    ~AppExitReasonHelper() = default;

    int32_t RecordAppExitReason(const ExitReason &exitReason)
    {
        return 0;
    }

    int32_t RecordAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex,
        const ExitReason &exitReason)
    {
        return 0;
    }
    int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason, bool fromKillWithReason)
    {
        return 0;
    }
    int32_t RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason)
    {
        return 0;
    }
    int32_t RecordUIAbilityExitReason(const pid_t pid, const std::string &abilityName, const ExitReason &exitReason)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
