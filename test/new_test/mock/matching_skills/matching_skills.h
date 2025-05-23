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

#ifndef MOCK_FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_MATCHING_SKILLS_H
#define MOCK_FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_MATCHING_SKILLS_H

#include <string>

namespace OHOS {
namespace EventFwk {
struct CommonEventSupport {
    inline static const std::string COMMON_EVENT_SCREEN_UNLOCKED = "usual.event.SCREEN_UNLOCKED";
    inline static const std::string COMMON_EVENT_USER_UNLOCKED = "usual.event.USER_UNLOCKED";
};

class MatchingSkills {
public:
    void AddEvent(const std::string &event) {}
};
}  // namespace EventFwk
}  // namespace OHOS
#endif