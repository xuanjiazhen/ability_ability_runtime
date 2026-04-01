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

#include <map>
#include <string>

namespace {
int g_callCount = 0;
std::map<std::string, int32_t> g_fds;
} // namespace

extern "C" void ChildMainThreadStart(const std::map<std::string, int32_t> &fds)
{
    ++g_callCount;
    g_fds = fds;
}

extern "C" void ResetFakeChildMainThreadStartState()
{
    g_callCount = 0;
    g_fds.clear();
}

extern "C" int GetFakeChildMainThreadStartCallCount()
{
    return g_callCount;
}

extern "C" int GetFakeChildMainThreadFdValue(const char *key)
{
    if (key == nullptr) {
        return -1;
    }
    auto iter = g_fds.find(key);
    if (iter == g_fds.end()) {
        return -1;
    }
    return iter->second;
}
