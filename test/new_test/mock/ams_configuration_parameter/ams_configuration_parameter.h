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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_AMS_CONFIGURATION_PARAMETER_H
#define MOCK_OHOS_ABILITY_RUNTIME_AMS_CONFIGURATION_PARAMETER_H

#include <map>

namespace OHOS {
namespace AAFwk {
class AmsConfigurationParameter final {
public:
    enum { READ_OK = 0, READ_FAIL = 1, READ_JSON_FAIL = 2 };

    static AmsConfigurationParameter &GetInstance()
    {
        static AmsConfigurationParameter instance;
        return instance;
    }

    void Parse() {}

    int GetBootAnimationTimeoutTime() const
    {
        return 0;
    }

    bool IsSupportAAKillWithReason() const
    {
        return false;
    }

    int MultiUserType() const
    {
        return 0;
    }

    std::map<std::string, std::string> GetPickerMap() const
    {
        std::map<std::string, std::string> ret;
        return ret;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
