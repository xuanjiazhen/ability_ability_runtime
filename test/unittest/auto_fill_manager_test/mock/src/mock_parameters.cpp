/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "parameters.h"
#include <map>

namespace OHOS {
namespace system {

bool g_returnFlag{false};
std::map<std::string, std::string> g_stringParameterMap;

void SetBoolParameter(const std::string &key, bool def) { g_returnFlag = def; }

bool GetBoolParameter(const std::string &key, bool def) { return g_returnFlag; }

void SetParameter(const std::string &key, const std::string &value) { g_stringParameterMap[key] = value; }

std::string GetParameter(const std::string &key, const std::string &def)
{
    auto it = g_stringParameterMap.find(key);
    if (it != g_stringParameterMap.end()) {
        return it->second;
    }
    return def;
}

std::string GetParameters(const std::string &key, const std::string &def)
{
    auto it = g_stringParameterMap.find(key);
    if (it != g_stringParameterMap.end()) {
        return it->second;
    }
    return def;
}

} // namespace system
} // namespace OHOS