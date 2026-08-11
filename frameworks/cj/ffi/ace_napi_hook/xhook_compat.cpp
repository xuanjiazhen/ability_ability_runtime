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

#include "xhook_compat.h"

#include <cstring>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "xhook/xh_core.h"

namespace {

struct HookCompatState {
    std::mutex mu;
    std::vector<std::pair<std::string, void*>> hooks;
    std::vector<std::string> ignores;
};

HookCompatState& GetState()
{
    static HookCompatState state;
    return state;
}

std::string EscapeRegex(const char* input)
{
    static constexpr char kRegexMeta[] = R"(\.^$|()[]{}*+?)";
    std::string escaped;
    if (input == nullptr) {
        return escaped;
    }
    for (const char* p = input; *p != '\0'; ++p) {
        if (std::char_traits<char>::find(kRegexMeta, sizeof(kRegexMeta) - 1, *p) != nullptr) {
            escaped.push_back('\\');
        }
        escaped.push_back(*p);
    }
    return escaped;
}

std::string BuildSonameRegex(const char* soname)
{
    return std::string(".*/") + EscapeRegex(soname) + "$";
}

int RegisterRulesForLibraryLocked(HookCompatState& state, const char* soname)
{
    const std::string regex = BuildSonameRegex(soname);
    for (const auto& [symbol, new_func] : state.hooks) {
        if (::xh_core_register(regex.c_str(), symbol.c_str(), new_func, nullptr) != 0) {
            return -1;
        }
    }
    for (const auto& symbol : state.ignores) {
        if (::xh_core_ignore(regex.c_str(), symbol.c_str()) != 0) {
            return -1;
        }
    }
    return 0;
}

}  // namespace

extern "C" int xhook_register(const char* symbol, void* new_func)
{
    if (symbol == nullptr || symbol[0] == '\0' || new_func == nullptr) {
        return -1;
    }
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.mu);
    state.hooks.emplace_back(symbol, new_func);
    return 0;
}

extern "C" int xhook_ignore(const char* symbol)
{
    if (symbol == nullptr || symbol[0] == '\0') {
        return -1;
    }
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.mu);
    state.ignores.emplace_back(symbol);
    return 0;
}

extern "C" int xhook_refresh(int async)
{
    return ::xh_core_refresh(async);
}

extern "C" int xhook_refresh_library(const char* soname, int async)
{
    return xhook_refresh_libraries(&soname, 1, async);
}

extern "C" int xhook_refresh_libraries(const char* const* sonames, size_t count, int async)
{
    if (sonames == nullptr || count == 0) {
        return -1;
    }
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.mu);
    for (size_t i = 0; i < count; ++i) {
        if (sonames[i] == nullptr || sonames[i][0] == '\0' ||
            RegisterRulesForLibraryLocked(state, sonames[i]) != 0) {
            return -1;
        }
    }
    return ::xh_core_refresh(async);
}

extern "C" void xhook_clear(void)
{
    auto& state = GetState();
    std::lock_guard<std::mutex> lock(state.mu);
    state.hooks.clear();
    state.ignores.clear();
    ::xh_core_clear();
}

extern "C" void xhook_enable_debug(int flag)
{
    // The bundled HarmonyOS xhook does not expose xh_core_enable_debug.
    (void)flag;
}

