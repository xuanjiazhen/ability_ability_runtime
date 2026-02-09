/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_CONFIGURATION_H
#define MOCK_CONFIGURATION_H

#include <string>
#include <map>

#include "configuration.h"

namespace OHOS {
namespace AppExecFwk {

class MockConfiguration : public Configuration {
public:
    MockConfiguration() = default;
    ~MockConfiguration() override = default;

    std::string GetItem(const std::string& key) const override
    {
        auto it = configMap_.find(key);
        if (it != configMap_.end()) {
            return it->second;
        }
        return "";
    }

    void SetItem(const std::string& key, const std::string& value)
    {
        configMap_[key] = value;
    }

    std::string GetName() const override
    {
        return "MockConfiguration";
    }

private:
    std::map<std::string, std::string> configMap_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_CONFIGURATION_H
