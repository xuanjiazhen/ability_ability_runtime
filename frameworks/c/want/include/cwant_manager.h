/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cwant.h"
#include "want.h"

struct AbilityRuntime_Want {
    AbilityRuntime_Element element;
    std::map<std::string, std::string> params;
    std::map<std::string, int32_t> fds;
};

namespace OHOS {
namespace AbilityRuntime {
class CWantManager {
public:
    [[maybe_unused]] static AbilityRuntime_ErrorCode TransformToWant(AbilityRuntime_Want cwant, bool isDup,
        AAFwk::Want &want);
};
}
}