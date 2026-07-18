/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RECOVERY_LOG_UTILS_H
#define OHOS_ABILITY_RUNTIME_RECOVERY_LOG_UTILS_H

#include <cstddef>
#include <string>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
constexpr size_t RECOVERY_LOG_MAX_LEN = 1024; // 1K threshold
constexpr size_t RECOVERY_LOG_CHUNK_SIZE = 900; // leave room for prefix and index

inline void LogRecoveryContent(AAFwkTag tag, const std::string &label, const std::string &content)
{
    if (content.empty()) {
        TAG_LOGI(tag, "%{public}s: <empty>", label.c_str());
        return;
    }
    if (content.size() <= RECOVERY_LOG_MAX_LEN) {
        TAG_LOGI(tag, "%{public}s (size:%{public}zu): %{public}s",
            label.c_str(), content.size(), content.c_str());
        return;
    }
    size_t total = (content.size() + RECOVERY_LOG_CHUNK_SIZE - 1) / RECOVERY_LOG_CHUNK_SIZE;
    size_t index = 1;
    for (size_t pos = 0; pos < content.size(); pos += RECOVERY_LOG_CHUNK_SIZE, ++index) {
        std::string chunk = content.substr(pos, RECOVERY_LOG_CHUNK_SIZE);
        TAG_LOGI(tag, "%{public}s (size:%{public}zu) [%{public}zu/%{public}zu]: %{public}s",
            label.c_str(), content.size(), index, total, chunk.c_str());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RECOVERY_LOG_UTILS_H
